#ifdef WIN32
#pragma once
#endif // WIN32
#ifndef _LINUX_TIMEWHEEL_H
#define _LINUX_TIMEWHEEL_H

#include <memory>  

#include "spin_threadsafe_list.h"
#include "ThreadPool.h"


typedef void(*func)(unsigned long);

enum eTimerType
{
	eTimer_circle = 0x01,
	eTimer2 = 0x02,
	eTimer3,
	eTimer4
};

struct timer_event
{
	int					eType; //定时器类型
	unsigned long       interval;  // 定时器的超时时间，从当时开始多久触发定时器事件
	void				*pFun;	// 定时器事件函数
};

class CircQueue;

class CTimeWheel
{
public:
	CTimeWheel(int iPoolThreadNum = 10);
	~CTimeWheel();

	// 根据定时器的超时时间(interval)和时间片的时长(timeslot)相除，得到该定时
	// 器超时所需的时间片总数(ticks)。根据该定时器所需的时间片总数(ticks)和当
	// 前的时间片号，得出该定时器在时间轮中应处的时间片号，如果该时间片尚为空
	// 节点，就把该定时器关联到这个时间片上；如果该时间片不为空，把该定时器插
	// 入该时间片上的定时器链表中，同时计算时间轮圈数。时间复杂度：最好情况为
	// O(1)；最差情况时所有的定时器都链接到同一个时间片上，时间复杂度为 O(n)。
	int addTimer(timer_event &timer);

	///
	///	删除定时器
	///
	/// @return  如果删除成功返回true,否则为false
	///
	bool  delete_timer(timer_event &times);

	// 根据计时器中记录的 slot 号直接定位到挂接在其上的小链表，从该小链表上查
	// 找目标定时器节点，时间复杂度平均为 O(1)，最差为 O(n)。
	bool scanTimer(timer_event &times);

	// 当定时器管理器时针将要指向下一个 slot时，对这个 slot 上所有链表节点的
	// 圈数进行减一操作，如果结果等于 0，则表示该节点上的计时器超时。时间复杂
	// 度平均为 O(1)，最差为 O(n)。
	void timerTick();

	// 退出定时器
	void timerStop();

private:
	CircQueue *ring_queue_;
	
};

#endif