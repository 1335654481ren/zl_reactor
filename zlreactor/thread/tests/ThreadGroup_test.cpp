/*************************************************************************
	File Name   : ThreadLocal_test.cpp
	Author      : LIZHENG
	Mail        : lizhenghn@gmail.com
	Created Time: 2015��06��23�� ���ڶ� 23ʱ26��50��
 ************************************************************************/
#include<iostream>
#include "thread/Thread.h"
#include "thread/Mutex.h"
#include "thread/Condition.h"
#include "thread/ThreadPool.h"
#include "thread/ThreadGroup.h"
using namespace std;
using namespace zl;
using namespace zl::thread;

void printI(int i)
{
    printf("this thread[%ld] print %d\n", this_thread::get_id().tid(), i);
    this_thread::sleep_for(chrono::milliseconds(1000));
}
void printString(const char* data)
{
    printf("this thread[%ld] print %s\n", this_thread::get_id().tid(), data);
}

void test_ThreadGroup()
{
    ThreadGroup tg;
    Thread *trd;
    for (int i = 1; i < 10; i++)
    {
        if (i%2 == 0)
        {
            trd = tg.create_thread(std::bind(printI, i));
        }
        else if(i%3 == 0)
        {
            tg.create_thread(std::bind(printString, "hello world"));
        }
        else
        {
            tg.add_thread(new Thread(std::bind(printI, i)));
        }
    }

    tg.remove_thread(trd); // �ýӿڲ�����ʹ�Ѽ�����߳�ֹͣ���У�����ʱ�����У�������ʹ���̲߳�����ThreadGroup����ɾ��

    tg.create_thread(std::bind(printString, "ThreadGroup"));

    tg.join_all();

    delete trd;
}

int main()
{
    cout << "###### test ThreadGroup ######\n";
    test_ThreadGroup();
    
    cout << "###### GAME OVER ######\n";
    return 0;
}
