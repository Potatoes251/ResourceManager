#include "threadPool.h"



ThreadPool::ThreadPool(size_t numThreads)
{
    for (TaskType type : { TaskType::GENERAL })
    {
        m_mutexes[type];
        m_cvs[type];
        m_queues[type];
    }

    printf("Creating %zu threads\n", numThreads);
    for (size_t i = 0; i < numThreads; i++)
    {
        m_workers.emplace_back(&ThreadPool::workerLoop, this, TaskType::GENERAL);
    }
}

ThreadPool::~ThreadPool()
{
    m_stop = true;
        for (auto &[type, cv] : m_cvs)
            cv.notify_all();

    for (auto& worker : m_workers)
    {
        worker.join();
    }
}

void    ThreadPool::workerLoop(TaskType myType)
{
    while (true)
    {
        // take a function like : void task();
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(m_mutexes[myType]);
            m_cvs[myType].wait(lock, [this, myType]
                {
                    return m_stop || !m_queues[myType].empty();
                });
            if (m_stop && m_queues[myType].empty())
                return;
            task = std::move(m_queues[myType].front());
            m_queues[myType].pop();
        }
        task();
    }
}
