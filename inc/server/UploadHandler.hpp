/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UploadHandler.hpp                                 :+:      :+:    :+:   */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*   Created: 2026/08/12 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/12 17:38:43 by hermarti         ###   ########.fr       */
/*   Created: 2026/07/31 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/07/31 17:38:43 by hermarti         ###   ########.fr       */
/*                                                                            */

#ifndef WEBSERV_SERVER_UPLOADHANDLER_HPP
#define WEBSERV_SERVER_UPLOADHANDLER_HPP

#include <string>

#include "server/IRequestHandler.hpp"

class UploadHandler : public IRequestHandler
{
  public:
	UploadHandler();
	~UploadHandler();
	UploadHandler(const UploadHandler& other);
	UploadHandler& operator=(const UploadHandler& other);

	HttpResponse handle(HttpRequest& request, LocationConfig& config);

  private:
	void storeFile(const std::string& store_path, const std::string& body);
};

#endif
