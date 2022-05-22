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
    std::lock_guard<std::mutex> guard(_tasksMutex);
    const auto expectedExecutionTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(delayMs);

    TaskInfo info{std::move(task), repeatable ? std::optional<int>{delayMs} : std::optional<int>{}};
    _tasks.emplace(expectedExecutionTime, std::move(info));
}


void TaskScheduler::stop()
{
    if (_stop)
    {
        return;
    }

    _stop = true;

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
            std::lock_guard<std::mutex> guard(_tasksMutex);
            auto top = _tasks.begin();
            if (top == _tasks.end())
            {
                // TODO: Optimization can be added:
                //       - wait for task in queue here?
                continue;
            }

            const auto expectedExecutionTime = top->first;
            const auto currentTime = std::chrono::steady_clock::now();
            if (expectedExecutionTime > currentTime)
            {
                // TODO: Optimization can be added:
                //       - wait for task OR for specific amount of time till the next task
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
