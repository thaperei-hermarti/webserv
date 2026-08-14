/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cookie.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:22:18 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:22:18 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server/Cookie.hpp"

Cookie::Cookie()
{
}

Cookie::~Cookie()
{
}

Cookie::Cookie(const Cookie& other)
	: name_(other.name_), value_(other.value_), attributes_(other.attributes_)
{
}

Cookie& Cookie::operator=(const Cookie& other)
{
	if (this != &other)
	{
		name_ = other.name_;
		value_ = other.value_;
		attributes_ = other.attributes_;
	}
	return *this;
}

Cookie Cookie::parse(const std::string& header_value)
{
	(void) header_value;
	return Cookie();
}

const std::string& Cookie::toSetCookieHeader() const
{
	static const std::string EMPTY;
	(void) name_;
	(void) value_;
	(void) attributes_;
	return EMPTY;
}
