#ifdef WIN32
#pragma once
#endif // WIN32
#ifndef CIRC_QUEUE_H_
#define CIRC_QUEUE_H_

#include <iostream>
#include <string>
#include <list>
#include <strstream>
#include <thread>
#include <atomic>

#include <boost/asio.hpp>  
#include <boost/progress.hpp>

#include "spin_threadsafe_list.h"
#include "TimeWheel.h"



using namespace std;

// 表盘的大小
const int defaultSize = 8;
// 默认情况下时钟时间片的大小，这里其实是设置时钟频度为100ms，但是
// 考虑到指令本身也需要执行时间，所以设为96
const int defaultTimeSlot = 96;

// 定时器事件结点
struct timer_node
{
	int etype; // 定时器类型
	unsigned long interval;  // 定时器的超时时间
	func timer_func; // 事件函数
	int slotNum; // 时间片号
	int round; // 时间轮圈数
};


// 定时器管理表盘
class CircQueue
{
public:
	CircQueue(int iPoolThreadNum = 10, int sz = defaultSize) //构造函数
		: thread_pool_(iPoolThreadNum)
		, currentSlot_(0)
		, timeslot_(defaultTimeSlot)
		, running_(true)
	{
		// 构造循环队列
		if (sz >= 0)
		{
			maxTimers_ = sz;
			elements = new threadsafe_list<timer_node>*[maxTimers_];
		}

		// 生成循环队列的每个元素
		for (size_t i = 0; i < maxTimers_; i++)
		{
			elements[i] = new threadsafe_list<timer_node>;
		}

		thread_pool_.start();
	};        
	virtual ~CircQueue() //析构函数
	{
		for (size_t i = 0; i < maxTimers_; i++)
		{
			delete elements[i];
			elements[i] = NULL;
		}

		delete[] elements;
		elements = NULL;
	};                  
public:
	//virtual int getSize() const;            //计算队列中元素个数
	// 添加一个定时事件
	int addTimer(timer_event &timer);

	// 删除一个定时事件
	bool delete_timer(timer_event &times);

	bool scanTimer(timer_event &times);

	// 开始计时
	void timerTick();

	// 停止计时
	void timerStop();

private:
	// 存放循环队列元素的队列数组
	threadsafe_list<timer_node> **elements;    
    // 循环队列最大可容纳元素个数
	size_t maxTimers_;

	// 时间槽指针(也就是时钟指针)
	int currentSlot_;
	// 时间片，隔多久跳一次
	int timeslot_; 

	std::atomic<bool> running_;

	// 线程池，用来运行定时器事件
	ThreadPool thread_pool_;
};

// 根据定时器的超时时间(interval)和时间片的时长(timeslot)相除，得到该定时
// 器超时所需的时间片总数(ticks)。根据该定时器所需的时间片总数(ticks)和当
// 前的时间片号，得出该定时器在时间轮中应处的时间片号，如果该时间片尚为空
// 节点，就把该定时器关联到这个时间片上；如果该时间片不为空，把该定时器插
// 入该时间片上的定时器链表中，同时计算时间轮圈数。时间复杂度：最好情况为
// O(1)；最差情况时所有的定时器都链接到同一个时间片上，时间复杂度为 O(n)。
inline 
int CircQueue::addTimer(timer_event &timer)
{
	timer_node node;
	// 定时器类型
	node.etype = timer.eType;
	// 定时器间隔执行时间
	node.interval = timer.interval;
	int ticks = timer.interval / timeslot_;
	// 定时器所处的槽位
	node.slotNum = (currentSlot_ + ticks % maxTimers_) % maxTimers_;
	// 经过多少轮回执行
	node.round = ticks / maxTimers_;
	// 定时器函数
	node.timer_func = (func)timer.pFun;
	elements[node.slotNum]->push_front(node);
	return 0;
}

// 根据计时器中记录的 slot 号直接定位到挂接在其上的小链表，从该小链表上查
// 找目标定时器节点，时间复杂度平均为 O(1)，最差为 O(n)。
inline
bool CircQueue::scanTimer(timer_event &timer)
{
	timer_node node;
	node.etype = timer.eType;
	int ticks = timer.interval / timeslot_;
	node.slotNum = currentSlot_ + (ticks % maxTimers_);
	node.round = ticks / maxTimers_;
	node.timer_func = (func)timer.pFun;
	std::shared_ptr<timer_node> pNode = elements[node.slotNum]->find_first_if([=, &node](timer_node &time_node) {
		return time_node.etype == node.etype && time_node.interval == node.interval && time_node.round == node.round &&
			time_node.slotNum == node.slotNum && time_node.timer_func == node.timer_func;
	});
	return NULL != pNode.get();
}

inline 
void CircQueue::timerTick()
{
	std::list<timer_node> node_list_tmp;

	while (running_.load())
	{
		// 先判断链表上的结点是否要执行
		//boost::progress_timer t;
		// 等待100毫秒
		{
			boost::asio::io_service io;
			boost::asio::deadline_timer timer(io, boost::posix_time::millisec(timeslot_));
			timer.wait();
		}

		// 触发定时器事件
		elements[currentSlot_]->for_each([&](timer_node &node) {
			// 2. 如果结点的环数等于0，则触发定时器事件
			if (0 == node.round)
			{
				func event_func = node.timer_func;
				ThreadPool::Task task = std::bind(event_func, 0);
				thread_pool_.run(task);
			}		

			// 1. 将结点的环数减1
			--node.round;
		});

		// 删去已触发的定时器事件
		elements[currentSlot_]->remove_if([&, this](timer_node &node) {
			// 如果定时事件类型是循环定时器，则重新插入计时器
			// 这里需要预先对循环定时器进行计算
			if (eTimer_circle & node.etype && 0 > node.round)
			{
				// 计算间隔总滴答数
				int ticks = node.interval / timeslot_;
				timer_node tmp_node = node;
				// 计算定时事件的时间槽
				tmp_node.slotNum = (currentSlot_ + (ticks % maxTimers_)) % maxTimers_;
				// 计算时钟周期
				tmp_node.round = ticks / maxTimers_;

				node_list_tmp.push_back(tmp_node);
			}

			return 0 > node.round;
		});

		// 如果定时事件类型是循环定时器，则重新插入计时器
		for (auto it = node_list_tmp.begin(); it != node_list_tmp.end(); it++)
		{
			elements[it->slotNum]->push_front(*it);
		}

		//完成本次计时后清空链表
		node_list_tmp.clear();
		

		// 转到下一个环结点
		currentSlot_ = ++currentSlot_ % maxTimers_;
	}
}

inline void CircQueue::timerStop()
{
	running_.store(false);
}

// 删除指定计时器
inline
bool CircQueue::delete_timer(timer_event &timer)
{
	for (size_t i = 0; i < maxTimers_; i++)
	{
		elements[i]->remove_if([=, &timer](timer_node &node) 
		{ // operation
			return  node.etype == timer.eType && node.interval == timer.interval && node.timer_func == timer.pFun;
		});
	}
	
	return true;
}

#endif