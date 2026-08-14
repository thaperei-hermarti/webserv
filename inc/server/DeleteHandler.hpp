/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DeleteHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:17:27 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:17:27 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_SERVER_DELETEHANDLER_HPP
#define WEBSERV_SERVER_DELETEHANDLER_HPP

#include "server/IRequestHandler.hpp"

class DeleteHandler : public IRequestHandler
{
  public:
	DeleteHandler();
	~DeleteHandler();
	DeleteHandler(const DeleteHandler& other);
	DeleteHandler& operator=(const DeleteHandler& other);

	HttpResponse handle(HttpRequest& request, LocationConfig& config);
};

#endif
