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
    using Log = std::vector<std::string>;
    static std::vector<std::string> log;
private:
    static std::mutex _logMutex;
    static void addLog(std::string record)
    {
        std::lock_guard<std::mutex> guard(_logMutex);
        log.emplace_back(std::move(record));
    }
};

TestEchoTask::Log TestEchoTask::log;
std::mutex TestEchoTask::_logMutex;

int main()
{
    std::cout << "Running test...\n";

    // Fill tasks
    TaskScheduler scheduler(5);
    scheduler.addTask(std::make_shared<TestEchoTask>(3, 200), 200);
    scheduler.addTask(std::make_shared<TestEchoTask>(4, 100), 100);
    scheduler.addTask(std::make_shared<TestEchoTask>(5, 100), 500, true);
    std::this_thread::sleep_for(std::chrono::seconds(5));
    scheduler.stop();

    // Check log
    const auto& log = TestEchoTask::log;
    assert(log.size() == 24);

    assert(log[0] == "BEGIN ID:4");
    assert(log[1] == "BEGIN ID:3");
    assert(log[2] == "END ID:4");
    assert(log[3] == "END ID:3");
    assert(log[4] == "BEGIN ID:5");
    assert(log[5] == "END ID:5");
    assert(log[6] == "BEGIN ID:5");
    assert(log[7] == "END ID:5");
    assert(log[8] == "BEGIN ID:5");
    assert(log[9] == "END ID:5");
    assert(log[10] == "BEGIN ID:5");
    assert(log[11] == "END ID:5");
    assert(log[12] == "BEGIN ID:5");
    assert(log[13] == "END ID:5");
    assert(log[14] == "BEGIN ID:5");
    assert(log[15] == "END ID:5");
    assert(log[16] == "BEGIN ID:5");
    assert(log[17] == "END ID:5");
    assert(log[18] == "BEGIN ID:5");
    assert(log[19] == "END ID:5");
    assert(log[20] == "BEGIN ID:5");
    assert(log[21] == "END ID:5");
    assert(log[22] == "BEGIN ID:5");
    assert(log[23] == "END ID:5");

    std::cout << "done\n";
}
