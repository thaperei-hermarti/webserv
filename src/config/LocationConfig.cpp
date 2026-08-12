/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.cpp                                :+:      :+:    :+:   */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*   Created: 2026/08/12 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/12 17:38:43 by hermarti         ###   ########.fr       */
/*   Created: 2026/07/31 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/07/31 17:38:43 by hermarti         ###   ########.fr       */
/*                                                                            */

#include "config/LocationConfig.hpp"

LocationConfig::LocationConfig() : autoindex_(false), upload_enabled_(false)
{
}

LocationConfig::LocationConfig(const LocationConfig& other)
	: path_(other.path_), allowed_methods_(other.allowed_methods_),
	  redirect_(other.redirect_), root_(other.root_),
	  autoindex_(other.autoindex_), index_file_(other.index_file_),
	  upload_enabled_(other.upload_enabled_),
	  upload_store_(other.upload_store_), cgi_extensions_(other.cgi_extensions_)
{
}

LocationConfig& LocationConfig::operator=(const LocationConfig& other)
{
	if (this != &other)
	{
		path_ = other.path_;
		allowed_methods_ = other.allowed_methods_;
		redirect_ = other.redirect_;
		root_ = other.root_;
		autoindex_ = other.autoindex_;
		index_file_ = other.index_file_;
		upload_enabled_ = other.upload_enabled_;
		upload_store_ = other.upload_store_;
		cgi_extensions_ = other.cgi_extensions_;
	}
	return *this;
}
