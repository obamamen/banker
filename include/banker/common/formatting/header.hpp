//
// Created by moosm on 11/5/2025.
//

#ifndef BANKER_SIMPLE_HEADER_HPP
#define BANKER_SIMPLE_HEADER_HPP

#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>

namespace banker::common::formatting
{
    template<typename... T>
    inline std::string format(T... values)
    {
        std::stringstream ss{};
        (ss << ... << values);
        return ss.str();
    }

    inline void print_divider(
        const int width = 50,
        const char c = '=',
        const std::string& title = "",
        const char sides = '\0',
        std::ostream& out = std::cout)
    {
        std::string divider(width, c);
        if (sides != 0)
        {
            divider[0] = sides;
            divider[width - 1] = sides;
        }

        for (int i = 0; i < title.size(); i++)
        {
            size_t index = (width/2) - (title.size() / 2) + i;
            if (index < 0) continue;
            if (index >= width) continue;
            divider[index] = title[i];
        }

        out << divider << std::endl;
    }
}

#endif //BANKER_SIMPLE_HEADER_HPP
