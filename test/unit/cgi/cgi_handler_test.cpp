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

#include <map>
#include <string>

#define private public
#include "http/response/HttpResponse.hpp"
#undef private

#include "server/IRequestHandler.hpp"

#define private public
#include "cgi/CgiHandler.hpp"
#undef private

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <gtest/gtest.h>
#include <unistd.h>
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

std::string createTempCgiDirectory()
{
	char directory[] = "/tmp/webserv-cgi-test-XXXXXX";
	char* result = mkdtemp(directory);
	return result == NULL ? "" : result;
}

bool writeCgiScript(const std::string& path, const std::string& body)
{
	std::ofstream script(path.c_str());
	if (!script)
	{
		return false;
	}
	script << "#!/bin/sh\nprintf 'Content-Type: text/plain\\r\\n"
			  "\\r\\n"
		   << body << "\\n'\n";
	return script.good();
}

void removeCgiFiles(const std::string& directory,
					const std::string& first_script,
					const std::string& second_script)
{
	unlink(first_script.c_str());
	if (!second_script.empty())
	{
		unlink(second_script.c_str());
	}
	rmdir(directory.c_str());
}
} // namespace

TEST(CgiHandlerTest, BuildsGetEnvironment)
{
	HttpRequest request;
	LocationConfig config;
	config.root_ = "/var/www/cgi-bin";
	std::map<std::string, std::string> headers;
	headers["host"] = "example.test:8080";
	headers["x-request-id"] = "abc-123";
	setRequest(request, "GET", "/hello.py?name=webserv", "", headers);

	CgiHandler handler;
	handler.buildEnv(request, config);
	std::vector<std::string> environment = asStrings(handler.toCharArray());

	EXPECT_NE(
		std::find(environment.begin(), environment.end(), "REQUEST_METHOD=GET"),
		environment.end());
	EXPECT_NE(std::find(environment.begin(),
						environment.end(),
						"SCRIPT_NAME=/hello.py"),
			  environment.end());
	EXPECT_NE(std::find(environment.begin(),
						environment.end(),
						"SCRIPT_FILENAME=/var/www/cgi-bin/hello.py"),
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
	handler.buildEnv(request, config);
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

TEST(CgiHandlerTest, HandlesMultipleConfiguredCgiExtensions)
{
	std::string directory = createTempCgiDirectory();
	ASSERT_FALSE(directory.empty());
	std::string python_script = directory + "/first.py";
	std::string shell_script = directory + "/second.sh";
	ASSERT_TRUE(writeCgiScript(python_script, "python extension response"));
	ASSERT_TRUE(writeCgiScript(shell_script, "shell extension response"));

	LocationConfig config;
	config.root_ = directory + "/";
	config.cgi_extensions_[".py"] = "/bin/sh";
	config.cgi_extensions_[".sh"] = "/bin/sh";
	std::map<std::string, std::string> headers;
	HttpRequest python_request;
	HttpRequest shell_request;
	setRequest(python_request, "GET", "/first.py", "", headers);
	setRequest(shell_request, "GET", "/second.sh", "", headers);

	CgiHandler python_handler;
	CgiHandler shell_handler;
	HttpResponse python_response =
		python_handler.handle(python_request, config);
	HttpResponse shell_response = shell_handler.handle(shell_request, config);

	EXPECT_EQ(python_response.status_code_, 200);
	EXPECT_EQ(shell_response.status_code_, 200);
	EXPECT_NE(python_response.toString().find("python extension response"),
			  std::string::npos);
	EXPECT_NE(shell_response.toString().find("shell extension response"),
			  std::string::npos);
	removeCgiFiles(directory, python_script, shell_script);
}

TEST(CgiHandlerTest, ExecutesExistingCgiAndReturnsHttpResponseToClient)
{
	std::string directory = createTempCgiDirectory();
	ASSERT_FALSE(directory.empty());
	std::string script_path = directory + "/response.cgi";
	ASSERT_TRUE(writeCgiScript(script_path, "hello from CGI"));

	LocationConfig config;
	config.root_ = directory + "/";
	config.cgi_extensions_[".cgi"] = "/bin/sh";
	std::map<std::string, std::string> headers;
	HttpRequest request;
	setRequest(request, "GET", "/response.cgi", "", headers);

	CgiHandler handler;
	HttpResponse response = handler.handle(request, config);
	std::string client_response = response.toString();

	EXPECT_EQ(response.status_code_, 200);
	EXPECT_NE(client_response.find("HTTP/1.1 200 OK\r\n"), std::string::npos);
	EXPECT_NE(client_response.find("Content-Type: text/plain\r\n"),
			  std::string::npos);
	EXPECT_NE(client_response.find("\r\n\r\nhello from CGI\n"),
			  std::string::npos);
	EXPECT_NE(client_response.find("Content-Length: 15\r\n"),
			  std::string::npos);
	removeCgiFiles(directory, script_path, "");
}

TEST(CgiHandlerTest, ReturnsErrorWhenCgiExtensionHasNoInterpreter)
{
	LocationConfig config;
	config.root_ = "/var/www/cgi-bin";
	std::map<std::string, std::string> headers;
	HttpRequest request;
	setRequest(request, "GET", "/missing.cgi", "", headers);

	CgiHandler handler;
	HttpResponse response = handler.handle(request, config);

	EXPECT_EQ(handler.pid_, -1);
	EXPECT_EQ(response.status_code_, 500);
	EXPECT_EQ(response.status_message_, "Internal Server Error");
}
