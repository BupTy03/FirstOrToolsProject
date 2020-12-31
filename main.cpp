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

void NurseSchedulingProblem()
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

    constexpr int num_nurses = 4;
    constexpr int num_shifts = 3;
    constexpr int num_days = 3;

    const auto all_nurses = range(num_nurses);
    const auto all_shifts = range(num_shifts);
    const auto all_days = range(num_days);

    CpModelBuilder cp_model;

    std::map<std::tuple<int, int, int>, BoolVar> shifts;
    for (auto n : all_nurses) {
        for (auto d : all_days) {
            for (auto s : all_shifts) {
                shifts[{n, d, s}] = cp_model.NewBoolVar().WithName(
                        "shift_n" + std::to_string(n) + 'd' + std::to_string(d) + 's' + std::to_string(s));
            }
        }
    }

    for (auto d : all_days) {
        for (auto s : all_shifts) {
            std::vector<BoolVar> v;
            for (auto n : all_nurses)
                v.emplace_back(shifts[{n, d, s}]);

            cp_model.AddEquality(LinearExpr::BooleanSum(v), 1);
        }
    }

    for (auto n : all_nurses) {
        for (auto d : all_days) {
            std::vector<BoolVar> v;
            for (auto s : all_shifts)
                v.emplace_back(shifts[{n, d, s}]);

            cp_model.AddLessOrEqual(LinearExpr::BooleanSum(v), 1);
        }
    }

    int min_shifts_per_nurse = (num_shifts * num_days) / num_nurses;
    int max_shifts_per_nurse = min_shifts_per_nurse;
    if ((num_shifts * num_days) % num_nurses == 0)
        max_shifts_per_nurse = min_shifts_per_nurse;
    else
        max_shifts_per_nurse = min_shifts_per_nurse + 1;

    for (auto n : all_nurses) {
        std::vector<BoolVar> v;
        for (auto d : all_days) {
            for (auto s : all_shifts) {
                v.emplace_back(shifts[{n, d, s}]);
            }
        }

        auto num_shifts_worked = LinearExpr::BooleanSum(v);
        cp_model.AddLessOrEqual(min_shifts_per_nurse, num_shifts_worked);
        cp_model.AddLessOrEqual(num_shifts_worked, max_shifts_per_nurse);
    }

    const CpSolverResponse response = Solve(cp_model.Build());
    LOG(INFO) << CpSolverResponseStats(response);

    for (auto d : all_days) {
        LOG(INFO) << "Day " << d;
        for (auto n : all_nurses) {
            bool is_working = false;
            for (auto s : all_shifts) {
                if (SolutionBooleanValue(response, shifts[{n, d, s}])) {
                    is_working = true;
                    LOG(INFO) << "\tNurse " << n << " works shift " << s;
                }
            }

            if (!is_working) {
                LOG(INFO) << "\tNurse " << n << " does not work";
            }
        }
    }
}

struct FileCloser {
    void operator()(FILE* f) const {
        fclose(f);
    }
};

using FilePtr = std::unique_ptr<FILE, FileCloser>;

void PrintSchedule(const Schedule& schedule,
                   const std::vector<std::string>& groups,
                   const std::vector<std::string>& subjects)
{
    FilePtr file(fopen("out.csv", "w"));

    const std::vector<std::string> days = {
            "Понедельник, числитель",
            "Вторник, числитель",
            "Среда, числитель",
            "Четверг, числитель",
            "Пятница, числитель",
            "Суббота, числитель",

            "Понедельник, знаменатель",
            "Вторник, знаменатель",
            "Среда, знаменатель",
            "Четверг, знаменатель",
            "Пятница, знаменатель",
            "Суббота, знаменатель",
    };

    fmt::print(file.get(), ";");
    for (std::size_t g = 0; g < groups.size(); ++g)
    {
        fmt::print(file.get(), "{};", groups.at(g));
    }
    fmt::print(file.get(), "\n");

    for (std::size_t d = 0; d < days.size(); ++d)
    {
        fmt::print(file.get(), "{}\n", days.at(d));
        for (std::size_t s = 0; s < subjects.size() - 1; ++s)
        {
            fmt::print(file.get(), ";");
            for (std::size_t g = 0; g < groups.size(); ++g)
            {
                fmt::print(file.get(), "{};", subjects.at(schedule.at(g).at(d).at(s)));
            }
            fmt::print(file.get(), "\n");
        }
    }
}

int main(int argc, char* argv[])
{
    const std::vector<std::string> groups = {"ИСТ", "ПИ"};
    const std::vector<std::string> subjects = {"Окно",
                                               "Математика",
                                               "Информатика",
                                               "Экономика",
                                               "Английский язык",
                                               "Бухгалтерия",
                                               "Делопроизводство"};

    const auto schedule = MakeLessonsSchedule(12, groups.size(), 6, 5, {
            std::vector<int>({10, 4, 2, 2, 2, 2}),
            std::vector<int>({10, 4, 2, 2, 2, 2})
    });

    PrintSchedule(schedule, groups, subjects);
    return 0;
}
