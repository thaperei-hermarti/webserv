/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.hpp                                    :+:      :+:    :+:   */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*   Created: 2026/08/12 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/12 17:38:43 by hermarti         ###   ########.fr       */
/*   Created: 2026/07/31 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/07/31 17:38:43 by hermarti         ###   ########.fr       */
/*                                                                            */

#ifndef WEBSERV_CGI_CGIHANDLER_HPP
#define WEBSERV_CGI_CGIHANDLER_HPP

#include <string>
#include <sys/types.h>
#include <vector>

#include "server/IEventHandler.hpp"
#include "server/IRequestHandler.hpp"

class CgiRegistry;
class HttpRequest;
struct LocationConfig;

class CgiHandler : public IEventHandler, public IRequestHandler
{
  public:
	CgiHandler();
	~CgiHandler();
	CgiHandler(const CgiHandler& other);
	CgiHandler& operator=(const CgiHandler& other);

	int getFd() const;
	void handleReadEvent();
	void handleWriteEvent();
	void handleTimeout();
	bool wantsWrite() const;

	HttpResponse handle(HttpRequest& request, LocationConfig& config);

  private:
	void execute(HttpRequest& request, LocationConfig& config);
	void buildEnv(HttpRequest& request, LocationConfig& config);
	void unchunkAndFeedStdin();

	pid_t pid_;
	int stdin_pipe_[2];
	int stdout_pipe_[2];
	std::vector<std::string> env_;
	std::string input_buffer_;
	std::string output_buffer_;
};

#endif
