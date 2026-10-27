/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:15:50 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:15:51 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_CONFIG_SERVERCONFIG_HPP
#define WEBSERV_CONFIG_SERVERCONFIG_HPP

#include <cstddef>
#include <map>
#include <string>
#include <vector>
#include <sstream>

#include "config/LocationConfig.hpp"

struct ServerConfig
{
	ServerConfig();
	ServerConfig(const ServerConfig& other);
	ServerConfig& operator=(const ServerConfig& other);

	LocationConfig& matchLocation(const std::string& uri);
	std::size_t getClientMaxBodySize() const;

	std::string host_;
	std::vector<int> ports_;
	std::vector<std::string> server_names_;
	std::string root_;
	std::vector<std::string> index_file_;
	std::map<int, std::string> error_pages_;
	std::size_t client_max_body_size_;
	std::vector<LocationConfig> locations_;
};

std::ostream& operator<<(std::ostream& out, const ServerConfig& server);

#endif
