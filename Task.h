#pragma once

class Task
{
public:
    virtual ~Task() = default;
    virtual void execute() = 0;
};
