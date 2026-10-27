/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgi_handler_test.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:23:57 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:23:57 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "http/request/HttpRequest.hpp"
#include "config/LocationConfig.hpp"
#include "server/IEventHandler.hpp"
#include "server/IRequestHandler.hpp"

#define private public
#include "cgi/CgiHandler.hpp"
#undef private

#include <algorithm>
#include <gtest/gtest.h>
#include <map>
#include <string>
#include <vector>

namespace
{
void setRequest(HttpRequest& request,
				const std::string& method,
				const std::string& uri,
				const std::string& body,
				const std::map<std::string, std::string>& headers)
{
	const_cast<std::string&>(request.getMethod()) = method;
	const_cast<std::string&>(request.getUri()) = uri;
	const_cast<std::string&>(request.getBody()) = body;
	const_cast<std::map<std::string, std::string>&>(request.getHeaders()) =
		headers;
}

std::vector<std::string> asStrings(const std::vector<char*>& environment)
{
	std::vector<std::string> result;
	for (std::size_t i = 0; environment[i] != NULL; ++i)
	{
		result.push_back(environment[i]);
	}
	return result;
}
} // namespace

TEST(CgiHandlerTest, BuildsGetEnvironment)
{
	HttpRequest request;
	LocationConfig config;
	config.root_ = "/var/www/cgi-bin";
	std::map<std::string, std::string> headers;
	headers["host"] = "example.test";
	headers["x-request-id"] = "abc-123";
	setRequest(request, "GET", "/hello.py?name=webserv", "", headers);

	CgiHandler handler;
	handler.buildEnv(request, config, "example.test", 8080);
	std::vector<std::string> environment = asStrings(handler.toCharArray());

	EXPECT_NE(
		std::find(environment.begin(), environment.end(), "REQUEST_METHOD=GET"),
		environment.end());
	EXPECT_NE(std::find(environment.begin(),
						environment.end(),
						"SCRIPT_NAME=/hello.py?name=webserv"),
			  environment.end());
	EXPECT_NE(std::find(environment.begin(),
						environment.end(),
						"SCRIPT_FILENAME=/var/www/cgi-bin"),
			  environment.end());
	EXPECT_NE(std::find(environment.begin(),
						environment.end(),
						"SERVER_NAME=example.test"),
			  environment.end());
	EXPECT_NE(
		std::find(environment.begin(), environment.end(), "SERVER_PORT=8080"),
		environment.end());
	EXPECT_NE(std::find(environment.begin(),
						environment.end(),
						"HTTP_X_REQUEST_ID=abc-123"),
			  environment.end());
}

TEST(CgiHandlerTest, BuildsPostEnvironmentAndNullTerminatesArray)
{
	HttpRequest request;
	LocationConfig config;
	config.root_ = "/srv/cgi";
	std::map<std::string, std::string> headers;
	headers["content-type"] = "application/x-www-form-urlencoded";
	headers["content-length"] = "12";
	setRequest(request, "POST", "/submit.py", "name=webserv", headers);

	CgiHandler handler;
	handler.buildEnv(request, config, "localhost", 80);
	std::vector<char*> environment = handler.toCharArray();
	std::vector<std::string> values = asStrings(environment);

	EXPECT_NE(std::find(values.begin(),
						values.end(),
						"CONTENT_TYPE=application/x-www-form-urlencoded"),
			  values.end());
	EXPECT_NE(std::find(values.begin(), values.end(), "CONTENT_LENGTH=12"),
			  values.end());
	EXPECT_EQ(environment[values.size()], static_cast<char*>(NULL));
}
