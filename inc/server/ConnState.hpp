/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnState.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:17:02 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:17:04 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_SERVER_CONNSTATE_HPP
#define WEBSERV_SERVER_CONNSTATE_HPP

enum ConnState
{
	READING_HEADERS,
	READING_BODY,
	DISPATCHING,
	RUNNING_CGI,
	WRITING_RESPONSE,
	CLOSED
};

#endif
