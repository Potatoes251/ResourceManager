#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <future>
#include <thread>
#include <mutex>
#include <queue>
#include <unordered_map>

class ThreadPool
{
public:
	ThreadPool(size_t numThreads = std::thread::hardware_concurrency() - 1);
	~ThreadPool();

	// enqueue a function with param
	template<class Func, typename... Args>
	void	enqueue(Func&& f, Args&&... args)
	{
		{
			std::unique_lock<std::mutex> lock(m_mutexes);
			// anqueue the function and its arguments as a callable without parameters
			m_queues.emplace(std::bind(std::forward<Func>(f), std::forward<Args>(args)...));
		}
		m_cvs.notify_one();
	}

	// enqueue a member function with param
	template <typename Obj, typename MemFunc, typename... Args>
	void	enqueue(Obj* obj, MemFunc func, Args&&... args)
	{
		{
			std::unique_lock<std::mutex> lock(m_mutexes);
			// call member function
			m_queues.emplace([obj, func, args...]() mutable {
				(obj->*func)(args...);
				});
		}
		m_cvs.notify_one();
	}

private :

	void	workerLoop();

	std::atomic<bool>												m_stop = false;
	std::vector<std::thread>										m_workers;
	std::queue<std::function<void()>> m_queues;
	std::mutex                        m_mutexes;
	std::condition_variable           m_cvs;
};

#endif