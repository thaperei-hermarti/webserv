/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:17:10 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:17:11 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_SERVER_CONNECTION_HPP
#define WEBSERV_SERVER_CONNECTION_HPP

#include <ctime>
#include <string>

#include "parser/request/HttpRequestParser.hpp"
#include "server/ConnState.hpp"
#include "server/IEventHandler.hpp"

class CgiHandler;
class HttpRequest;
class HttpResponse;
struct ServerConfig;

class Connection : public IEventHandler
{
  public:
	Connection(int fd, ServerConfig& config);
	~Connection();
	Connection(const Connection& other);
	Connection& operator=(const Connection& other);

	int getFd() const;
	void handleReadEvent();
	void handleWriteEvent();
	void handleTimeout();
	bool wantsWrite() const;

  private:
	void processRequest();
	void sendResponse(HttpResponse& response);

	int fd_;
	ConnState state_;
	std::string in_buffer_;
	std::string out_buffer_;
	HttpRequestParser request_parser_;
	HttpRequest* current_request_;
	HttpResponse* current_response_;
	ServerConfig& server_config_;
	CgiHandler* cgi_handler_;
	time_t last_activity_;
	bool keep_alive_;
};

#endif
