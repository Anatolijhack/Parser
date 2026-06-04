#pragma once
#include <condition_variable>
#include <functional>
#include <thread>
#include <queue>
class ThreadPool
{
public:
	ThreadPool(int n);
	~ThreadPool();
	void enqueue(std::function<void()> task);
	void waitFinished();
private:
	std::vector<std::thread> workers;
	std::queue < std::function<void()>> tasks;
	std::mutex mtx;
	std::condition_variable cv;
	std::atomic<int> activeTask{ 0 };
	bool stop = false;
	void worker();
};