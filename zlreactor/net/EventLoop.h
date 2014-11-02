// ***********************************************************************
// Filename         : EventLoop.h
// Author           : LIZHENG
// Created          : 2014-10-26
// Description      : io service, �ɹ���socket��timer��file��ioģʽ
//
// Last Modified By : LIZHENG
// Last Modified On : 2014-10-26
//
// Copyright (c) lizhenghn@gmail.com. All rights reserved.
// ***********************************************************************
#ifndef ZL_EVENTLOOP_H
#define ZL_EVENTLOOP_H
#include "Define.h"
#include "base/Timestamp.h"
#include "base/NonCopy.h"
#include "thread/Mutex.h"
#include "thread/Thread.h"
NAMESPACE_ZL_NET_START
class Channel;
class Poller;

class EventLoop : zl::NonCopy
{
public:
    typedef std::function<void()> Functor;
public:
    EventLoop();
    ~EventLoop();

    void loop();
    void stop();

public:
    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);
    bool hasChannel(Channel *channel);

    //�����߳������У�����������̵߳��ã���תΪ����queueInLoop
    void runInLoop(const Functor& func);
    
    //�����첽���ô洢�����ȴ�poller����ʱ��ע������첽�����еĲ���
    void queueInLoop(const Functor& func);

    bool isInLoopThread() const { return currentThreadId_ == thread::this_thread::get_id(); }
    void assertInLoopThread() const;

private:
    void wakeupPoller();          //wakeup the waiting poller
    void callPendingFunctors();   //call when loop() return

private:
    typedef std::vector<Channel*> ChannelList;

    const thread::Thread::id currentThreadId_;  // thread id of this object created

    ChannelList              activeChannels_;
    Channel                  *currentActiveChannel_;
    Poller                   *poller_;          // I/O poller
    bool                     looping_;          // atomic 
    bool                     running_;          // atomic and shared between threads
    bool                     eventHandling_;    // atomic 

    bool                     callingPendingFunctors_;  // flag for callPendingFunctors func
    thread::Mutex            mutex_;            // for guard  pendingFunctors_
    std::vector<Functor>     pendingFunctors_;  // ��poll�ȴ�ʱ�������¼�����Ҫ����
};

NAMESPACE_ZL_NET_END
#endif  /* ZL_EVENTLOOP_H */