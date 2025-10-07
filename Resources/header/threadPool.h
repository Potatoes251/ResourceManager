#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <future>
#include <thread>
#include <mutex>
#include <queue>

class ThreadPool
{
public:
	ThreadPool(size_t numThreads = std::thread::hardware_concurrency());
	~ThreadPool();

	// enqueue a function with param
	template<class Func, typename... Args>
	void enqueue(Func&& f, Args&&... args)
	{
		{
			std::unique_lock<std::mutex> lock(m_queueMutex);
			// anqueue the function and its arguments as a callable without parameters
			m_tasks.emplace(std::bind(std::forward<Func>(f), std::forward<Args>(args)...));
		}
		m_condition.notify_one();
	}

	// enqueue a member function with param
	template <typename Obj, typename MemFunc, typename... Args>
	void enqueue(Obj* obj, MemFunc func, Args&&... args)
	{
		{
			std::unique_lock<std::mutex> lock(m_queueMutex);
			// call member function
			m_tasks.emplace([obj, func, args...]() mutable {
				(obj->*func)(args...);
				});
		}
		m_condition.notify_one();
	}

private :

	void workerLoop();

	std::vector<std::thread>			m_workers;
	std::queue<std::function<void()>>	m_tasks;

	std::mutex                          m_queueMutex;
	std::condition_variable             m_condition;
	bool                                m_stop = false;
};

#endif