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
// Shared fixtures for the Connection suite: real socketpairs, a real Reactor,
// and a wakeup pipe that unwinds run() deterministically.
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

	void closeClientEnd()
	{
		closeEnd(fds_[1]);
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

// Everything currently readable on fd, without blocking.
std::string recvAvailable(int fd)
{
	std::string out;
	char buffer[4096];
	ssize_t got;

	while ((got = ::recv(fd, buffer, sizeof buffer, MSG_DONTWAIT)) > 0)
	{
		out.append(buffer, static_cast<std::string::size_type>(got));
	}
	return out;
}

// Patterned payload: a prefix/suffix mix-up cannot pass by coincidence.
std::string makePayload(std::size_t size)
{
	std::string payload;
	char next = 'a';

	payload.reserve(size);
	for (std::size_t i = 0; i < size; ++i)
	{
		payload.push_back(next);
		next = (next == 'z') ? 'a' : static_cast<char>(next + 1);
	}
	return payload;
}

// Shrinks the kernel send buffer so a large queue cannot flush in one send().
bool setSmallSendBuffer(int fd)
{
	const int bytes = 4096;

	return ::setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &bytes, sizeof bytes) == 0;
}

// One send() per write event: drain the peer so the next send() has room.
bool flushWithReads(Connection& connection,
					int client_fd,
					std::string& received,
					int max_iterations)
{
	for (int i = 0; i < max_iterations; ++i)
	{
		connection.handleWriteEvent();
		received += recvAvailable(client_fd);
		if (!connection.wantsWrite())
		{
			received += recvAvailable(client_fd);
			return true;
		}
		::usleep(100);
	}
	return false;
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

	WakeupHandler stopper(wakeup[0]);
	reactor().registerHandler(&connection(), READ);
	reactor().registerHandler(&stopper, READ);

	// Pending data before the loop starts: level-triggered epoll fires at once.
	const ssize_t sent = ::write(pair().client_end(), kPayload, kPayloadSize);
	ASSERT_EQ(static_cast<ssize_t>(kPayloadSize), sent) << std::strerror(errno);

	testing::internal::CaptureStdout();
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

TEST_F(ConnectionTest, PeerCloseClosesTheSocket)
{
	pair().closeClientEnd();

	connection().handleReadEvent();

	EXPECT_EQ(-1, connection().getFd());
	EXPECT_EQ(-1, ::fcntl(wired_fd(), F_GETFL));
}

TEST_F(ConnectionTest, PeerCloseUnregistersFromTheReactor)
{
	reactor().registerHandler(&connection(), READ);
	pair().closeClientEnd();

	connection().handleReadEvent();

	EXPECT_FALSE(reactor().isRegistered(wired_fd()));
}

TEST_F(ConnectionTest, PeerCloseMarksTheConnectionClosed)
{
	pair().closeClientEnd();

	connection().handleReadEvent();

	EXPECT_EQ(CLOSED, connection().getState());
}

TEST_F(ConnectionTest, DataBeforePeerCloseIsKept)
{
	const ssize_t sent = ::write(pair().client_end(), kPayload, kPayloadSize);
	ASSERT_EQ(static_cast<ssize_t>(kPayloadSize), sent) << std::strerror(errno);
	pair().closeClientEnd();

	// Level-triggered epoll delivers the buffered bytes and then the EOF as
	// separate readable events; one recv() cannot observe both.
	connection().handleReadEvent();
	EXPECT_EQ(std::string(kPayload), connection().getInBuffer());
	EXPECT_NE(-1, connection().getFd());

	connection().handleReadEvent();
	EXPECT_EQ(std::string(kPayload), connection().getInBuffer());
	EXPECT_EQ(-1, connection().getFd());
	EXPECT_EQ(CLOSED, connection().getState());
}

TEST_F(ConnectionTest, DataWithoutPeerCloseKeepsTheConnectionOpen)
{
	// EAGAIN after a successful read is not a disconnect: the subject forbids
	// inspecting errno, so a drain loop would tear this connection down.
	const ssize_t sent = ::write(pair().client_end(), kPayload, kPayloadSize);
	ASSERT_EQ(static_cast<ssize_t>(kPayloadSize), sent) << std::strerror(errno);

	connection().handleReadEvent();

	EXPECT_EQ(std::string(kPayload), connection().getInBuffer());
	EXPECT_NE(-1, connection().getFd());
	EXPECT_NE(CLOSED, connection().getState());
}

TEST_F(ConnectionTest, ReadErrorClosesTheConnection)
{
	// recv() on a regular file fails with ENOTSOCK; a read error must still
	// tear the connection down instead of leaving the descriptor dangling.
	const int broken_fd = ::open("/dev/null", O_RDONLY);
	ASSERT_GE(broken_fd, 0) << std::strerror(errno);
	Connection broken(broken_fd, config(), reactor());

	broken.handleReadEvent();

	EXPECT_EQ(-1, broken.getFd());
	EXPECT_EQ(-1, ::fcntl(broken_fd, F_GETFL));
	EXPECT_EQ(CLOSED, broken.getState());
}

TEST_F(ConnectionTest, RepeatedReadEventAfterDisconnectIsANoOp)
{
	pair().closeClientEnd();
	connection().handleReadEvent();
	ASSERT_EQ(-1, connection().getFd());

	connection().handleReadEvent();

	EXPECT_EQ(-1, connection().getFd());
	EXPECT_EQ(CLOSED, connection().getState());
}

TEST_F(ConnectionTest, ReactorHandlesDisconnectWithoutDanglingHandler)
{
	int wakeup[2] = {-1, -1};
	ASSERT_EQ(0, ::pipe(wakeup)) << std::strerror(errno);

	reactor().registerHandler(&connection(), READ);
	WakeupHandler stopper(wakeup[0]);
	reactor().registerHandler(&stopper, READ);

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

	pair().closeClientEnd();

	// The peer closing makes epoll report the socket readable; the loop must
	// close and unregister it without any further request from the test.
	bool closed = false;
	for (int waited = 0; waited < kDrainWaitMs; ++waited)
	{
		if (::fcntl(wired_fd(), F_GETFL) == -1)
		{
			closed = true;
			break;
		}
		::usleep(1000);
	}

	const char wake = 'x';
	(void) ::write(wakeup[1], &wake, 1);
	loop.join();

	EXPECT_TRUE(closed) << "the reactor never delivered the disconnect";
	EXPECT_FALSE(reactor().isRegistered(wired_fd()));
	EXPECT_EQ(CLOSED, connection().getState());

	::close(wakeup[0]);
	::close(wakeup[1]);
}

TEST_F(ConnectionTest, ReactorClosesConnectionAfterDataAndPeerClose)
{
	int wakeup[2] = {-1, -1};
	ASSERT_EQ(0, ::pipe(wakeup)) << std::strerror(errno);

	reactor().registerHandler(&connection(), READ);
	WakeupHandler stopper(wakeup[0]);
	reactor().registerHandler(&stopper, READ);

	const ssize_t sent = ::write(pair().client_end(), kPayload, kPayloadSize);
	ASSERT_EQ(static_cast<ssize_t>(kPayloadSize), sent) << std::strerror(errno);
	pair().closeClientEnd();

	testing::internal::CaptureStdout();
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

	// epoll reports EPOLLIN and EPOLLHUP together here: the loop must let the
	// handler consume the bytes and observe the EOF, not unregister it early.
	bool closed = false;
	for (int waited = 0; waited < kDrainWaitMs; ++waited)
	{
		if (::fcntl(wired_fd(), F_GETFL) == -1)
		{
			closed = true;
			break;
		}
		::usleep(1000);
	}

	const char wake = 'x';
	(void) ::write(wakeup[1], &wake, 1);
	loop.join();
	std::cout.flush();
	testing::internal::GetCapturedStdout();

	EXPECT_TRUE(closed)
		<< "the reactor stranded the connection on a data+EOF event";
	EXPECT_FALSE(reactor().isRegistered(wired_fd()));
	EXPECT_EQ(std::string(kPayload), connection().getInBuffer());
	EXPECT_EQ(CLOSED, connection().getState());

	::close(wakeup[0]);
	::close(wakeup[1]);
}

TEST_F(ConnectionTest, QueueOutputHoldsTheBytesUntilThePeerTakesThem)
{
	reactor().registerHandler(&connection(), READ);
	EXPECT_FALSE(connection().wantsWrite());

	connection().queueOutput(kPayload);

	EXPECT_EQ(std::string(kPayload), connection().getOutBuffer());
	EXPECT_TRUE(connection().wantsWrite());
}

TEST_F(ConnectionTest, QueueOutputAppendsAcrossCalls)
{
	reactor().registerHandler(&connection(), READ);

	connection().queueOutput("hello ");
	connection().queueOutput("world\n");

	EXPECT_EQ(std::string("hello world\n"), connection().getOutBuffer());
}

TEST_F(ConnectionTest, WriteEventFlushesQueuedBytesToThePeer)
{
	reactor().registerHandler(&connection(), READ);
	connection().queueOutput(kPayload);

	connection().handleWriteEvent();

	EXPECT_EQ(std::string(kPayload), recvAvailable(pair().client_end()));
	EXPECT_TRUE(connection().getOutBuffer().empty());
	EXPECT_FALSE(connection().wantsWrite());
	EXPECT_NE(-1, connection().getFd());
}

TEST_F(ConnectionTest, WriteEventWithNothingQueuedIsANoOp)
{
	// send(fd, "", 0) returns 0; without an empty-buffer guard that would look
	// like an error and kill a healthy connection.
	reactor().registerHandler(&connection(), READ);

	connection().handleWriteEvent();

	EXPECT_EQ(std::string(), recvAvailable(pair().client_end()));
	EXPECT_NE(-1, connection().getFd());
	EXPECT_NE(CLOSED, connection().getState());
}

TEST_F(ConnectionTest, PartialWriteKeepsTheUnsentSuffix)
{
	reactor().registerHandler(&connection(), READ);
	ASSERT_TRUE(setSmallSendBuffer(wired_fd())) << std::strerror(errno);

	const std::string payload = makePayload(1 << 20);
	connection().queueOutput(payload);

	connection().handleWriteEvent();

	const std::string received = recvAvailable(pair().client_end());
	ASSERT_GT(received.size(), 0u) << "the first write event flushed nothing";
	ASSERT_LT(received.size(), payload.size())
		<< "the payload fit in one send(); no partial write was forced";
	EXPECT_EQ(payload.substr(0, received.size()), received);
	EXPECT_EQ(payload.substr(received.size()), connection().getOutBuffer());
	EXPECT_TRUE(connection().wantsWrite());
}

TEST_F(ConnectionTest, RepeatedWriteEventsEventuallyFlushTheWholePayload)
{
	reactor().registerHandler(&connection(), READ);
	ASSERT_TRUE(setSmallSendBuffer(wired_fd())) << std::strerror(errno);

	const std::string payload = makePayload(1 << 20);
	connection().queueOutput(payload);

	std::string received;
	ASSERT_TRUE(
		flushWithReads(connection(), pair().client_end(), received, 100000))
		<< "the write path stopped making progress";
	EXPECT_EQ(payload, received);
	EXPECT_TRUE(connection().getOutBuffer().empty());
	EXPECT_NE(-1, connection().getFd());
	EXPECT_NE(CLOSED, connection().getState());
}

TEST_F(ConnectionTest, WriteErrorClosesTheConnection)
{
	// send() on a non-socket fails with ENOTSOCK; like the read path, a write
	// error must tear the connection down instead of leaving a dangling fd.
	const int broken_fd = ::open("/dev/null", O_WRONLY);
	ASSERT_GE(broken_fd, 0) << std::strerror(errno);
	Connection broken(broken_fd, config(), reactor());

	broken.queueOutput(kPayload);
	broken.handleWriteEvent();

	EXPECT_EQ(-1, broken.getFd());
	EXPECT_EQ(-1, ::fcntl(broken_fd, F_GETFL));
	EXPECT_EQ(CLOSED, broken.getState());
}

TEST_F(ConnectionTest, RepeatedWriteEventAfterDisconnectIsANoOp)
{
	const int broken_fd = ::open("/dev/null", O_WRONLY);
	ASSERT_GE(broken_fd, 0) << std::strerror(errno);
	Connection broken(broken_fd, config(), reactor());
	broken.queueOutput(kPayload);
	broken.handleWriteEvent();
	ASSERT_EQ(-1, broken.getFd());

	broken.handleWriteEvent();

	EXPECT_EQ(-1, broken.getFd());
	EXPECT_EQ(CLOSED, broken.getState());
}

TEST_F(ConnectionTest, QueueOutputArmsWriteInterest)
{
	reactor().registerHandler(&connection(), READ);

	connection().queueOutput(kPayload);

	EXPECT_EQ(EPOLLIN | EPOLLOUT, reactor().eventMask(wired_fd()));
}

TEST_F(ConnectionTest, DrainingTheQueueDisarmsWriteInterest)
{
	reactor().registerHandler(&connection(), READ);
	connection().queueOutput(kPayload);

	connection().handleWriteEvent();

	EXPECT_TRUE(connection().getOutBuffer().empty());
	EXPECT_EQ(EPOLLIN, reactor().eventMask(wired_fd()));
}

TEST_F(ConnectionTest, ReactorFlushesQueuedOutputEndToEnd)
{
	int wakeup[2] = {-1, -1};
	ASSERT_EQ(0, ::pipe(wakeup)) << std::strerror(errno);

	reactor().registerHandler(&connection(), READ);
	WakeupHandler stopper(wakeup[0]);
	reactor().registerHandler(&stopper, READ);

	const std::string payload = makePayload(64 * 1024);
	connection().queueOutput(payload);

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

	// Without EPOLLOUT armed, the reactor never delivers a write event and
	// this loop times out with only a fraction of the payload.
	std::string received;
	for (int waited = 0;
		 waited < kDrainWaitMs && received.size() < payload.size();
		 ++waited)
	{
		received += recvAvailable(pair().client_end());
		::usleep(1000);
	}

	const char wake = 'x';
	(void) ::write(wakeup[1], &wake, 1);
	loop.join();

	EXPECT_EQ(payload, received) << "the reactor never flushed the output";
	EXPECT_TRUE(connection().getOutBuffer().empty());
	EXPECT_FALSE(connection().wantsWrite());
	EXPECT_EQ(EPOLLIN, reactor().eventMask(wired_fd()));

	::close(wakeup[0]);
	::close(wakeup[1]);
}
