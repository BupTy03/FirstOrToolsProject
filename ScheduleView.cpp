#include "ScheduleView.h"
#include "ScheduleCommon.h"

#include <iostream>


std::string AlignCenter(std::string str, std::size_t n)
{
    const auto sz = std::size(str);
    if(sz >= n)
        return str;

    const auto fields = n - sz;
    const auto marginLeft = fields / 2;
    const auto marginRight = fields - marginLeft;

    str.insert(std::begin(str), marginLeft, ' ');
    str.insert(std::end(str), marginRight, ' ');
    return str;
}


void ConsoleScheduleView::Show(const ScheduleData& data)
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
    for (auto&& group : data.Groups())
        std::cout << align(group.Name()) << '|';
    std::cout << '\n';

    for (std::size_t d = 0; d < days.size(); ++d)
    {
        std::cout << std::string(94, '-') << '\n';
        std::cout << '|' << std::string(31, ' ') << align(days.at(d)) << std::string(31, ' ') << "|\n";
        std::cout << std::string(94, '-') << '\n';
        for (std::size_t lesson = 0; lesson < MAX_LESSONS_PER_DAY; ++lesson)
        {
            std::cout << '|' << std::string(30, ' ') << '|';
            for (auto&& group : data.Groups())
            {
                std::cout << align(group.Days().at(d).Lessons().at(lesson)) << '|';
            }
            std::cout << '\n';
        }
    }
}
