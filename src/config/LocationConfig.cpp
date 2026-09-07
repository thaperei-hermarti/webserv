/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:20:44 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:20:44 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "config/LocationConfig.hpp"

LocationConfig::LocationConfig()
	: autoindex_(false), index_file_(1, "index.html"),
	  client_max_body_size_(1024 * 1024), upload_enabled_(false)
{
	allowed_methods_.push_back("GET");
	allowed_methods_.push_back("POST");
	allowed_methods_.push_back("DELETE");
}

LocationConfig::LocationConfig(const LocationConfig& other)
	: path_(other.path_), allowed_methods_(other.allowed_methods_),
	  redirect_(other.redirect_), root_(other.root_),
	  autoindex_(other.autoindex_), index_file_(other.index_file_),
	  client_max_body_size_(other.client_max_body_size_),
	  error_pages_(other.error_pages_), upload_enabled_(other.upload_enabled_),
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
		client_max_body_size_ = other.client_max_body_size_;
		error_pages_ = other.error_pages_;
		upload_enabled_ = other.upload_enabled_;
		upload_store_ = other.upload_store_;
		cgi_extensions_ = other.cgi_extensions_;
	}
	return *this;
}

std::ostream& operator<<(std::ostream& out, const LocationConfig& location)
{
	out << "##### Location #####" << "\n";
	out << "path: " << location.path_ << "\n";
	out << "methods:";
	for (std::size_t i = 0; i < location.allowed_methods_.size(); ++i)
	{
		out << " " << location.allowed_methods_[i] << ",";
	}
	out << "\nredirect: " << location.redirect_ << "\n";
	out << "root: " << location.root_ << "\n";
	out << "autoindex: " << location.autoindex_ << "\n";
	out << "index: ";
	for (std::vector<std::string>::const_iterator it =
			 location.index_file_.begin();
		 it != location.index_file_.end();
		 it++)
	{
		out << *it << " ";
	}
	out << "\nclient max body size: " << location.client_max_body_size_ << "\n";
	out << "upload enabled: " << location.upload_enabled_ << "\n";
	if (location.upload_enabled_)
		out << "upload dir: " << location.upload_store_ << "\n";
	for (std::map<std::string, std::string>::const_iterator it =
			 location.cgi_extensions_.begin();
		 it != location.cgi_extensions_.end();
		 it++)
	{
		if (it == location.cgi_extensions_.begin())
			out << "cgi_extension: ";
		out << it->first << " => " << it->second << ",";
	}
	return (out);
}
