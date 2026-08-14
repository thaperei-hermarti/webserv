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

ServerConfig::ServerConfig() : client_max_body_size_(1024 * 1024)
{
}

ServerConfig::ServerConfig(const ServerConfig& other)
	: host_(other.host_), ports_(other.ports_),
	  server_names_(other.server_names_), error_pages_(other.error_pages_),
	  client_max_body_size_(other.client_max_body_size_),
	  locations_(other.locations_)
{
}

ServerConfig& ServerConfig::operator=(const ServerConfig& other)
{
	if (this != &other)
	{
		host_ = other.host_;
		ports_ = other.ports_;
		server_names_ = other.server_names_;
		error_pages_ = other.error_pages_;
		client_max_body_size_ = other.client_max_body_size_;
		locations_ = other.locations_;
	}
	return *this;
}

LocationConfig& ServerConfig::matchLocation(const std::string& uri)
{
	(void) uri;
	return locations_.front();
}

std::size_t ServerConfig::getClientMaxBodySize() const
{
	return client_max_body_size_;
}
