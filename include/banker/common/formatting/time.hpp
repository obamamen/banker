//
// Created by moosm on 11/5/2025.
//

#ifndef BANKER_TIME_HPP
#define BANKER_TIME_HPP
#include <chrono>
#include <ctime>
#include <iomanip>
#include <string>

namespace banker::common::formatting
{
    inline std::string get_current_time(
        bool easy_printing = false,
        const std::string &b = "{",
        const std::string &a = "}")
    {
        const auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::ostringstream oss;
        if (easy_printing) oss << b;
        oss << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S");
        if (easy_printing) oss << a << " ";
        return oss.str();
    }
}

#endif //BANKER_TIME_HPP