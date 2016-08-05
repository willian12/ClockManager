#ifdef WIN32
#pragma once
#endif // WIN32
#ifndef SPINLOCK_MUTEX_H_
#define SPINLOCK_MUTEX_H_

#include <atomic>


// ����������߶Զ�ʱ�������ķ�������
class spinlock_mutex
{
	std::atomic_flag flag = ATOMIC_FLAG_INIT;
public:
	void lock()
	{
		while (flag.test_and_set(std::memory_order_acquire));
	}

	void unlock()
	{
		flag.clear(std::memory_order_release);
	}
};

#endif // SPINLOCK_MUTEX_H_