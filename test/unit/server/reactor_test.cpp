/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   reactor_test.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:25:33 by hermarti          #+#    #+#             */
/*   Updated: 2026/09/23 00:00:00 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server/EventType.hpp"
#include "server/IEventHandler.hpp"
#include "server/Reactor.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <cerrno>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <exception>
#include <mutex>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

namespace
{
const int kReadCode = 10;
const int kWriteCode = 20;
const int kBothCode = 30;
const int kSecondCode = 21;
const int kCancelCode = 99;
const int kTimeoutMs = 3000;
const int kQuietMs = 300;

// Thrown from a probe callback to unwind Reactor::run() on the worker thread.
class StopReactor : public std::exception
{
  public:
	explicit StopReactor(int code) : code_(code)
	{
	}

	int code() const
	{
		return code_;
	}

	const char* what() const noexcept override
	{
		return "reactor probe requested shutdown";
	}

  private:
	int code_;
};

// Records the callbacks the reactor dispatches and stops run() once the
// configured condition is met.
class ProbeHandler : public IEventHandler
{
  public:
	enum Mode
	{
		STOP_ON_READ,
		STOP_ON_WRITE,
		STOP_WHEN_BOTH
	};

	ProbeHandler(int fd, Mode mode, int stop_code)
		: fd_(fd), mode_(mode), stop_code_(stop_code), reads_(0), writes_(0)
	{
	}

	int getFd() const
	{
		return fd_;
	}

	void handleReadEvent()
	{
		++reads_;
		check();
	}

	void handleWriteEvent()
	{
		++writes_;
		check();
	}

	bool wantsWrite() const
	{
		return false;
	}

	void handleTimeout()
	{
	}

	int reads() const
	{
		return reads_.load();
	}

	int writes() const
	{
		return writes_.load();
	}

  private:
	void check()
	{
		switch (mode_)
		{
			case STOP_ON_READ:
				if (reads_.load() > 0)
					throw StopReactor(stop_code_);
				break;
			case STOP_ON_WRITE:
				if (writes_.load() > 0)
					throw StopReactor(stop_code_);
				break;
			case STOP_WHEN_BOTH:
				if (reads_.load() > 0 && writes_.load() > 0)
					throw StopReactor(stop_code_);
				break;
		}
	}

	int fd_;
	Mode mode_;
	int stop_code_;
	std::atomic<int> reads_;
	std::atomic<int> writes_;
};

// Never stops the reactor: used where no callback is expected.
class SilentHandler : public IEventHandler
{
  public:
	explicit SilentHandler(int fd) : fd_(fd)
	{
	}

	int getFd() const
	{
		return fd_;
	}

	void handleReadEvent()
	{
	}

	void handleWriteEvent()
	{
	}

	bool wantsWrite() const
	{
		return false;
	}

	void handleTimeout()
	{
	}

  private:
	int fd_;
};

// Wakes a blocked run() so the worker thread can be joined deterministically.
class CancelHandler : public IEventHandler
{
  public:
	explicit CancelHandler(int fd) : fd_(fd)
	{
	}

	int getFd() const
	{
		return fd_;
	}

	void handleReadEvent()
	{
		throw StopReactor(kCancelCode);
	}

	void handleWriteEvent()
	{
	}

	bool wantsWrite() const
	{
		return false;
	}

	void handleTimeout()
	{
	}

  private:
	int fd_;
};

// Runs Reactor::run() on a worker thread until a callback unwinds it.
class RunSession
{
  public:
	RunSession(Reactor& reactor, int cancel_fd)
		: reactor_(reactor), cancel_fd_(cancel_fd), finished_(false),
		  stopped_(false), code_(0)
	{
		worker_ = std::thread(&RunSession::work, this);
	}

	~RunSession()
	{
		cancel();
		join();
	}

	bool waitFor(int timeout_ms)
	{
		std::unique_lock<std::mutex> lock(mutex_);
		return condition_.wait_for(lock,
								   std::chrono::milliseconds(timeout_ms),
								   [this]() { return finished_; });
	}

	void cancel()
	{
		if (cancel_fd_ < 0)
			return;
		const char byte = 'x';
		const ssize_t written = ::write(cancel_fd_, &byte, 1);
		(void) written;
	}

	void join()
	{
		if (worker_.joinable())
			worker_.join();
	}

	bool stopped()
	{
		std::lock_guard<std::mutex> lock(mutex_);
		return stopped_;
	}

	int code()
	{
		std::lock_guard<std::mutex> lock(mutex_);
		return code_;
	}

  private:
	void work()
	{
		try
		{
			reactor_.run();
		}
		catch (const StopReactor& stop)
		{
			std::lock_guard<std::mutex> lock(mutex_);
			stopped_ = true;
			code_ = stop.code();
		}
		catch (...)
		{
			// Any other exception also ends the run; assertions catch it.
		}
		{
			std::lock_guard<std::mutex> lock(mutex_);
			finished_ = true;
		}
		condition_.notify_all();
	}

	Reactor& reactor_;
	int cancel_fd_;
	std::thread worker_;
	std::mutex mutex_;
	std::condition_variable condition_;
	bool finished_;
	bool stopped_;
	int code_;
};

class SocketPair
{
  public:
	SocketPair() : ok_(::socketpair(AF_UNIX, SOCK_STREAM, 0, fds_) == 0)
	{
		if (!ok_)
		{
			fds_[0] = -1;
			fds_[1] = -1;
		}
	}

	~SocketPair()
	{
		closeEnd(fds_[0]);
		closeEnd(fds_[1]);
	}

	bool ok() const
	{
		return ok_;
	}

	int read_end() const
	{
		return fds_[0];
	}

	int write_end() const
	{
		return fds_[1];
	}

  private:
	static void closeEnd(int& fd)
	{
		if (fd >= 0)
		{
			::close(fd);
			fd = -1;
		}
	}

	int fds_[2];
	bool ok_;
};

class Pipe
{
  public:
	Pipe() : ok_(::pipe(fds_) == 0)
	{
		if (!ok_)
		{
			fds_[0] = -1;
			fds_[1] = -1;
		}
	}

	~Pipe()
	{
		closeReadEnd();
		closeWriteEnd();
	}

	bool ok() const
	{
		return ok_;
	}

	int read_end() const
	{
		return fds_[0];
	}

	void closeWriteEnd()
	{
		if (fds_[1] >= 0)
		{
			::close(fds_[1]);
			fds_[1] = -1;
		}
	}

  private:
	void closeReadEnd()
	{
		if (fds_[0] >= 0)
		{
			::close(fds_[0]);
			fds_[0] = -1;
		}
	}

	int fds_[2];
	bool ok_;
};
} // namespace

class ReactorTest : public ::testing::Test
{
  protected:
	void SetUp() override
	{
		ASSERT_EQ(0, ::pipe(cancel_pipe_)) << std::strerror(errno);
	}

	void TearDown() override
	{
		if (cancel_pipe_[0] >= 0)
			::close(cancel_pipe_[0]);
		if (cancel_pipe_[1] >= 0)
			::close(cancel_pipe_[1]);
	}

	int cancel_read() const
	{
		return cancel_pipe_[0];
	}

	int cancel_write() const
	{
		return cancel_pipe_[1];
	}

  private:
	int cancel_pipe_[2] = {-1, -1};
};

TEST_F(ReactorTest, DispatchesReadEventToRegisteredHandler)
{
	SocketPair pair;
	ASSERT_TRUE(pair.ok()) << std::strerror(errno);

	Reactor reactor;
	ProbeHandler probe(pair.read_end(), ProbeHandler::STOP_ON_READ, kReadCode);
	CancelHandler cancel(cancel_read());
	reactor.registerHandler(&probe, READ);
	reactor.registerHandler(&cancel, READ);

	ASSERT_EQ(1, ::write(pair.write_end(), "x", 1)) << std::strerror(errno);
	RunSession session(reactor, cancel_write());

	EXPECT_TRUE(session.waitFor(kTimeoutMs));
	EXPECT_TRUE(session.stopped());
	EXPECT_EQ(kReadCode, session.code());
	EXPECT_EQ(1, probe.reads());
	EXPECT_EQ(0, probe.writes());

	session.join();
}

TEST_F(ReactorTest, DispatchesWriteEventToRegisteredHandler)
{
	SocketPair pair;
	ASSERT_TRUE(pair.ok()) << std::strerror(errno);

	Reactor reactor;
	ProbeHandler probe(
		pair.read_end(), ProbeHandler::STOP_ON_WRITE, kWriteCode);
	CancelHandler cancel(cancel_read());
	reactor.registerHandler(&probe, WRITE);
	reactor.registerHandler(&cancel, READ);

	RunSession session(reactor, cancel_write());

	EXPECT_TRUE(session.waitFor(kTimeoutMs));
	EXPECT_TRUE(session.stopped());
	EXPECT_EQ(kWriteCode, session.code());
	EXPECT_EQ(0, probe.reads());
	EXPECT_EQ(1, probe.writes());

	session.join();
}

TEST_F(ReactorTest, DispatchesBothCallbacksWhenReadAndWriteAreRegistered)
{
	SocketPair pair;
	ASSERT_TRUE(pair.ok()) << std::strerror(errno);

	Reactor reactor;
	ProbeHandler probe(
		pair.read_end(), ProbeHandler::STOP_WHEN_BOTH, kBothCode);
	CancelHandler cancel(cancel_read());
	const EventType both = static_cast<EventType>(EPOLLIN | EPOLLOUT);
	reactor.registerHandler(&probe, both);
	reactor.registerHandler(&cancel, READ);

	ASSERT_EQ(1, ::write(pair.write_end(), "x", 1)) << std::strerror(errno);
	RunSession session(reactor, cancel_write());

	EXPECT_TRUE(session.waitFor(kTimeoutMs));
	EXPECT_TRUE(session.stopped());
	EXPECT_EQ(kBothCode, session.code());
	EXPECT_EQ(1, probe.reads());
	EXPECT_EQ(1, probe.writes());

	session.join();
}

TEST_F(ReactorTest, UpdateEventsSwitchesReadToWrite)
{
	SocketPair pair;
	ASSERT_TRUE(pair.ok()) << std::strerror(errno);

	Reactor reactor;
	ProbeHandler probe(
		pair.read_end(), ProbeHandler::STOP_ON_WRITE, kWriteCode);
	CancelHandler cancel(cancel_read());
	reactor.registerHandler(&probe, READ);
	reactor.registerHandler(&cancel, READ);
	reactor.updateEvents(pair.read_end(), WRITE);

	RunSession session(reactor, cancel_write());

	EXPECT_TRUE(session.waitFor(kTimeoutMs));
	EXPECT_TRUE(session.stopped());
	EXPECT_EQ(kWriteCode, session.code());
	EXPECT_EQ(0, probe.reads());
	EXPECT_EQ(1, probe.writes());

	session.join();
}

TEST_F(ReactorTest, UnregisterHandlerStopsDispatch)
{
	SocketPair pair;
	ASSERT_TRUE(pair.ok()) << std::strerror(errno);

	Reactor reactor;
	ProbeHandler probe(pair.read_end(), ProbeHandler::STOP_ON_READ, kReadCode);
	CancelHandler cancel(cancel_read());
	reactor.registerHandler(&probe, READ);
	reactor.registerHandler(&cancel, READ);
	reactor.unregisterHandler(pair.read_end());

	ASSERT_EQ(1, ::write(pair.write_end(), "x", 1)) << std::strerror(errno);
	RunSession session(reactor, cancel_write());

	EXPECT_FALSE(session.waitFor(kQuietMs));
	EXPECT_EQ(0, probe.reads());

	session.cancel();
	EXPECT_TRUE(session.waitFor(kTimeoutMs));
	EXPECT_EQ(kCancelCode, session.code());

	session.join();
}

TEST_F(ReactorTest, UnregisterFreesFdForReRegistration)
{
	SocketPair pair;
	ASSERT_TRUE(pair.ok()) << std::strerror(errno);

	Reactor reactor;
	ProbeHandler first(pair.read_end(), ProbeHandler::STOP_ON_READ, kReadCode);
	ProbeHandler second(
		pair.read_end(), ProbeHandler::STOP_ON_READ, kSecondCode);
	CancelHandler cancel(cancel_read());
	reactor.registerHandler(&first, READ);
	reactor.unregisterHandler(pair.read_end());
	reactor.registerHandler(&second, READ);
	reactor.registerHandler(&cancel, READ);

	ASSERT_EQ(1, ::write(pair.write_end(), "x", 1)) << std::strerror(errno);
	RunSession session(reactor, cancel_write());

	EXPECT_TRUE(session.waitFor(kTimeoutMs));
	EXPECT_TRUE(session.stopped());
	EXPECT_EQ(kSecondCode, session.code());
	EXPECT_EQ(0, first.reads());
	EXPECT_EQ(1, second.reads());

	session.join();
}

TEST_F(ReactorTest, HangupUnregistersHandlerWithoutCallback)
{
	Pipe pipe;
	ASSERT_TRUE(pipe.ok()) << std::strerror(errno);

	Reactor reactor;
	SilentHandler silent(pipe.read_end());
	CancelHandler cancel(cancel_read());
	reactor.registerHandler(&silent, READ);
	reactor.registerHandler(&cancel, READ);

	RunSession session(reactor, cancel_write());
	pipe.closeWriteEnd();

	EXPECT_FALSE(session.waitFor(kQuietMs));

	session.cancel();
	EXPECT_TRUE(session.waitFor(kTimeoutMs));
	EXPECT_EQ(kCancelCode, session.code());

	session.join();
}

TEST_F(ReactorTest, UnknownFdOperationsAreNoOps)
{
	Reactor reactor;
	reactor.unregisterHandler(12345);
	reactor.updateEvents(12345, READ);
	SUCCEED();
}

TEST_F(ReactorTest, InvalidFdRegistrationDoesNotCrash)
{
	Reactor reactor;
	ProbeHandler probe(-1, ProbeHandler::STOP_ON_READ, kReadCode);
	reactor.registerHandler(&probe, READ);
	reactor.unregisterHandler(-1);
	reactor.updateEvents(-1, WRITE);
	SUCCEED();
}
