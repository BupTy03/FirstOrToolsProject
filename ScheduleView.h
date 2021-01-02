#pragma once
#include "ScheduleData.h"

#include <string>


class ScheduleView
{
public:
    ~ScheduleView() = default;
    virtual void Show(const ScheduleData& data) = 0;
};


std::string AlignCenter(std::string str, std::size_t n);


class ConsoleScheduleView : public ScheduleView
{
public:
    void Show(const ScheduleData& data) override;
};
