NAME = webserv
CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98
TEST_CFLAGS = -Wall -Wextra -Werror -std=c++17

SRC_DIR = src
INC_DIR = inc
OBJ_DIR = obj
TEST_DIR = test
TEST_NAME = unit_tests

INCLUDES = -I./$(INC_DIR)
GTEST_CFLAGS = $(shell sh scripts/gtest-flags.sh cflags)
GTEST_LIBS = $(shell sh scripts/gtest-flags.sh libs)

LIBS =

SRC_FILES = \
	main.cpp \
	http/request/HttpRequest.cpp \
	http/response/HttpResponse.cpp \
	http/response/ResponseBuilder.cpp \
	parser/config/ConfigParser.cpp \
	parser/request/HttpRequestParser.cpp \
	config/ServerConfig.cpp \
	config/LocationConfig.cpp \
	server/Server.cpp \
	server/Reactor.cpp \
	server/IEventHandler.cpp \
	server/Acceptor.cpp \
	server/Connection.cpp \
	server/Router.cpp \
	server/IRequestHandler.cpp \
	server/StaticFileHandler.cpp \
	server/UploadHandler.cpp \
	server/DeleteHandler.cpp \
	server/Cookie.cpp \
	server/Session.cpp \
	server/SessionManager.cpp \
	cgi/CgiHandler.cpp \
	cgi/CgiRegistry.cpp
