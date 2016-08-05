#include "TimeWheel.h"
#include "CircQueue.h"


CTimeWheel::CTimeWheel(int iPoolThreadNum)
	: ring_queue_(new CircQueue(iPoolThreadNum))
{
}

CTimeWheel::~CTimeWheel()
{
	if (NULL != ring_queue_)
	{
		delete ring_queue_;
		ring_queue_ = NULL;
	}
}

// ���ݶ�ʱ���ĳ�ʱʱ��(interval)��ʱ��Ƭ��ʱ��(timeslot)������õ��ö�ʱ
// ����ʱ�����ʱ��Ƭ����(ticks)�����ݸö�ʱ�������ʱ��Ƭ����(ticks)�͵�
// ǰ��ʱ��Ƭ�ţ��ó��ö�ʱ����ʱ������Ӧ����ʱ��Ƭ�ţ������ʱ��Ƭ��Ϊ��
// �ڵ㣬�ͰѸö�ʱ�����������ʱ��Ƭ�ϣ������ʱ��Ƭ��Ϊ�գ��Ѹö�ʱ����
// ���ʱ��Ƭ�ϵĶ�ʱ�������У�ͬʱ����ʱ����Ȧ����ʱ�临�Ӷȣ�������Ϊ
// O(1)��������ʱ���еĶ�ʱ�������ӵ�ͬһ��ʱ��Ƭ�ϣ�ʱ�临�Ӷ�Ϊ O(n)��
int CTimeWheel::addTimer(timer_event &timer)
{
	return ring_queue_->addTimer(timer);
}

// ���ݼ�ʱ���м�¼�� slot ��ֱ�Ӷ�λ���ҽ������ϵ�С�����Ӹ�С�����ϲ�
// ��Ŀ�궨ʱ���ڵ㣬ʱ�临�Ӷ�ƽ��Ϊ O(1)�����Ϊ O(n)��
bool CTimeWheel::scanTimer(timer_event &timer)
{
	//ring_queue_.entries[times->slotNum];
	return true;
}

bool CTimeWheel::delete_timer(timer_event &timer)
{
	return ring_queue_->delete_timer(timer);
}

// ����ʱ��������ʱ�뽫Ҫָ����һ�� slotʱ������� slot ����������ڵ��
// Ȧ�����м�һ���������������� 0�����ʾ�ýڵ��ϵļ�ʱ����ʱ��ʱ�临��
// ��ƽ��Ϊ O(1)�����Ϊ O(n)��
void CTimeWheel::timerTick()
{
	ring_queue_->timerTick();
}

void CTimeWheel::timerStop()
{
	ring_queue_->timerStop();
}
