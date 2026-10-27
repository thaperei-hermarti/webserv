/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:15:43 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:15:43 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_CONFIG_LOCATIONCONFIG_HPP
#define WEBSERV_CONFIG_LOCATIONCONFIG_HPP

#include <map>
#include <string>
#include <vector>
#include <sstream>

struct LocationConfig
{
	LocationConfig();
	LocationConfig(const LocationConfig& other);
	LocationConfig& operator=(const LocationConfig& other);

	std::string path_;
	std::vector<std::string> allowed_methods_;
	std::string redirect_;
	std::string root_;
	bool autoindex_;
	std::vector<std::string> index_file_;
	std::size_t client_max_body_size_;
	std::map<int, std::string> error_pages_;
	bool upload_enabled_;
	std::string upload_store_;
	std::map<std::string, std::string> cgi_extensions_;
};

std::ostream& operator<<(std::ostream& out, const LocationConfig& location);

#endif
