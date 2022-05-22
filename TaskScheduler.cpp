#include "TaskScheduler.h"


TaskScheduler::TaskScheduler(int threadsNumber)
{
    _executors.reserve(threadsNumber);
    for (int i = 0; i < threadsNumber; ++i)
    {
        _executors.emplace_back(&TaskScheduler::taskExecutor, this);
    }
}


TaskScheduler::~TaskScheduler()
{
    stop();
}


void TaskScheduler::addTask(std::shared_ptr<Task> task, int delayMs, bool repeatable)
{
    {
        std::unique_lock<std::mutex> lock(_tasksMutex);
        const auto expectedExecutionTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(delayMs);

        TaskInfo info{std::move(task), repeatable ? std::optional<int>{delayMs} : std::optional<int>{}};
        _tasks.emplace(expectedExecutionTime, std::move(info));
    }
    _tasksCv.notify_all();
}


void TaskScheduler::stop()
{
    if (_stop.exchange(true))
    {
        return;
    }

    _tasksCv.notify_all();

    // wait for all threads
    for (auto& executor : _executors)
    {
        executor.join();
    }
}


void TaskScheduler::taskExecutor()
{
    std::shared_ptr<Task> taskToExecute;
    while (!_stop)
    {
        {
            std::unique_lock<std::mutex> lock(_tasksMutex);
            auto top = _tasks.begin();
            if (top == _tasks.end())
            {
                // Optimization: wait for task in queue here or for stop (notify_all is called in stop())
                _tasksCv.wait(lock);
                continue;
            }

            const auto expectedExecutionTime = top->first;
            const auto currentTime = std::chrono::steady_clock::now();
            if (expectedExecutionTime > currentTime)
            {
                // Optimization: wait for task OR for specific amount of time till the top task
                _tasksCv.wait_for(lock, expectedExecutionTime - currentTime); // TODO: threshold should be added here
                continue;
            }

            // remove task from the queue
            auto taskInfo = std::move(top->second);
            taskToExecute = taskInfo.task;
            _tasks.erase(top);

            // reschedule task if it is repeatable
            // TODO: consider the case when actual execution time > period ?
            if (taskInfo.repeatPeriodMs)
            {
                const auto newExpectedExecutionTime = expectedExecutionTime + std::chrono::milliseconds(*taskInfo.repeatPeriodMs);
                _tasks.emplace(newExpectedExecutionTime, std::move(taskInfo));
            }
        }

        taskToExecute->execute();
    }
}
