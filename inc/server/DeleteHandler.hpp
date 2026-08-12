/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DeleteHandler.hpp                                 :+:      :+:    :+:   */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*   Created: 2026/08/12 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/12 17:38:43 by hermarti         ###   ########.fr       */
/*   Created: 2026/07/31 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/07/31 17:38:43 by hermarti         ###   ########.fr       */
/*                                                                            */

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
