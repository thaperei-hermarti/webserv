/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   i_event_handler_test.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:25:25 by hermarti          #+#    #+#             */
/*   Updated: 2026/10/05 00:00:00 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server/Acceptor.hpp"
#include "server/IEventHandler.hpp"

#include <gtest/gtest.h>

#include <type_traits>

namespace
{
class TestHandler : public IEventHandler
{
  public:
	TestHandler(int fd)
		: fd_(fd), read_calls_(0), write_calls_(0), timeout_calls_(0),
		  wants_write_(false), destroyed_(NULL)
	{
	}

	~TestHandler()
	{
		if (destroyed_ != NULL)
			*destroyed_ = true;
	}

	int getFd() const
	{
		return fd_;
	}

	void handleReadEvent()
	{
		++read_calls_;
	}

	void handleWriteEvent()
	{
		++write_calls_;
	}

	bool wantsWrite() const
	{
		return wants_write_;
	}

	void handleTimeout()
	{
		++timeout_calls_;
	}

	void setWantsWrite(bool value)
	{
		wants_write_ = value;
	}

	void setDestroyedFlag(bool* flag)
	{
		destroyed_ = flag;
	}

	int readCalls() const
	{
		return read_calls_;
	}

	int writeCalls() const
	{
		return write_calls_;
	}

	int timeoutCalls() const
	{
		return timeout_calls_;
	}

  private:
	int fd_;
	int read_calls_;
	int write_calls_;
	int timeout_calls_;
	bool wants_write_;
	bool* destroyed_;
};

class MinimalHandler : public IEventHandler
{
  public:
	explicit MinimalHandler(int fd) : fd_(fd)
	{
	}

	int getFd() const
	{
		return fd_;
	}

	void handleReadEvent()
	{
	}

	void handleWriteEvent()
	{
	}

  private:
	int fd_;
};
} // namespace

static_assert(std::is_abstract<IEventHandler>::value,
			  "IEventHandler is expected to be a pure interface");
static_assert(std::has_virtual_destructor<IEventHandler>::value,
			  "IEventHandler requires a virtual destructor");
static_assert(!std::is_abstract<Acceptor>::value,
			  "Acceptor must be concrete once wantsWrite() is optional");

TEST(IEventHandlerTest, IsAnAbstractInterface)
{
	EXPECT_TRUE(std::is_abstract<IEventHandler>::value);
	EXPECT_TRUE(std::has_virtual_destructor<IEventHandler>::value);
}

TEST(IEventHandlerTest, ExposesItsFileDescriptor)
{
	TestHandler handler(42);
	IEventHandler& event_handler = handler;

	EXPECT_EQ(42, event_handler.getFd());
}

TEST(IEventHandlerTest, DispatchesEventsThroughTheBaseInterface)
{
	TestHandler handler(7);
	IEventHandler& event_handler = handler;

	event_handler.handleReadEvent();
	event_handler.handleReadEvent();
	event_handler.handleWriteEvent();
	event_handler.handleTimeout();

	EXPECT_EQ(2, handler.readCalls());
	EXPECT_EQ(1, handler.writeCalls());
	EXPECT_EQ(1, handler.timeoutCalls());
}

TEST(IEventHandlerTest, ReportsWhetherItWantsWriteEvents)
{
	TestHandler handler(0);
	IEventHandler& event_handler = handler;

	EXPECT_FALSE(event_handler.wantsWrite());
	handler.setWantsWrite(true);
	EXPECT_TRUE(event_handler.wantsWrite());
}

TEST(IEventHandlerTest, DestroyingThroughBaseRunsTheDerivedDestructor)
{
	bool destroyed = false;
	TestHandler* handler = new TestHandler(3);
	handler->setDestroyedFlag(&destroyed);

	IEventHandler* event_handler = handler;
	delete event_handler;

	EXPECT_TRUE(destroyed);
}

TEST(IEventHandlerTest, OptionalHooksDefaultToNoOpAndFalse)
{
	MinimalHandler handler(5);
	IEventHandler& event_handler = handler;

	EXPECT_FALSE(event_handler.wantsWrite());
	event_handler.handleTimeout();
}

TEST(IEventHandlerTest, AcceptorImplementsTheContract)
{
	EXPECT_FALSE(std::is_abstract<Acceptor>::value);
}
