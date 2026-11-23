/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   router_test.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:25:38 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:25:39 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server/Router.hpp"

#include "config/ServerConfig.hpp"
#include "http/request/HttpRequest.hpp"

#include <gtest/gtest.h>

// Scenario: the request URI does not match any configured location.
//   Given : a server with one location "/api"
//   When  : "/missing" is routed
//   Then  : the router returns 404 instead of dispatching to an arbitrary location
TEST(RouterTest, NoMatchingLocationReturnsNotFound)
{
	Router router;
	ServerConfig config;
	LocationConfig location;
	location.path_ = "/api";
	config.locations_.push_back(location);
	HttpRequest request;
	request.setUri("/missing");

	HttpResponse response = router.route(request, config);

	EXPECT_EQ(404, response.getStatusCode());
}
