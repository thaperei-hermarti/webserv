/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   connection_timeout_test.cpp                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 00:00:00 by hermarti          #+#    #+#             */
/*   Updated: 2026/11/12 00:00:00 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "config/ServerConfig.hpp"
#include "server/ConnState.hpp"
#include "server/Connection.hpp"
#include "server/EventType.hpp"
#include "server/Reactor.hpp"

#include <gtest/gtest.h>

#include <cerrno>
#include <cstring>
#include <ctime>
#include <exception>
#include <fcntl.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

namespace
{
const time_t kIdleTimeout = 5;
const int kSweepWaitMs = 2500;

// Thrown from a wakeup handler to unwind Reactor::run() on the worker thread.
class LoopFinished : public std::exception
{
  public:
	const char* what() const noexcept override
	{
		return "timeout test: stop the loop";
	}
};

// Wakes a blocked run() so the worker thread can be joined deterministically.
class WakeupHandler : public IEventHandler
{
  public:
	explicit WakeupHandler(int fd) : fd_(fd)
	{
	}

	int getFd() const
	{
		return fd_;
	}

	void handleReadEvent()
	{
		throw LoopFinished();
	}

	void handleWriteEvent()
	{
	}

	void handleTimeout()
	{
	}

  private:
	int fd_;
};

// A generic handler that relies on the IEventHandler default: no timeout.
// This is what keeps non-Connection handlers (e.g. Acceptor) out of the sweep.
class ImmortalHandler : public IEventHandler
{
  public:
	explicit ImmortalHandler(int fd) : fd_(fd), timeouts_(0)
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

	void handleTimeout()
	{
		++timeouts_;
	}

	int timeouts() const
	{
		return timeouts_;
	}

  private:
	int fd_;
	int timeouts_;
};

// One end is handed to the Connection, the other end plays the client.
class PairFds
{
  public:
	PairFds() : ok_(::socketpair(AF_UNIX, SOCK_STREAM, 0, fds_) == 0)
	{
		if (!ok_)
		{
			fds_[0] = -1;
			fds_[1] = -1;
		}
	}

	~PairFds()
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

	int client_end() const
	{
		return fds_[1];
	}

  private:
	static void closeEnd(int& fd)
	{
		if (fd < 0)
		{
			return;
		}
		if (::close(fd) == -1 && errno != EBADF)
		{
			// The Connection may already have closed its own end.
		}
		fd = -1;
	}

	int fds_[2];
	bool ok_;
};

bool setNonBlocking(int fd)
{
	const int flags = ::fcntl(fd, F_GETFL, 0);
	if (flags == -1)
	{
		return false;
	}
	return ::fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1;
}

// Guarantees the wall clock moved at least `seconds` past `from`, without
// assuming that sleep() cannot be interrupted.
void waitForClockAdvance(time_t from, int seconds)
{
	while (std::time(NULL) < from + seconds)
	{
		::usleep(1000);
	}
}
} // namespace

class ConnectionTimeoutTest : public ::testing::Test
{
  protected:
	void SetUp() override
	{
		ASSERT_TRUE(pair_.ok()) << std::strerror(errno);
		ASSERT_TRUE(setNonBlocking(pair_.read_end())) << std::strerror(errno);
		config_.idle_timeout_ = kIdleTimeout;
		connection_ = new Connection(pair_.read_end(), config_, reactor_);
	}

	void TearDown() override
	{
		// Destroy the Connection before the pair closes the descriptors.
		delete connection_;
		connection_ = nullptr;
	}

	Connection& connection()
	{
		return *connection_;
	}

	PairFds& pair()
	{
		return pair_;
	}

	ServerConfig& config()
	{
		return config_;
	}

	Reactor& reactor()
	{
		return reactor_;
	}

	int wired_fd() const
	{
		return pair_.read_end();
	}

  private:
	ServerConfig config_;
	Reactor reactor_;
	PairFds pair_;
	Connection* connection_ = nullptr;
};

TEST_F(ConnectionTimeoutTest, FreshConnectionIsNotTimedOutBeforeTheDeadline)
{
	const time_t now = std::time(NULL);

	EXPECT_FALSE(connection().isTimedOut(now));
	EXPECT_FALSE(connection().isTimedOut(now + kIdleTimeout - 2));
}

TEST_F(ConnectionTimeoutTest, ConnectionIsTimedOutAtAndAfterTheDeadline)
{
	const time_t now = std::time(NULL);

	EXPECT_TRUE(connection().isTimedOut(now + kIdleTimeout));
	EXPECT_TRUE(connection().isTimedOut(now + kIdleTimeout + 1));
}

TEST_F(ConnectionTimeoutTest, SweepBeforeTheDeadlineKeepsTheConnectionOpen)
{
	reactor().registerHandler(&connection(), READ);
	const time_t now = std::time(NULL);

	reactor().sweepTimeouts(now + kIdleTimeout - 2);

	EXPECT_NE(-1, connection().getFd());
	EXPECT_NE(CLOSED, connection().getState());
	EXPECT_TRUE(reactor().isRegistered(wired_fd()));
}

TEST_F(ConnectionTimeoutTest, SweepAfterTheDeadlineClosesAndUnregisters)
{
	reactor().registerHandler(&connection(), READ);
	const time_t now = std::time(NULL);

	reactor().sweepTimeouts(now + kIdleTimeout + 1);

	EXPECT_EQ(-1, connection().getFd());
	EXPECT_EQ(CLOSED, connection().getState());
	EXPECT_FALSE(reactor().isRegistered(wired_fd()));
	EXPECT_EQ(-1, ::fcntl(wired_fd(), F_GETFL));
	EXPECT_FALSE(connection().isTimedOut(now + kIdleTimeout + 1))
		<< "a closed connection must not report a timeout";
}

TEST_F(ConnectionTimeoutTest, HandleTimeoutClosesAndUnregistersTheConnection)
{
	// The Reactor calls handleTimeout() only after isTimedOut(now) returned
	// true; the hook itself is an unconditional, idempotent teardown.
	reactor().registerHandler(&connection(), READ);

	connection().handleTimeout();

	EXPECT_EQ(-1, connection().getFd());
	EXPECT_EQ(CLOSED, connection().getState());
	EXPECT_FALSE(reactor().isRegistered(wired_fd()));
	EXPECT_EQ(-1, ::fcntl(wired_fd(), F_GETFL));

	connection().handleTimeout();
	EXPECT_EQ(-1, connection().getFd());
	EXPECT_EQ(CLOSED, connection().getState());
}

TEST_F(ConnectionTimeoutTest, SweepClosesEveryExpiredConnection)
{
	PairFds second_pair;
	PairFds third_pair;
	ASSERT_TRUE(second_pair.ok()) << std::strerror(errno);
	ASSERT_TRUE(third_pair.ok()) << std::strerror(errno);
	ASSERT_TRUE(setNonBlocking(second_pair.read_end())) << std::strerror(errno);
	ASSERT_TRUE(setNonBlocking(third_pair.read_end())) << std::strerror(errno);
	Connection second(second_pair.read_end(), config(), reactor());
	Connection third(third_pair.read_end(), config(), reactor());

	reactor().registerHandler(&connection(), READ);
	reactor().registerHandler(&second, READ);
	reactor().registerHandler(&third, READ);
	const time_t now = std::time(NULL);

	reactor().sweepTimeouts(now + kIdleTimeout + 1);

	// Erasing the current map entry while iterating must keep the sweep
	// walking the remaining handlers.
	EXPECT_EQ(-1, connection().getFd());
	EXPECT_EQ(-1, second.getFd());
	EXPECT_EQ(-1, third.getFd());
	EXPECT_FALSE(reactor().isRegistered(wired_fd()));
	EXPECT_FALSE(reactor().isRegistered(second_pair.read_end()));
	EXPECT_FALSE(reactor().isRegistered(third_pair.read_end()));
}

TEST_F(ConnectionTimeoutTest, SweepSparesConnectionsWithRecentActivity)
{
	PairFds idle_pair;
	ASSERT_TRUE(idle_pair.ok()) << std::strerror(errno);
	ASSERT_TRUE(setNonBlocking(idle_pair.read_end())) << std::strerror(errno);
	Connection idle(idle_pair.read_end(), config(), reactor());

	const time_t before = std::time(NULL);
	waitForClockAdvance(before, 1);

	const char payload[] = "x";
	ASSERT_EQ(1, ::write(pair().client_end(), payload, 1))
		<< std::strerror(errno);
	connection().handleReadEvent();

	reactor().registerHandler(&connection(), READ);
	reactor().registerHandler(&idle, READ);

	reactor().sweepTimeouts(before + kIdleTimeout);

	EXPECT_NE(-1, connection().getFd())
		<< "a read must refresh the idle deadline";
	EXPECT_TRUE(reactor().isRegistered(wired_fd()));
	EXPECT_EQ(-1, idle.getFd()) << "an idle connection must be swept";
	EXPECT_FALSE(reactor().isRegistered(idle_pair.read_end()));
}

TEST_F(ConnectionTimeoutTest, TheConfiguredTimeoutDecidesTheDeadline)
{
	ServerConfig patient_config;
	patient_config.idle_timeout_ = kIdleTimeout + 100;
	PairFds patient_pair;
	ASSERT_TRUE(patient_pair.ok()) << std::strerror(errno);
	ASSERT_TRUE(setNonBlocking(patient_pair.read_end()))
		<< std::strerror(errno);
	Connection patient(patient_pair.read_end(), patient_config, reactor());

	reactor().registerHandler(&connection(), READ);
	reactor().registerHandler(&patient, READ);
	const time_t now = std::time(NULL);

	reactor().sweepTimeouts(now + kIdleTimeout + 1);

	EXPECT_EQ(-1, connection().getFd());
	EXPECT_NE(-1, patient.getFd());
	EXPECT_TRUE(reactor().isRegistered(patient_pair.read_end()));
}

TEST_F(ConnectionTimeoutTest, HandlersWithoutATimeoutAreNeverSwept)
{
	PairFds pair;
	ASSERT_TRUE(pair.ok()) << std::strerror(errno);
	ImmortalHandler immortal(pair.read_end());
	reactor().registerHandler(&immortal, READ);
	const time_t now = std::time(NULL);

	reactor().sweepTimeouts(now + 3600);

	EXPECT_EQ(0, immortal.timeouts());
	EXPECT_TRUE(reactor().isRegistered(pair.read_end()));
}

TEST_F(ConnectionTimeoutTest, ReactorSweepsIdleConnectionsWhileBlocked)
{
	int wakeup[2] = {-1, -1};
	ASSERT_EQ(0, ::pipe(wakeup)) << std::strerror(errno);

	ServerConfig idle_config;
	idle_config.idle_timeout_ = 1;
	PairFds idle_pair;
	ASSERT_TRUE(idle_pair.ok()) << std::strerror(errno);
	ASSERT_TRUE(setNonBlocking(idle_pair.read_end())) << std::strerror(errno);
	Connection idle(idle_pair.read_end(), idle_config, reactor());
	reactor().registerHandler(&idle, READ);

	WakeupHandler stopper(wakeup[0]);
	reactor().registerHandler(&stopper, READ);

	const time_t started = std::time(NULL);
	std::thread loop(
		[this]()
		{
			try
			{
				reactor().run();
			}
			catch (const LoopFinished&)
			{
			}
		});

	// The connection becomes idle only after the reactor is already blocked
	// in epoll_wait(): a bounded wait is the only way the sweep can run. No
	// socket traffic is generated, so an epoll_wait(-1) implementation hangs
	// here and fails the expectation below.
	waitForClockAdvance(started, 2);
	bool swept_while_blocked = false;
	for (int waited = 0; waited < kSweepWaitMs && idle.getFd() >= 0; ++waited)
	{
		::usleep(1000);
	}
	swept_while_blocked = idle.getFd() < 0;

	const char wake = 'x';
	(void) ::write(wakeup[1], &wake, 1);
	loop.join();

	EXPECT_TRUE(swept_while_blocked)
		<< "run() must wake up periodically (<= 1s) to sweep timeouts";
	EXPECT_EQ(-1, idle.getFd());
	EXPECT_EQ(CLOSED, idle.getState());
	EXPECT_FALSE(reactor().isRegistered(idle_pair.read_end()));

	reactor().unregisterHandler(wakeup[0]);
	::close(wakeup[0]);
	::close(wakeup[1]);
}
