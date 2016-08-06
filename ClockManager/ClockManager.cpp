// ClockManager.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"

#include <boost/asio.hpp>  
#include <boost/progress.hpp>

#include "managercode/TimeWheel.h"

// �����õĶ�ʱ�¼�����
void timer_func(unsigned long para)
{
	static int iTimes = 0;
	std::cout << "timer_func execute times: " << ++iTimes << std::endl;
}

// ʵ�����ж�ʱ���������̵߳���ں���
void thread_func(CTimeWheel &timewheel, timer_event &timerevent)
{
	timewheel.addTimer(timerevent);
	timewheel.timerTick();
}

int main(int argc, char* argv[])
{
	std::unique_ptr<CTimeWheel> pTimewheel(new CTimeWheel(8));

	timer_event timerevent;
	timerevent.eType = eTimer_circle/* | eTimer2*/;
	timerevent.interval = 500;
	timerevent.pFun = timer_func;

	// ��������һ���߳�ȥ���ж�ʱ��
	std::thread t(&thread_func, std::ref(*pTimewheel), std::ref(timerevent));
	std::cout << "ʱ���ּ�ʱ���ѿ���" << std::endl; // ʵ����仰�ܿ����ڶ�ʱ���Ѿ�����һ��ʱ����ִ��

	// �������ע�͵Ĵ�������������ã���ӡʱ��
	//boost::progress_timer t;
	{
		// ���߳�����100000ms
		boost::asio::io_service io;
		boost::asio::deadline_timer timer(io, boost::posix_time::millisec(100000));
		timer.wait();
	}

	pTimewheel->delete_timer(timerevent);
	pTimewheel->timerStop();

	t.join();
	system("pause");
	return 0;
}
