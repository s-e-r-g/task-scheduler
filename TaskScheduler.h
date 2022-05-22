#pragma once

#include <condition_variable>
#include <chrono>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

#include "Task.h"


class TaskScheduler
{
public:
    TaskScheduler(int threadsNumber = 1);
    ~TaskScheduler();

    void addTask(std::shared_ptr<Task> task, int delayMs = 0, bool repeatable = false);
    void stop();

private:
    struct TaskInfo
    {
        std::shared_ptr<Task> task;
        std::optional<int> repeatPeriodMs;
    };

private:
    std::atomic<bool> _stop = false;
    std::mutex _tasksMutex;
    std::condition_variable _tasksCv;
    // TODO: probably, std:make_heap can be used instead of map (check performance)
    std::multimap<std::chrono::time_point<std::chrono::steady_clock>, TaskInfo> _tasks;
    std::vector<std::thread> _executors;

private:
    void taskExecutor();
};
