#pragma once
#include <cstddef>
#include <vector>


struct ScheduleTask
{
    explicit ScheduleTask(std::size_t countLessonsPerDay,
                          std::vector<std::vector<std::size_t>> countLessonsPerSubjectForGroup)
        : countGroups_(std::size(countLessonsPerSubjectForGroup))
        , countSubjects_(std::size(countLessonsPerSubjectForGroup.front()))
        , countLessonsPerDay_(countLessonsPerDay)
        , countLessonsPerSubjectForGroup_(std::move(countLessonsPerSubjectForGroup))
    {
    }

    std::size_t CountGroups() const { return countGroups_; }
    std::size_t CountSubjects() const { return countSubjects_; }
    std::size_t LessonsPerDay() const { return countLessonsPerDay_; }
    std::size_t CountLessonsForGroup(std::size_t group, std::size_t subject) const
    {
        return countLessonsPerSubjectForGroup_.at(group).at(subject);
    }

private:
    std::size_t countGroups_;
    std::size_t countSubjects_;
    std::size_t countLessonsPerDay_;
    std::vector<std::vector<std::size_t>> countLessonsPerSubjectForGroup_;
};
