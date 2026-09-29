/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server_test.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:25:43 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:25:44 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server/Server.hpp"

#include <gtest/gtest.h>

#include <signal.h>

TEST(ServerTest, HeaderCompiles)
{
	SUCCEED();
}

TEST(ServerTest, SigpipeIsIgnoredAtStartup)
{
	std::vector<ServerConfig> configs;
	Server server(configs);
	struct sigaction current;

	server.init();
	ASSERT_EQ(0, sigaction(SIGPIPE, NULL, &current));
	EXPECT_EQ(SIG_IGN, current.sa_handler);
}
