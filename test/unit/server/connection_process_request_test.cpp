/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   connection_process_request_test.cpp              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/18 00:00:00 by hermarti          #+#    #+#             */
/*   Updated: 2026/11/18 00:00:00 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "config/ServerConfig.hpp"
#include "http/response/HttpResponse.hpp"
#include "server/Connection.hpp"
#include "server/Reactor.hpp"

#include <gtest/gtest.h>

#include <cerrno>
#include <cstddef>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

namespace
{
const char* const kSimpleGet = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
const char* const kSecondGet =
	"GET /second HTTP/1.1\r\nHost: localhost\r\n\r\n";
const char* const kThirdGet = "GET /third HTTP/1.1\r\nHost: localhost\r\n\r\n";

// Owns a socketpair whose first end is handed to Connection.
class PairFds
{
  public:
	PairFds() : ok_(false)
	{
		if (::socketpair(AF_UNIX, SOCK_STREAM, 0, fds_) == 0)
		{
			ok_ = true;
		}
		else
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
	PairFds(const PairFds& other);
	PairFds& operator=(const PairFds& other);

	static void closeEnd(int& fd)
	{
		if (fd < 0)
		{
			return;
		}
		if (::close(fd) == -1 && errno != EBADF)
		{
			// Connection may already have closed its own end.
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

bool writeAll(int fd, const std::string& data)
{
	std::size_t sent = 0;

	while (sent < data.size())
	{
		const ssize_t written =
			::write(fd, data.data() + sent, data.size() - sent);
		if (written <= 0)
		{
			return false;
		}
		sent += static_cast<std::size_t>(written);
	}
	return true;
}
} // namespace

class ConnectionProcessRequestTest : public ::testing::Test
{
  protected:
	ConnectionProcessRequestTest() : connection_(nullptr)
	{
	}

	void SetUp() override
	{
		ASSERT_TRUE(pair_.ok()) << std::strerror(errno);
		ASSERT_TRUE(setNonBlocking(pair_.read_end())) << std::strerror(errno);
		connection_ = new Connection(pair_.read_end(), config_, reactor_);
	}

	void TearDown() override
	{
		delete connection_;
		connection_ = nullptr;
	}

	// One client write followed by one readable event, like Reactor would do.
	void deliver(const std::string& data)
	{
		ASSERT_TRUE(writeAll(pair_.client_end(), data)) << std::strerror(errno);
		connection_->handleReadEvent();
	}

	Connection& connection()
	{
		return *connection_;
	}

	Reactor& reactor()
	{
		return reactor_;
	}

  private:
	PairFds pair_;
	ServerConfig config_;
	Reactor reactor_;
	Connection* connection_;
};

TEST_F(ConnectionProcessRequestTest, CompleteRequestStoresARoutedResponse)
{
	deliver(kSimpleGet);

	const HttpResponse* response = connection().getCurrentResponse();
	ASSERT_NE(nullptr, response);
	EXPECT_EQ(200, response->getStatusCode());
	EXPECT_TRUE(connection().getInBuffer().empty());
	EXPECT_TRUE(connection().getOutBuffer().empty());
	EXPECT_NE(-1, connection().getFd());
}

TEST_F(ConnectionProcessRequestTest, IncompleteRequestWaitsForMoreBytes)
{
	const std::string partial = "GET / HTTP/1.1\r\nHost: local";
	deliver(partial);

	EXPECT_EQ(nullptr, connection().getCurrentResponse());
	EXPECT_EQ(partial, connection().getInBuffer());
	EXPECT_TRUE(connection().getOutBuffer().empty());
	EXPECT_NE(-1, connection().getFd());
}

TEST_F(ConnectionProcessRequestTest, PipelinedRequestLeavesTheRemainderBuffered)
{
	deliver(std::string(kSimpleGet) + kSecondGet);

	const HttpResponse* response = connection().getCurrentResponse();
	ASSERT_NE(nullptr, response);
	EXPECT_EQ(200, response->getStatusCode());
	EXPECT_EQ(std::string(kSecondGet), connection().getInBuffer());
}

TEST_F(ConnectionProcessRequestTest,
	   BufferedPipelinedRequestIsProcessedOnTheNextRead)
{
	deliver(std::string(kSimpleGet) + kSecondGet);
	deliver(kThirdGet);

	const HttpResponse* response = connection().getCurrentResponse();
	ASSERT_NE(nullptr, response);
	EXPECT_EQ(200, response->getStatusCode());
	EXPECT_EQ(std::string(kThirdGet), connection().getInBuffer());
}

TEST_F(ConnectionProcessRequestTest, MalformedRequestLineStoresABadRequest)
{
	deliver("GET /\r\n\r\n");

	const HttpResponse* response = connection().getCurrentResponse();
	ASSERT_NE(nullptr, response);
	EXPECT_EQ(400, response->getStatusCode());
	EXPECT_TRUE(connection().getInBuffer().empty());
	EXPECT_TRUE(connection().getOutBuffer().empty());
	EXPECT_NE(-1, connection().getFd());
}

TEST_F(ConnectionProcessRequestTest, MissingHostStoresABadRequest)
{
	deliver("GET / HTTP/1.1\r\n\r\n");

	const HttpResponse* response = connection().getCurrentResponse();
	ASSERT_NE(nullptr, response);
	EXPECT_EQ(400, response->getStatusCode());
	EXPECT_TRUE(connection().getOutBuffer().empty());
	EXPECT_NE(-1, connection().getFd());
}

TEST_F(ConnectionProcessRequestTest, RequestWithBodyIsFullyConsumed)
{
	const std::string request = "POST /submit HTTP/1.1\r\nHost: localhost\r\n"
								"Content-Length: 4\r\n\r\nbody";
	deliver(request);

	const HttpResponse* response = connection().getCurrentResponse();
	ASSERT_NE(nullptr, response);
	EXPECT_EQ(200, response->getStatusCode());
	EXPECT_TRUE(connection().getInBuffer().empty());
	EXPECT_TRUE(connection().getOutBuffer().empty());
}

TEST_F(ConnectionProcessRequestTest,
	   BodyAboveTheConfiguredLimitStoresPayloadTooLarge)
{
	ServerConfig limited;
	limited.client_max_body_size_ = 10;
	PairFds body_pair;
	ASSERT_TRUE(body_pair.ok()) << std::strerror(errno);
	ASSERT_TRUE(setNonBlocking(body_pair.read_end())) << std::strerror(errno);
	Connection limited_connection(body_pair.read_end(), limited, reactor());

	const std::string request = "POST /upload HTTP/1.1\r\nHost: localhost\r\n"
								"Content-Length: 11\r\n\r\n" +
								std::string(11, 'x');
	ASSERT_TRUE(writeAll(body_pair.client_end(), request))
		<< std::strerror(errno);
	limited_connection.handleReadEvent();

	const HttpResponse* response = limited_connection.getCurrentResponse();
	ASSERT_NE(nullptr, response);
	EXPECT_EQ(413, response->getStatusCode());
	EXPECT_TRUE(limited_connection.getOutBuffer().empty());
	EXPECT_NE(-1, limited_connection.getFd());
}
