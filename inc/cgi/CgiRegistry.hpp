/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiRegistry.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:15:31 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:15:33 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_CGI_CGIREGISTRY_HPP
#define WEBSERV_CGI_CGIREGISTRY_HPP

#include <map>
#include <string>

class CgiRegistry
{
  public:
	CgiRegistry();
	~CgiRegistry();
	CgiRegistry(const CgiRegistry& other);
	CgiRegistry& operator=(const CgiRegistry& other);

	void registerCgi(const std::string& ext,
					 const std::string& interpreter_path);
	const std::string& getInterpreter(const std::string& ext) const;
	bool hasHandlerFor(const std::string& ext) const;

  private:
	std::map<std::string, std::string> interpreters_;
};

#endif
