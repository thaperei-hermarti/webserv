/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:20:51 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:20:52 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "config/ServerConfig.hpp"

static std::string pathWithoutQuery(const std::string& uri)
{
	const std::size_t query = uri.find('?');
	if (query == std::string::npos)
	{
		return uri;
	}
	return uri.substr(0, query);
}

static bool locationMatches(const std::string& path, const std::string& prefix)
{
	if (prefix.empty() || path.size() < prefix.size() ||
		path.compare(0, prefix.size(), prefix) != 0)
	{
		return false;
	}
	return path.size() == prefix.size() || prefix[prefix.size() - 1] == '/' ||
		   path[prefix.size()] == '/';
}

ServerConfig::ServerConfig()
	: host_("localhost"), ports_(1, 80), index_file_(1, "index.html"),
	  client_max_body_size_(1024 * 1024), idle_timeout_(30)
{
}

ServerConfig::ServerConfig(const ServerConfig& other)
	: host_(other.host_), ports_(other.ports_),
	  server_names_(other.server_names_), root_(other.root_),
	  index_file_(other.index_file_), error_pages_(other.error_pages_),
	  client_max_body_size_(other.client_max_body_size_),
	  locations_(other.locations_), idle_timeout_(other.idle_timeout_)
{
}

ServerConfig& ServerConfig::operator=(const ServerConfig& other)
{
	if (this != &other)
	{
		host_ = other.host_;
		ports_ = other.ports_;
		server_names_ = other.server_names_;
		root_ = other.root_;
		index_file_ = other.index_file_;
		error_pages_ = other.error_pages_;
		client_max_body_size_ = other.client_max_body_size_;
		locations_ = other.locations_;
		idle_timeout_ = other.idle_timeout_;
	}
	return *this;
}

LocationConfig* ServerConfig::matchLocation(const std::string& uri)
{
	const std::string path = pathWithoutQuery(uri);
	LocationConfig* best = NULL;
	for (std::size_t i = 0; i < locations_.size(); ++i)
	{
		LocationConfig& candidate = locations_[i];
		if (!locationMatches(path, candidate.path_))
		{
			continue;
		}
		if (best == NULL || candidate.path_.size() > best->path_.size())
		{
			best = &candidate;
		}
	}
	return best;
}

std::size_t ServerConfig::getClientMaxBodySize() const
{
	return client_max_body_size_;
}

std::ostream& operator<<(std::ostream& out, const ServerConfig& server)
{
	out << "##### Server #####" << "\n";
	out << "Host: " << server.host_ << "\n";
	out << "ports:";
	for (std::size_t i = 0; i < server.ports_.size(); ++i)
	{
		out << " " << server.ports_[i] << ",";
	}
	for (std::size_t i = 0; i < server.server_names_.size(); ++i)
	{
		if (i == 0)
		{
			out << "\nserver_names:";
		}
		out << " " << server.server_names_[i] << ",";
	}
	for (std::map<int, std::string>::const_iterator it =
			 server.error_pages_.begin();
		 it != server.error_pages_.end();
		 it++)
	{
		if (it == server.error_pages_.begin())
		{
			out << "\nerror_pages:";
		}
		out << " " << it->first << " => " << it->second << ",";
	}
	out << "\nRoot: " << server.root_ << "\n";
	out << "Index: ";
	for (std::vector<std::string>::const_iterator it =
			 server.index_file_.begin();
		 it != server.index_file_.end();
		 it++)
	{
		out << *it << " ";
	}
	out << "\nclient Max body size: " << server.client_max_body_size_ << "\n";
	for (std::vector<LocationConfig>::const_iterator it =
			 server.locations_.begin();
		 it != server.locations_.end();
		 it++)
	{
		out << " " << *it;
	}
	return out;
}
