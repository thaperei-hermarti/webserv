/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseBuilder.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:21:26 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:21:27 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "http/response/ResponseBuilder.hpp"

#include <cctype>
#include <ctime>
#include <fstream>
#include <sstream>

static std::string lowerExtension(const std::string& path)
{
	const std::size_t slash = path.find_last_of('/');
	const std::size_t dot = path.find_last_of('.');
	if (dot == std::string::npos || (slash != std::string::npos && dot < slash))
	{
		return "";
	}
	std::string extension = path.substr(dot);
	for (std::size_t i = 0; i < extension.size(); ++i)
	{
		extension[i] = static_cast<char>(
			std::tolower(static_cast<unsigned char>(extension[i])));
	}
	return extension;
}

static std::string contentTypeForPath(const std::string& path)
{
	const std::string extension = lowerExtension(path);

	if (extension == ".html" || extension == ".htm")
	{
		return "text/html";
	}
	if (extension == ".css")
	{
		return "text/css";
	}
	if (extension == ".js")
	{
		return "application/javascript";
	}
	if (extension == ".txt")
	{
		return "text/plain";
	}
	if (extension == ".png")
	{
		return "image/png";
	}
	if (extension == ".jpg" || extension == ".jpeg")
	{
		return "image/jpeg";
	}
	if (extension == ".gif")
	{
		return "image/gif";
	}
	if (extension == ".svg")
	{
		return "image/svg+xml";
	}
	if (extension == ".json")
	{
		return "application/json";
	}
	return "application/octet-stream";
}

static std::string httpDate()
{
	const std::time_t now = std::time(NULL);
	const std::tm* gmt = std::gmtime(&now);
	if (gmt == NULL)
	{
		return "";
	}
	char buffer[64];
	if (std::strftime(
			buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", gmt) == 0)
	{
		return "";
	}
	return buffer;
}

HttpResponse ResponseBuilder::buildErrorResponse(int code,
												 const ServerConfig& config)
{
	(void) code;
	(void) config;
	return HttpResponse();
}

HttpResponse ResponseBuilder::buildRedirect(const std::string& location,
											int code)
{
	(void) location;
	(void) code;
	return HttpResponse();
}

HttpResponse ResponseBuilder::buildFromFile(const std::string& path)
{
	HttpResponse response;
	std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);

	if (!file.is_open())
	{
		response.setStatus(404, "Not Found");
		response.setHeader("Date", httpDate());
		response.setHeader("Server", "webserv");
		return response;
	}

	std::ostringstream content;
	content << file.rdbuf();

	response.setBody(content.str());
	response.setHeader("Content-Type", contentTypeForPath(path));
	response.setHeader("Date", httpDate());
	response.setHeader("Server", "webserv");
	return response;
}
