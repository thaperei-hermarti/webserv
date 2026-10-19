/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   connection_test.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:25:07 by hermarti          #+#    #+#             */
/*   Updated: 2026/10/19 00:00:00 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "config/ServerConfig.hpp"
#include "server/Connection.hpp"
#include "server/EventType.hpp"
#include "server/Reactor.hpp"

#include <gtest/gtest.h>

#include <cerrno>
#include <cstring>
#include <exception>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

namespace
{
// On read-ready, recv() into the input buffer and echo the byte count to
// stdout (issue #164). Disconnect handling (recv() 0/error) is issue #165 and
// is deliberately not exercised here.
const int kDrainWaitMs = 2000;
const char kPayload[] = "hello\n";
const std::size_t kPayloadSize = 6;

// Thrown from a wakeup handler to unwind Reactor::run() on the worker thread.
class LoopFinished : public std::exception
{
  public:
	const char* what() const noexcept override
	{
		return "connection test: stop the loop";
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

// True once every byte written to the pair has been recv()'d away.
bool isDrained(int fd)
{
	char probe = 0;
	const ssize_t got = ::recv(fd, &probe, 1, MSG_DONTWAIT | MSG_PEEK);
	return got == -1 && errno == EAGAIN;
}

bool waitForDrain(int fd, int timeout_ms)
{
	for (int waited = 0; waited < timeout_ms; ++waited)
	{
		if (isDrained(fd))
		{
			return true;
		}
		::usleep(1000);
	}
	return isDrained(fd);
}
} // namespace

class ConnectionTest : public ::testing::Test
{
  protected:
	void SetUp() override
	{
		ASSERT_TRUE(pair_.ok()) << std::strerror(errno);
		// Production fds behind epoll are non-blocking; this also makes a
		// missing EAGAIN check fail fast instead of hanging the suite.
		ASSERT_TRUE(setNonBlocking(pair_.read_end())) << std::strerror(errno);
		connection_ = new Connection(pair_.read_end(), config_);
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

  private:
	ServerConfig config_;
	PairFds pair_;
	Connection* connection_ = nullptr;
};

TEST_F(ConnectionTest, GetFdReturnsTheWiredSocket)
{
	EXPECT_EQ(pair().read_end(), connection().getFd());
}

TEST_F(ConnectionTest, ReadEventConsumesThePendingBytes)
{
	const ssize_t sent = ::write(pair().client_end(), kPayload, kPayloadSize);
	ASSERT_EQ(static_cast<ssize_t>(kPayloadSize), sent) << std::strerror(errno);

	connection().handleReadEvent();

	EXPECT_TRUE(isDrained(pair().read_end()))
		<< "handleReadEvent() must recv() the pending bytes off the socket";
}

TEST_F(ConnectionTest, ReadEventKeepsTheBytesInItsInputBuffer)
{
	// This test drives one small read-only accessor:
	//   const std::string& getInBuffer() const;
	const ssize_t sent = ::write(pair().client_end(), kPayload, kPayloadSize);
	ASSERT_EQ(static_cast<ssize_t>(kPayloadSize), sent) << std::strerror(errno);

	connection().handleReadEvent();

	EXPECT_EQ(std::string(kPayload), connection().getInBuffer());
}

TEST_F(ConnectionTest, ReadEventEchoesTheByteCountToStdout)
{
	const ssize_t sent = ::write(pair().client_end(), kPayload, kPayloadSize);
	ASSERT_EQ(static_cast<ssize_t>(kPayloadSize), sent) << std::strerror(errno);

	testing::internal::CaptureStdout();
	connection().handleReadEvent();
	std::cout.flush();
	const std::string echoed = testing::internal::GetCapturedStdout();

	// Any echo format is acceptable; only the byte count must show up.
	EXPECT_NE(std::string::npos, echoed.find("6"))
		<< "expected the byte count on stdout, got: [" << echoed << "]";
}

TEST_F(ConnectionTest, ReadEventWithNothingPendingIsSafeAndSilent)
{
	testing::internal::CaptureStdout();
	connection().handleReadEvent();
	std::cout.flush();
	const std::string echoed = testing::internal::GetCapturedStdout();

	EXPECT_TRUE(echoed.empty())
		<< "nothing was pending, so nothing should be echoed: [" << echoed
		<< "]";
}

TEST_F(ConnectionTest, ReadEventDoesNotStartTheWritePathYet)
{
	const ssize_t sent = ::write(pair().client_end(), kPayload, kPayloadSize);
	ASSERT_EQ(static_cast<ssize_t>(kPayloadSize), sent) << std::strerror(errno);

	connection().handleReadEvent();

	// #164 is read-only: no HTTP parsing, no response queued for writing.
	EXPECT_FALSE(connection().wantsWrite());
}

TEST_F(ConnectionTest, ReactorDispatchesReadEventsToConnection)
{
	int wakeup[2] = {-1, -1};
	ASSERT_EQ(0, ::pipe(wakeup)) << std::strerror(errno);

	Reactor reactor;
	WakeupHandler stopper(wakeup[0]);
	reactor.registerHandler(&connection(), READ);
	reactor.registerHandler(&stopper, READ);

	// Pending data before the loop starts: level-triggered epoll fires at once.
	const ssize_t sent = ::write(pair().client_end(), kPayload, kPayloadSize);
	ASSERT_EQ(static_cast<ssize_t>(kPayloadSize), sent) << std::strerror(errno);

	testing::internal::CaptureStdout();
	std::thread loop(
		[&reactor]()
		{
			try
			{
				reactor.run();
			}
			catch (const LoopFinished&)
			{
			}
		});

	const bool drained = waitForDrain(pair().read_end(), kDrainWaitMs);

	const char wake = 'x';
	(void) ::write(wakeup[1], &wake, 1);
	loop.join();
	std::cout.flush();
	const std::string echoed = testing::internal::GetCapturedStdout();

	EXPECT_TRUE(drained) << "the reactor never delivered a read event";
	EXPECT_NE(std::string::npos, echoed.find("6"))
		<< "expected the byte count on stdout, got: [" << echoed << "]";

	::close(wakeup[0]);
	::close(wakeup[1]);
}
