// ClockManager.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <boost/asio.hpp>  
#include <boost/progress.hpp>

#include "managercode/TimeWheel.h"

void timer_func(unsigned long para)
{
	static int iTimes = 0;
	std::cout << "timer_func execute times: " << ++iTimes << std::endl;
}

void thread_func(CTimeWheel &timewheel, timer_event &timerevent)
{
	timewheel.addTimer(timerevent);
	timewheel.timerTick();
}

int main(int argc, char* argv[])
{
	CTimeWheel *pTimewheel = new CTimeWheel(8);
	timer_event timerevent;
	timerevent.eType = eTimer_circle | eTimer2;
	timerevent.interval = 500;
	timerevent.pFun = timer_func;
	std::thread t(&thread_func, std::ref(*pTimewheel), std::ref(timerevent));
	std::cout << "11111111" << std::endl;
	//boost::progress_timer t;
	{
		boost::asio::io_service io;
		boost::asio::deadline_timer timer(io, boost::posix_time::millisec(100000));
		timer.wait();
	}

	pTimewheel->delete_timer(timerevent);
	pTimewheel->timerStop();
	//std::this_thread::sleep_for(std::chrono::seconds{ 10 });
	t.join();
	system("pause");
	return 0;
}
