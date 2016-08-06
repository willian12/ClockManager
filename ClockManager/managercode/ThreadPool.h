#ifdef WIN32
#pragma once
#endif // WIN32

#ifndef THREADPOOL_H_
#define THREADPOOL_H_


#include <iostream>  
#include <stdlib.h>  
#include <functional>  
#include <thread>  
#include <string>  
#include <mutex>  
#include <condition_variable>  
#include <vector>  
#include <memory>  
#include <assert.h>  
#include <algorithm>  
#include <queue>  
//#include <process.h>

// 循环定时器用来执行定时器事件的线程池
class ThreadPool
{
public:
	typedef std::function<void()> Task;

	ThreadPool(int threads_num)
		: threads_num_(threads_num)
		, max_task_queue_size_(0)
		, running_(false)
	{
	}

	~ThreadPool()
	{
		if (running_) {
			stop();
		}
	}

	ThreadPool(const ThreadPool&) = delete;
	void operator=(const ThreadPool&) = delete;

	void setMaxQueueSize(int max_task_queue_size)
	{
		max_task_queue_size_ = max_task_queue_size;
	}

	void start()
	{
		assert(threads_.empty());
		running_ = true;
		threads_.reserve(threads_num_);
		for (int i = 0; i<threads_num_; i++) {
			threads_.push_back(
				std::unique_ptr<std::thread>(
					new std::thread(
						std::bind(&ThreadPool::runInThread, this)
					)
					)
			);
		}
	}

	void stop()
	{
		{
			std::unique_lock<std::mutex> ul(mutex_);
			running_ = false;
			notEmpty_.notify_all();
		}

		for (auto &iter : threads_) {
			iter->join();
		}
	}

	void run(const Task &t)
	{
		if (threads_.empty()) {
			t();
		}
		else {
			std::unique_lock<std::mutex> ul(mutex_);
			while (isFull()) {
				notFull_.wait(ul);
			}
			assert(!isFull());
			queue_.push_back(t);
			notEmpty_.notify_one();
		}
	}

private:
	bool isFull() const
	{
		return max_task_queue_size_ > 0 && queue_.size() >= max_task_queue_size_;
	}

	void runInThread()
	{
		while (running_) {
			Task task(getTask());
			if (task) {
				task();
			}
		}
	}

	Task getTask()
	{
		std::unique_lock<std::mutex> ul(mutex_);
		while (queue_.empty() && running_) {
			notEmpty_.wait(ul);
		}

		Task task;
		if (!queue_.empty()) {
			task = queue_.front();
			queue_.pop_front();
			if (max_task_queue_size_ > 0) {
				notFull_.notify_one();
			}
		}

		return task;
	}

private:
	std::mutex mutex_;
	std::condition_variable notEmpty_;
	std::condition_variable notFull_;

	int threads_num_;
	std::vector<std::unique_ptr<std::thread>> threads_;

	std::deque<Task> queue_;
	size_t max_task_queue_size_;

	bool running_;
};

#endif // !THREADPOOL_H_
