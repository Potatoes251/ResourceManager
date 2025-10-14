#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <unordered_map>
#include <string>
#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>

#include "threadPool.h"

// add timer when creating resources
#if 1
#include <chrono>
#define TEST_TIME
#endif


class IResource
{
public:
	virtual ~IResource() = default;
};


class ResourceManager
{
public:
	ResourceManager(bool multi) : m_multithread(multi) {}
	~ResourceManager()
	{
		clearAll();
	}

	void	Delete(std::string id)
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_resourceMap.erase(id);
	}

	void	clearAll()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_resourceMap.clear();	// Explicitly destroys all unique_ptr-held resources
	}

	// create a resource on another thread without callback
	template <typename T>
	void	createAsync(std::string id)
	{
		m_pool.enqueue(this, &ResourceManager::Create<T>, id);
	}

	// create a resource on another thread with callback
	template <typename T, typename Callback, typename... Args>
	void	createAsync(std::string id, Callback&& onLoaded, Args&&... args)
	{
		m_pool.enqueue([this, id,
			onLoaded = std::forward<Callback>(onLoaded),
			argsTuple = std::make_tuple(std::forward<Args>(args)...)]() mutable
			{
				Create<T>(id);
				T* result = Get<T>(id);

				std::apply([&](auto&&... unpackedArgs) {
					onLoaded(result, std::forward<decltype(unpackedArgs)>(unpackedArgs)...);
					}, std::move(argsTuple));
			});
	}

	template <typename T>
	void	Create(std::string id)
	{
#ifdef TEST_TIME
		auto start = std::chrono::high_resolution_clock::now();
#endif
		if (Get<T>(id) != nullptr)
		{
			Delete(id);
		}

		std::unique_ptr<T> resource = nullptr;

		// Try to construct with (string, ThreadPool&)
		if constexpr (std::is_constructible_v<T, std::string, ThreadPool&>)
		{
			if (m_multithread)
				resource = std::make_unique<T>(id, m_pool);
			else
				resource = std::make_unique<T>(id);
		}
		// Otherwise, fallback to (string)
		else if constexpr (std::is_constructible_v<T, std::string>)
		{
			resource = std::make_unique<T>(id);
		}
		else
		{
			printf("Type T must be constructible with (std::string) or (std::string, ThreadPool&)");
		}

		if (resource)
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			T* rawPtr = resource.get();
			m_resourceMap[id] = std::move(resource);
		}
#ifdef TEST_TIME
		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsed = end - start;
		printf("Parsed %s in %fs (%s)\n", id.c_str(), elapsed.count(), m_multithread ? "multithread" : "singlethread");
#endif
	}

	template <typename T>
	T*		Get(std::string id)
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		auto it = m_resourceMap.find(id);

		if (it != m_resourceMap.end())
		{
			return dynamic_cast<T*>(it->second.get());
		}
		else
		{
			return nullptr;
		}
	}

	ThreadPool		m_pool;
private:

	bool			m_multithread;
	std::mutex		m_mutex;

	std::unordered_map<std::string, std::unique_ptr<IResource>> m_resourceMap;
};

#endif // !RESOURCE_MANAGER_H