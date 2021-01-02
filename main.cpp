#include "ScheduleCommon.h"
#include "ScheduleTask.h"
#include "ScheduleData.h"
#include "ScheduleView.h"

#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <map>
#include <string>
#include <tuple>
#include <memory>
#include <cstdio>
#include <clocale>

#include "absl/strings/numbers.h"
#include "absl/strings/str_split.h"
#include "ortools/base/commandlineflags.h"
#include "ortools/base/filelineiter.h"
#include "ortools/base/logging.h"
#include "ortools/sat/cp_model.h"
#include "ortools/sat/model.h"

#include <fmt/core.h>


using Day = std::vector<std::size_t>;
using Group = std::vector<Day>;
using Schedule = std::vector<Group>;


Schedule MakeLessonsSchedule(const ScheduleTask& task)
{
    using operations_research::sat::BoolVar;
    using operations_research::sat::CpModelBuilder;
    using operations_research::sat::LinearExpr;
    using operations_research::sat::Model;
    using operations_research::sat::CpSolverResponse;

    using operations_research::sat::SolutionBooleanValue;
    using operations_research::sat::NewSatParameters;
    using operations_research::sat::NewFeasibleSolutionObserver;
    using operations_research::sat::Solve;

    CpModelBuilder cp_model;
    std::map<std::tuple<std::size_t, std::size_t, std::size_t>, BoolVar> lessons;

    // создание переменных
    for(std::size_t d = 0; d < DAYS_IN_SCHEDULE; ++d)
    {
        for(std::size_t g = 0; g < task.CountGroups(); ++g)
        {
            for(std::size_t s = 0; s < task.CountSubjects(); ++s)
                lessons[{d, g, s}] = cp_model.NewBoolVar();
        }
    }

    // для каждой группы в день не может быть более 5 пар
    for(std::size_t d = 0; d < DAYS_IN_SCHEDULE; ++d)
    {
        for(std::size_t g = 0; g < task.CountGroups(); ++g)
        {
            std::vector<BoolVar> sum_subjects;
            sum_subjects.reserve(task.CountSubjects());
            for(std::size_t s = 0; s < task.CountSubjects(); ++s)
                sum_subjects.emplace_back(lessons[{d, g, s}]);

            cp_model.AddLessOrEqual(LinearExpr::BooleanSum(sum_subjects), task.LessonsPerDay());
        }
    }

    // в сумме для одной группы за весь период должно быть ровно стролько пар, сколько выделено на каждый предмет
    for (std::size_t g = 0; g < task.CountGroups(); ++g)
    {
        for(std::size_t s = 0; s < task.CountSubjects(); ++s)
        {
            std::vector<BoolVar> sum_days;
            sum_days.reserve(DAYS_IN_SCHEDULE);
            for (std::size_t d = 0; d < DAYS_IN_SCHEDULE; ++d)
                sum_days.emplace_back(lessons[{d, g, s}]);

            cp_model.AddEquality(LinearExpr::BooleanSum(sum_days), task.CountLessonsForGroup(g, s));
        }
    }

    const CpSolverResponse response = Solve(cp_model.Build());
    //LOG(INFO) << CpSolverResponseStats(response);

    Schedule schedule;
    schedule.reserve(task.CountGroups());
    for (std::size_t g = 0; g < task.CountGroups(); ++g)
    {
        Group group;
        group.reserve(DAYS_IN_SCHEDULE);
        for (std::size_t d = 0; d < DAYS_IN_SCHEDULE; ++d)
        {
            Day day;
            day.reserve(task.CountSubjects());
            for (std::size_t s = 0; s < task.CountSubjects(); ++s)
            {
                if (SolutionBooleanValue(response, lessons[{d, g, s}]))
                    day.emplace_back(s + 1);
                else
                    day.emplace_back(0);
            }
            group.emplace_back(std::move(day));
        }

        schedule.emplace_back(std::move(group));
    }

    return schedule;
}

Schedule OptimizeWindows(Schedule schedule)
{
    for(auto&& group : schedule)
    {
        for(auto&& day : group)
        {
            std::partition(day.begin(), day.end(), [](int s){ return s > 0; });
            day.erase(day.begin() + MAX_LESSONS_PER_DAY, day.end());
        }
    }

    return schedule;
}

ScheduleData MakeScheduleData(const Schedule& schedule,
                              const std::vector<std::string>& groups,
                              const std::vector<std::string>& subjects)
{
    std::vector<GroupSchedule> groupsSchedules;
    groupsSchedules.reserve(std::size(groups));
    for (std::size_t g = 0; g < groups.size(); ++g)
    {
        std::vector<DaySchedule> daySchedules;
        for (std::size_t d = 0; d < DAYS_IN_SCHEDULE; ++d)
        {
            std::vector<std::string> lessons;
            lessons.reserve(MAX_LESSONS_PER_DAY);
            for (auto&& lesson : schedule.at(g).at(d))
                lessons.emplace_back(subjects.at(lesson));

            daySchedules.emplace_back(std::move(lessons));
        }

        groupsSchedules.emplace_back(groups.at(g), std::move(daySchedules));
    }

    return ScheduleData(std::move(groupsSchedules));
}

int main(int argc, char* argv[])
{
    try {
        std::locale::global(std::locale("ru_Ru.UTF-8"));

        const std::vector<std::string> groups = {"IST", "PI"};
        const std::vector<std::string> subjects = {"#",
                                                   "Mathematics",
                                                   "Informatics",
                                                   "Economics",
                                                   "English",
                                                   "Accounting",
                                                   "Management"};

        const ScheduleTask task(2, {
                std::vector<std::size_t>({10, 4, 2, 2, 2, 2}),
                std::vector<std::size_t>({7, 4, 2, 2, 2, 2})
        });


        const auto schedule = OptimizeWindows(MakeLessonsSchedule(task));
        const auto data = MakeScheduleData(schedule, groups, subjects);
        ConsoleScheduleView().Show(data);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
