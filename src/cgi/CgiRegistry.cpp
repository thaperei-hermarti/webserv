/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiRegistry.cpp                                   :+:      :+:    :+:   */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*   Created: 2026/08/12 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/12 17:38:43 by hermarti         ###   ########.fr       */
/*   Created: 2026/07/31 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/07/31 17:38:43 by hermarti         ###   ########.fr       */
/*                                                                            */

#include "cgi/CgiRegistry.hpp"

CgiRegistry::CgiRegistry()
{
}

CgiRegistry::~CgiRegistry()
{
}

CgiRegistry::CgiRegistry(const CgiRegistry& other)
	: interpreters_(other.interpreters_)
{
}

CgiRegistry& CgiRegistry::operator=(const CgiRegistry& other)
{
	if (this != &other)
	{
		interpreters_ = other.interpreters_;
	}
	return *this;
}

void CgiRegistry::registerCgi(const std::string& ext,
							  const std::string& interpreter_path)
{
	interpreters_[ext] = interpreter_path;
}

const std::string& CgiRegistry::getInterpreter(const std::string& ext) const
{
	static const std::string EMPTY;
	std::map<std::string, std::string>::const_iterator it =
		interpreters_.find(ext);
	return it != interpreters_.end() ? it->second : EMPTY;
}

bool CgiRegistry::hasHandlerFor(const std::string& ext) const
{
	return interpreters_.find(ext) != interpreters_.end();
}
