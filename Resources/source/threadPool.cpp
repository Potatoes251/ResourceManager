#include "threadPool.h"



ThreadPool::ThreadPool(size_t numThreads)
{
    printf("Creating %zu threads\n", numThreads);
    for (size_t i = 0; i < numThreads; i++)
    {
        m_workers.emplace_back(&ThreadPool::workerLoop, this);
    }
}

ThreadPool::~ThreadPool()
{
    m_stop = true;
    m_cvs.notify_all();

    for (auto& worker : m_workers)
    {
        worker.join();
    }
}

void    ThreadPool::workerLoop()
{
    while (true)
    {
        // take a function like : void task();
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(m_mutexes);
            m_cvs.wait(lock, [this]
                {
                    return m_stop || !m_queues.empty();
                });
            if (m_stop && m_queues.empty())
                return;
            task = std::move(m_queues.front());
            m_queues.pop();
        }
        task();
    }
}
