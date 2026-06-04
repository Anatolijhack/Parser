#include "ThreadPool.h"
ThreadPool::ThreadPool(int n)
{
	for (int i = 0; i < n; i++)
	{
		workers.emplace_back(&ThreadPool::worker, this);
	}
}
ThreadPool::~ThreadPool()
{
	{
		std::lock_guard<std::mutex> lock(mtx);
		stop = true;
	}
	cv.notify_all();
	for (auto& t : workers)
	{
		t.join();
	}
}
void ThreadPool::enqueue(std::function<void()> task)
{
	activeTask++;

	{
		std::lock_guard<std::mutex> lock(mtx);

		tasks.push([this, task]()
			{
				try
				{
					task();
				}
				catch (...)
				{
					// лог при желании
				}

				if (--activeTask == 0)
				{
					std::lock_guard<std::mutex> lock(mtx);
					cv.notify_all();
				}
			});
	}

	cv.notify_one();
}
void ThreadPool::worker()
{
	while (true)
	{
		std::function<void()> task;
		{

			std::unique_lock<std::mutex> lock(mtx);
			cv.wait(lock, [this] {return stop || !tasks.empty(); });

			if (stop && tasks.empty())
			{
				return;
			}
			task = tasks.front();
			tasks.pop();

		}
		task();
	}
}
void ThreadPool::waitFinished()
{
	std::unique_lock<std::mutex> lock(mtx);

	cv.wait(lock, [this]()
		{
			return activeTask.load() == 0;
		});
}