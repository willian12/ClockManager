// ClockManager.cpp : 定义控制台应用程序的入口点。
//
#include <boost/asio.hpp>  
#include <boost/progress.hpp>

#include "managercode/TimeWheel.h"

// 测试用的定时事件函数
void timer_func(unsigned long para)
{
	static int iTimes = 0;
	std::cout << "timer_func execute times: " << ++iTimes << std::endl;
}

// 实际运行定时管理区的线程的入口函数
void thread_func(CTimeWheel &timewheel, timer_event &timerevent)
{
	timewheel.addTimer(timerevent);
	timewheel.timerTick();
}

int main(int argc, char* argv[])
{
	std::unique_ptr<CTimeWheel> pTimewheel(new CTimeWheel(8));

	timer_event timerevent;
	timerevent.eType = eTimer_circle | eTimer2;
	timerevent.interval = 500;
    timerevent.pFun = (void *)timer_func;

	// 开启另外一个线程去运行定时器
	std::thread t(&thread_func, std::ref(*pTimewheel), std::ref(timerevent));
	std::cout << "Timer Manager Being!" << std::endl; // 实际这句话很可能在定时器已经开启一段时间后才执行

	// 下面这句注释的代码可以做测试用，打印时间
	//boost::progress_timer t;
	{
		// 主线程休眠100000ms
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
