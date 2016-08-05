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
	int					eType; //��ʱ������
	unsigned long       interval;  // ��ʱ���ĳ�ʱʱ�䣬�ӵ�ʱ��ʼ��ô�����ʱ���¼�
	void				*pFun;	// ��ʱ���¼�����
};

class CircQueue;

class CTimeWheel
{
public:
	CTimeWheel(int iPoolThreadNum = 10);
	~CTimeWheel();

	// ���ݶ�ʱ���ĳ�ʱʱ��(interval)��ʱ��Ƭ��ʱ��(timeslot)������õ��ö�ʱ
	// ����ʱ�����ʱ��Ƭ����(ticks)�����ݸö�ʱ�������ʱ��Ƭ����(ticks)�͵�
	// ǰ��ʱ��Ƭ�ţ��ó��ö�ʱ����ʱ������Ӧ����ʱ��Ƭ�ţ������ʱ��Ƭ��Ϊ��
	// �ڵ㣬�ͰѸö�ʱ�����������ʱ��Ƭ�ϣ������ʱ��Ƭ��Ϊ�գ��Ѹö�ʱ����
	// ���ʱ��Ƭ�ϵĶ�ʱ�������У�ͬʱ����ʱ����Ȧ����ʱ�临�Ӷȣ�������Ϊ
	// O(1)��������ʱ���еĶ�ʱ�������ӵ�ͬһ��ʱ��Ƭ�ϣ�ʱ�临�Ӷ�Ϊ O(n)��
	int addTimer(timer_event &timer);

	///
	///	ɾ����ʱ��
	///
	/// @return  ���ɾ���ɹ�����true,����Ϊfalse
	///
	bool  delete_timer(timer_event &times);

	// ���ݼ�ʱ���м�¼�� slot ��ֱ�Ӷ�λ���ҽ������ϵ�С�����Ӹ�С�����ϲ�
	// ��Ŀ�궨ʱ���ڵ㣬ʱ�临�Ӷ�ƽ��Ϊ O(1)�����Ϊ O(n)��
	bool scanTimer(timer_event &times);

	// ����ʱ��������ʱ�뽫Ҫָ����һ�� slotʱ������� slot ����������ڵ��
	// Ȧ�����м�һ���������������� 0�����ʾ�ýڵ��ϵļ�ʱ����ʱ��ʱ�临��
	// ��ƽ��Ϊ O(1)�����Ϊ O(n)��
	void timerTick();

	// �˳���ʱ��
	void timerStop();

private:
	CircQueue *ring_queue_;
	
};

#endif