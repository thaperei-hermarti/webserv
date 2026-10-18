/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   acceptor_test.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:24:57 by hermarti          #+#    #+#             */
/*   Updated: 2026/10/05 00:00:00 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "config/ServerConfig.hpp"
#include "server/Acceptor.hpp"
#include "server/Reactor.hpp"

#include <gtest/gtest.h>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

TEST(AcceptorTest, CreatesANonBlockingListeningSocket)
{
	Reactor reactor;
	ServerConfig config;
	Acceptor acceptor("127.0.0.1", 0, config, reactor);

	int fd = acceptor.getFd();
	ASSERT_GE(fd, 0);

	int flags = fcntl(fd, F_GETFL, 0);
	ASSERT_NE(-1, flags);
	EXPECT_NE(0, flags & O_NONBLOCK);

	int accepting = 0;
	socklen_t length = sizeof(accepting);
	ASSERT_EQ(0,
			  getsockopt(fd, SOL_SOCKET, SO_ACCEPTCONN, &accepting, &length));
	EXPECT_EQ(1, accepting);
}

TEST(AcceptorTest, SetsReuseAddressBeforeBinding)
{
	Reactor reactor;
	ServerConfig config;
	Acceptor acceptor("127.0.0.1", 0, config, reactor);

	int fd = acceptor.getFd();
	ASSERT_GE(fd, 0);

	int reuse = 0;
	socklen_t length = sizeof(reuse);
	ASSERT_EQ(0, getsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, &length));
	EXPECT_EQ(1, reuse);
}

TEST(AcceptorTest, IsReachableThroughTheBoundEndpoint)
{
	Reactor reactor;
	ServerConfig config;
	Acceptor acceptor("127.0.0.1", 0, config, reactor);

	int fd = acceptor.getFd();
	ASSERT_GE(fd, 0);

	struct sockaddr_in address;
	socklen_t length = sizeof(address);
	ASSERT_EQ(
		0,
		getsockname(fd, reinterpret_cast<struct sockaddr*>(&address), &length));
	EXPECT_NE(0, ntohs(address.sin_port));

	int client = socket(AF_INET, SOCK_STREAM, 0);
	ASSERT_GE(client, 0);
	EXPECT_EQ(
		0,
		connect(client, reinterpret_cast<struct sockaddr*>(&address), length));
	close(client);
}

TEST(AcceptorTest, NeverWantsWriteEvents)
{
	Reactor reactor;
	ServerConfig config;
	Acceptor acceptor("127.0.0.1", 0, config, reactor);

	EXPECT_FALSE(acceptor.wantsWrite());
}
