#include <iostream>

#include "TaskScheduler.h"


class TestEchoTask : public Task
{
public:
    TestEchoTask(uint64_t id, uint64_t executionTimeMs) :
        _id(id),
        _executionTime((executionTimeMs))
    {
        // Do nothing
    }

    virtual void execute() override
    {
        TestEchoTask::addLog(std::string("BEGIN ID:") + std::to_string(_id));
        std::this_thread::sleep_for(_executionTime);
        TestEchoTask::addLog(std::string("END ID:") + std::to_string(_id));
    }

private:
    uint64_t _id;
    std::chrono::milliseconds _executionTime;

public:
    static std::vector<std::string> log;
private:
    static std::mutex _logMutex;
    static void addLog(std::string record)
    {
        std::lock_guard<std::mutex> guard(_logMutex);
        log.emplace_back(std::move(record));
    }
};

std::vector<std::string> TestEchoTask::log;
std::mutex TestEchoTask::_logMutex;

int main()
{
    TaskScheduler scheduler(5);
    scheduler.addTask(std::make_shared<TestEchoTask>(3, 200), 200);
    scheduler.addTask(std::make_shared<TestEchoTask>(4, 100), 100);
    scheduler.addTask(std::make_shared<TestEchoTask>(5, 100), 500, true);
    std::this_thread::sleep_for(std::chrono::seconds(5));
    scheduler.stop();

    std::cout << "=== START ===\n";
    for (const auto& rec : TestEchoTask::log)
    {
        std::cout << rec << "\n";
    }
    std::cout << "=== END ===\n";

    // Check log
}
