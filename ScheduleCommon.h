#pragma once

#include <cstddef>

static constexpr std::size_t MAX_LESSONS_PER_DAY = 5;
static constexpr std::size_t DAYS_IN_SCHEDULE = 12;

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

