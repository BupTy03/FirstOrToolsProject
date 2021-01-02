#pragma once

#include "ScheduleCommon.h"

#include <string>
#include <vector>
#include <stdexcept>


enum class ScheduleDay {
    MondayEven,
    TuesdayEven,
    WednesdayEven,
    ThursdayEven,
    FridayEven,
    SaturdayEven,

    MondayOdd,
    TuesdayOdd,
    WednesdayOdd,
    ThursdayOdd,
    FridayOdd,
    SaturdayOdd
};

struct DaySchedule
{
    explicit DaySchedule(std::vector<std::string> lessons)
        : lessons_(std::move(lessons))
    {
        if(std::size(lessons_) > MAX_LESSONS_PER_DAY)
            throw std::invalid_argument("Lessons count is greater than MAX_LESSONS");
    }

    const std::vector<std::string>& Lessons() const { return lessons_; }

private:
    std::vector<std::string> lessons_;
};

struct GroupSchedule
{
    static constexpr std::size_t COUNT_DAYS = 12;

    explicit GroupSchedule(std::string groupName, std::vector<DaySchedule> schedule)
        : name_(std::move(groupName))
        , days_(std::move(schedule))
    {
        if(std::empty(name_))
            throw std::invalid_argument("Group name is empty");

        if(std::size(days_) != COUNT_DAYS)
            throw std::invalid_argument("Invalid days count");
    }

    const std::string& Name() const { return name_; }
    const std::vector<DaySchedule>& Days() const { return days_; }

private:
    std::string name_;
    std::vector<DaySchedule> days_;
};


struct ScheduleData
{
    explicit ScheduleData(std::vector<GroupSchedule> groups)
        : groups_(groups)
    {}

    const std::vector<GroupSchedule>& Groups() const { return groups_; }
    std::size_t CountGroups() const { return groups_.size(); }

private:
    std::vector<GroupSchedule> groups_;
};
