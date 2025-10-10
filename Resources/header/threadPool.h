#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <future>
#include <thread>
#include <mutex>
#include <queue>
#include <unordered_map>

enum class TaskType
{
	GENERAL
};


class ThreadPool
{
public:
	ThreadPool(size_t numThreads = std::thread::hardware_concurrency() - 1);
	~ThreadPool();

	// enqueue a function with param
	template<class Func, typename... Args>
	void	enqueue(TaskType type, Func&& f, Args&&... args)
	{
		{
			std::unique_lock<std::mutex> lock(m_mutexes[type]);
			// anqueue the function and its arguments as a callable without parameters
			m_queues[type].emplace(std::bind(std::forward<Func>(f), std::forward<Args>(args)...));
		}
		m_cvs[type].notify_one();
	}

	// enqueue a member function with param
	template <typename Obj, typename MemFunc, typename... Args>
	void	enqueue(TaskType type, Obj* obj, MemFunc func, Args&&... args)
	{
		{
			std::unique_lock<std::mutex> lock(m_mutexes[type]);
			// call member function
			m_queues[type].emplace([obj, func, args...]() mutable {
				(obj->*func)(args...);
				});
		}
		m_cvs[type].notify_one();
	}

private :

	void	workerLoop(TaskType myType);

	std::atomic<bool>												m_stop = false;
	std::vector<std::thread>										m_workers;
	std::unordered_map<TaskType, std::queue<std::function<void()>>> m_queues;
	std::unordered_map<TaskType, std::mutex>                        m_mutexes;
	std::unordered_map<TaskType, std::condition_variable>           m_cvs;
};

#endif