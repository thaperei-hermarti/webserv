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

#include "config/ServerConfig.hpp"

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

static bool readFile(const std::string& path, std::string& content)
{
	std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
	if (!file.is_open())
	{
		return false;
	}
	std::ostringstream buffer;
	buffer << file.rdbuf();
	content = buffer.str();
	return true;
}

static void setDefaultHeaders(HttpResponse& response)
{
	response.setHeader("Date", httpDate());
	response.setHeader("Server", "webserv");
}

static std::string statusReason(int code)
{
	switch (code)
	{
		case 400:
			return "Bad Request";
		case 403:
			return "Forbidden";
		case 404:
			return "Not Found";
		case 405:
			return "Method Not Allowed";
		case 413:
			return "Payload Too Large";
		case 500:
			return "Internal Server Error";
		default:
			return "Error";
	}
}

static std::string statusDetail(int code)
{
	switch (code)
	{
		case 400:
			return "The server could not understand the request.";
		case 403:
			return "You do not have permission to access this resource.";
		case 404:
			return "The requested resource could not be found.";
		case 405:
			return "This HTTP method is not allowed for the requested "
				   "resource.";
		case 413:
			return "The request body is larger than the server allows.";
		case 500:
			return "The server encountered an unexpected condition.";
		default:
			return "An unexpected error occurred.";
	}
}

static std::string defaultErrorBody(int code, const std::string& reason)
{
	std::ostringstream static_path;
	static_path << "www/error_pages/" << code << ".html";
	std::string static_page;
	if (readFile(static_path.str(), static_page))
	{
		return static_page;
	}

	const std::string DETAIL = statusDetail(code);
	std::ostringstream body;
	body << "<!DOCTYPE html>\n"
		 << "<html lang=\"en\">\n"
		 << "<head>\n"
		 << "<meta charset=\"utf-8\">\n"
		 << "<meta name=\"viewport\" content=\"width=device-width, "
			"initial-scale=1\">\n"
		 << "<title>" << code << " " << reason << " | webserv</title>\n"
		 << "<style>\n"
		 << "*{box-sizing:border-box}\n"
		 << "body{margin:0;min-height:100vh;display:flex;align-items:center;"
			"justify-content:center;padding:24px;background:#0b0d10;"
			"color:#f4f4f5;font-family:ui-monospace,SFMono-Regular,Menlo,"
			"Consolas,monospace}\n"
		 << "main{width:min(100%,720px);padding:clamp(20px,4vw,40px);"
			"border:1px solid #2a2e35;border-radius:18px;background:#12151a;"
			"box-shadow:0 24px 80px rgba(0,0,0,.45);text-align:center}\n"
		 << ".eyebrow{margin:0 0 8px;color:#8b949e;font-size:12px;"
			"letter-spacing:.18em;text-transform:uppercase}\n"
		 << "h1{margin:0;font-size:clamp(28px,6vw,52px);"
			"letter-spacing:-.04em}\n"
		 << "p{margin:12px 0 0;color:#b7c0cc;line-height:1.5}\n"
		 << "img{display:block;width:100%;height:auto;max-height:52vh;"
			"object-fit:contain;margin:24px auto;border-radius:12px}\n"
		 << "a{display:inline-block;margin-top:8px;padding:10px 16px;"
			"border:1px solid #30363d;border-radius:999px;color:#f4f4f5;"
			"text-decoration:none}\n"
		 << "a:hover{border-color:#8b949e}\n"
		 << "footer{margin-top:18px;color:#6e7681;font-size:11px}\n"
		 << "</style>\n"
		 << "</head>\n"
		 << "<body>\n"
		 << "<main>\n"
		 << "<p class=\"eyebrow\">webserv</p>\n"
		 << "<h1>" << code << " " << reason << "</h1>\n"
		 << "<p>" << DETAIL << "</p>\n"
		 << "<img src=\"/error_pages/" << code << ".jpg\" alt=\"" << code << " "
		 << reason << "\">\n"
		 << "<a href=\"/\">Back to home</a>\n"
		 << "<footer>HTTP " << code << "</footer>\n"
		 << "</main>\n"
		 << "</body>\n"
		 << "</html>\n";
	return body.str();
}

HttpResponse ResponseBuilder::buildErrorResponse(int code,
												 const ServerConfig& config)
{
	const std::string REASON = statusReason(code);
	std::map<int, std::string>::const_iterator configured =
		config.error_pages_.find(code);

	if (configured != config.error_pages_.end())
	{
		HttpResponse custom = buildFromFile(configured->second);
		if (custom.getStatusCode() == 200)
		{
			custom.setStatus(code, REASON);
			return custom;
		}
	}

	HttpResponse response;
	response.setStatus(code, REASON);
	response.setHeader("Content-Type", "text/html");
	response.setBody(defaultErrorBody(code, REASON));
	setDefaultHeaders(response);
	return response;
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
	std::string content;

	if (!readFile(path, content))
	{
		response.setStatus(404, "Not Found");
		setDefaultHeaders(response);
		return response;
	}

	response.setBody(content);
	response.setHeader("Content-Type", contentTypeForPath(path));
	setDefaultHeaders(response);
	return response;
}
