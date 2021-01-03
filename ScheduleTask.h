#pragma once

#include "ScheduleCommon.h"

#include <cstddef>
#include <vector>
#include <map>
#include <set>

using LessonWishes = std::set<std::size_t>;
using DayWishes = std::map<ScheduleDay, LessonWishes>;
using GroupWishes = std::map<std::size_t, DayWishes>;


struct ScheduleTask
{
    explicit ScheduleTask(std::size_t countLessonsPerDay,
                          std::vector<std::vector<std::size_t>> countLessonsPerSubjectForGroup,
                          std::map<std::size_t, GroupWishes> wishes)
        : countGroups_(std::size(countLessonsPerSubjectForGroup))
        , countSubjects_(std::size(countLessonsPerSubjectForGroup.front()))
        , countLessonsPerDay_(countLessonsPerDay)
        , countLessonsPerSubjectForGroup_(std::move(countLessonsPerSubjectForGroup))
        , wishes_(std::move(wishes))
    {
    }

    std::size_t CountGroups() const { return countGroups_; }
    std::size_t CountSubjects() const { return countSubjects_; }
    std::size_t CountLessonsPerDay() const { return countLessonsPerDay_; }
    std::size_t CountLessonsForGroup(std::size_t group, std::size_t subject) const
    {
        return countLessonsPerSubjectForGroup_.at(group).at(subject);
    }

    std::map<std::tuple<std::size_t, std::size_t, std::size_t, std::size_t>, bool> Requests() const
    {
        std::map<std::tuple<std::size_t, std::size_t, std::size_t, std::size_t>, bool> result;
        for(std::size_t d = 0; d < DAYS_IN_SCHEDULE; ++d)
        {
            const auto scheduleDay = static_cast<ScheduleDay>(d);
            for(std::size_t g = 0; g < CountGroups(); ++g)
            {
                for(std::size_t l = 0; l < CountLessonsPerDay(); ++l)
                {
                    for (std::size_t s = 0; s < CountSubjects(); ++s)
                    {
                        if(wishes_.count(s) && wishes_.at(s).count(g) && wishes_.at(s).at(g).count(scheduleDay) && wishes_.at(s).at(g).at(scheduleDay).count(l))
                            result[{d, g, l, s}] = true;
                        else
                            result[{d, g, l, s}] = false;
                    }
                }
            }
        }

        return result;
    }

private:
    std::size_t countGroups_;
    std::size_t countSubjects_;
    std::size_t countLessonsPerDay_;
    std::vector<std::vector<std::size_t>> countLessonsPerSubjectForGroup_;
    std::map<std::size_t, GroupWishes> wishes_;
};
