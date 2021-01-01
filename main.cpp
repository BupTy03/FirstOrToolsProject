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


using Day = std::vector<int>;
using Group = std::vector<Day>;
using Schedule = std::vector<Group>;


auto range(int n)
{
    std::vector<int> result(n);
    std::iota(result.begin(), result.end(), 0);
    return result;
}


Schedule MakeLessonsSchedule(int num_days,
                         int num_groups,
                         int num_subjects,
                         int max_lessons_per_day,
                         const std::vector<std::vector<int>>& count_lessons_for_subject_for_group)
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
    for(std::size_t d = 0; d < num_days; ++d)
    {
        for(std::size_t g = 0; g < num_groups; ++g)
        {
            for(std::size_t s = 0; s < num_subjects; ++s)
                lessons[{d, g, s}] = cp_model.NewBoolVar();
        }
    }

    // для каждой группы в день не может быть более 5 пар
    for(std::size_t d = 0; d < num_days; ++d)
    {
        for(std::size_t g = 0; g < num_groups; ++g)
        {
            std::vector<BoolVar> sum_subjects;
            sum_subjects.reserve(num_subjects);
            for(std::size_t s = 0; s < num_subjects; ++s)
                sum_subjects.emplace_back(lessons[{d, g, s}]);

            cp_model.AddLessOrEqual(LinearExpr::BooleanSum(sum_subjects), max_lessons_per_day);
        }
    }

    // в сумме для одной группы за весь период должно быть ровно стролько пар, сколько выделено на каждый предмет
    for (std::size_t g = 0; g < num_groups; ++g)
    {
        for(std::size_t s = 0; s < num_subjects; ++s)
        {
            std::vector<BoolVar> sum_days;
            sum_days.reserve(num_days);
            for (std::size_t d = 0; d < num_days; ++d)
                sum_days.emplace_back(lessons[{d, g, s}]);

            cp_model.AddEquality(LinearExpr::BooleanSum(sum_days), count_lessons_for_subject_for_group.at(g).at(s));
        }
    }

    const CpSolverResponse response = Solve(cp_model.Build());
    //LOG(INFO) << CpSolverResponseStats(response);

    Schedule schedule;
    schedule.reserve(num_groups);
    for (std::size_t g = 0; g < num_groups; ++g)
    {
        Group group;
        group.reserve(num_days);
        for (std::size_t d = 0; d < num_days; ++d)
        {
            Day day;
            day.reserve(num_subjects);
            for (std::size_t s = 0; s < num_subjects; ++s)
            {
                if (SolutionBooleanValue(response, lessons[{d, g, s}]))
                {
                    day.emplace_back(s + 1);
                }
                else
                {
                    day.emplace_back(0);
                }
            }
            group.emplace_back(std::move(day));
        }

        schedule.emplace_back(std::move(group));
    }

    return schedule;
}

std::string AlignCenter(std::string str, std::size_t n)
{
    const auto sz = std::size(str);
    if(sz >= n)
        return str;

    const auto fields = (n - sz);
    const auto marginLeft = fields / 2;
    const auto marginRight = fields - marginLeft;

    str.insert(std::begin(str), marginLeft, ' ');
    str.insert(std::end(str), marginRight, ' ');
    return str;
}

void PrintSchedule(const Schedule& schedule,
                   const std::vector<std::string>& groups,
                   const std::vector<std::string>& subjects)
{
    const std::vector<std::string> days = {
            "Monday",
            "Tuesday",
            "Wednesday",
            "Thursday",
            "Friday",
            "Saturday",

            "Monday",
            "Tuesday",
            "Wednesday",
            "Thursday",
            "Friday",
            "Saturday",
    };

    auto align = [] (std::string s) { return AlignCenter(std::move(s), 30); };

    std::cout << align(" ") << " |";
    for (std::size_t g = 0; g < groups.size(); ++g)
    {
        std::cout << align(groups.at(g)) << '|';
    }
    std::cout << '\n';

    for (std::size_t d = 0; d < days.size(); ++d)
    {
        std::cout << std::string(94, '-') << '\n';
        std::cout << '|' << std::string(31, ' ') << align(days.at(d)) << std::string(31, ' ') << "|\n";
        std::cout << std::string(94, '-') << '\n';
        for (std::size_t s = 0; s < subjects.size() - 1; ++s)
        {
            std::cout << '|' << std::string(30, ' ') << '|';
            for (std::size_t g = 0; g < groups.size(); ++g)
            {
                std::cout << align(subjects.at(schedule.at(g).at(d).at(s))) << '|';
            }
            std::cout << '\n';
        }
    }
}

int main(int argc, char* argv[])
{
    std::locale::global(std::locale("ru_Ru.UTF-8"));
    const std::vector<std::string> groups = {"IST", "PI"};
    const std::vector<std::string> subjects = {"#",
                                               "Mathematics",
                                               "Informatics",
                                               "Economics",
                                               "English",
                                               "Accounting",
                                               "Management"};

    const auto schedule = MakeLessonsSchedule(12, groups.size(), 6, 2, {
            std::vector<int>({10, 4, 2, 2, 2, 2}),
            std::vector<int>({7, 4, 2, 2, 2, 2})
    });

    PrintSchedule(schedule, groups, subjects);
    return 0;
}
