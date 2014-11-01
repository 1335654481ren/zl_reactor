#include "net/Channel.h"
#include <sstream>
#include <assert.h>
#include "net/Connection.h"
#include "base/ZLog.h"
#include "net/EventLoop.h"
using namespace zl::base;
NAMESPACE_ZL_NET_START

//const int Channel::kNoneEvent = SOCKETEVENT_NONE;
//const int Channel::kReadEvent = SOCKETEVENT_READ;
//const int Channel::kWriteEvent = SOCKETEVENT_WRITE;
const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, ZL_SOCKET fd)
    : loop_(loop), fd_(fd), events_(0), revents_(0)
{
}

Channel::~Channel()
{
    //if (loop_->isInLoopThread())
    {
        assert(!loop_->hasChannel(this));
    }
}

void Channel::update()
{
    loop_->updateChannel(this);
}

void Channel::remove()
{
    assert(isNoneEvent());
    loop_->removeChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime)
{
    handleEventWithHold(receiveTime);
}

void Channel::handleEventWithHold(Timestamp receiveTime)
{
    if ((revents_ & SOCKETEVENT_HUP) && !(revents_ & SOCKETEVENT_READ))
    {
		LOG_INFO("Channel::handleEventWithHold Channel::handleEventWithHold");
        if (closeCallback_) 
			closeCallback_();
    }

    //if (revents_ & POLLNVAL)
    //{
    //	LOG_WARN << "Channel::handle_event() POLLNVAL";
    //}

    if (revents_ & (SOCKETEVENT_ERROR/* | POLLNVAL*/))
    {
        if (errorCallback_) errorCallback_();
    }
    if (revents_ & (SOCKETEVENT_READ/* | POLLPRI */| SOCKETEVENT_HUP))
    {
        if (readCallback_) readCallback_(receiveTime);
    }
    if (revents_ & SOCKETEVENT_WRITE)
    {
        if (writeCallback_) writeCallback_();
    }
}

std::string Channel::reventsToString() const
{
    std::ostringstream oss;
    oss << fd_ << ": ";
    if (revents_ & SOCKETEVENT_READ)
        oss << "IN ";
    //if (revents_ & POLLPRI)
    //	oss << "PRI ";
    if (revents_ & SOCKETEVENT_WRITE)
        oss << "OUT ";
    if (revents_ & SOCKETEVENT_HUP)
        oss << "HUP ";
    //if (revents_ & POLLRDHUP)
    //	oss << "RDHUP ";
    if (revents_ & SOCKETEVENT_ERROR)
        oss << "ERR ";
    //if (revents_ & POLLNVAL)
    //	oss << "NVAL ";

    return oss.str().c_str();
}

NAMESPACE_ZL_NET_END