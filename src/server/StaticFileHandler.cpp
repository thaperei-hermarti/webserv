/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StaticFileHandler.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:23:22 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:23:22 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server/StaticFileHandler.hpp"

#include "config/FileConfig.hpp"
#include "config/LocationConfig.hpp"
#include "http/request/HttpRequest.hpp"

static std::string pathWithoutQuery(const std::string& uri)
{
	const std::size_t query = uri.find('?');
	if (query == std::string::npos)
	{
		return uri;
	}
	return uri.substr(0, query);
}

static bool hasParentSegment(const std::string& path)
{
	std::size_t start = 0;
	while (start <= path.size())
	{
		const std::size_t slash = path.find('/', start);
		const std::size_t end =
			slash == std::string::npos ? path.size() : slash;
		if (end - start == 2 && path.compare(start, 2, "..") == 0)
		{
			return true;
		}
		if (slash == std::string::npos)
		{
			break;
		}
		start = slash + 1;
	}
	return false;
}

static std::string joinPath(const std::string& root, const std::string& suffix)
{
	if (root.empty())
	{
		return "";
	}
	if (suffix.empty())
	{
		return root;
	}
	std::string result = root;
	while (result.size() > 1 && result[result.size() - 1] == '/')
	{
		result.erase(result.size() - 1);
	}
	if (result == "/")
	{
		if (suffix[0] == '/')
		{
			result += suffix.substr(1);
		}
		else
		{
			result += suffix;
		}
		return result;
	}
	if (suffix[0] != '/')
	{
		result += '/';
	}
	result += suffix;
	return result;
}

std::string StaticFileHandler::resolvePath(const std::string& uri,
										   const LocationConfig& config)
{
	const std::string path = pathWithoutQuery(uri);
	if (path.empty() || config.path_.empty() || config.root_.empty() ||
		path.compare(0, config.path_.size(), config.path_) != 0)
	{
		return "";
	}
	const std::string suffix = path.substr(config.path_.size());
	if (!suffix.empty() && suffix[0] != '/' &&
		config.path_[config.path_.size() - 1] != '/')
	{
		return "";
	}
	if (hasParentSegment(path))
	{
		return "";
	}
	return joinPath(config.root_, suffix);
}

StaticFileHandler::StaticFileHandler()
{
}

StaticFileHandler::~StaticFileHandler()
{
}

StaticFileHandler::StaticFileHandler(const StaticFileHandler& other)
{
	(void) other;
}

StaticFileHandler& StaticFileHandler::operator=(const StaticFileHandler& other)
{
	(void) other;
	return *this;
}

HttpResponse StaticFileHandler::handle(HttpRequest& request,
									   LocationConfig& config)
{
	const std::string path = resolvePath(request.getUri(), config);
	if (path.empty())
	{
		HttpResponse response;
		response.setStatus(400, "Bad Request");
		return response;
	}
	const int type = FileConfig::getTypePath(path);
	if (type == 1)
	{
		return serveFile(path);
	}
	if (type == 2)
	{
		return listDirectory(path);
	}
	HttpResponse response;
	response.setStatus(404, "Not Found");
	return response;
}

HttpResponse StaticFileHandler::serveFile(const std::string& path)
{
	(void) path;
	return HttpResponse();
}

HttpResponse StaticFileHandler::listDirectory(const std::string& path)
{
	(void) path;
	return HttpResponse();
}
