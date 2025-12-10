/* ================================== *\
 @file     debugger.hpp
 @project  banker
 @author   moosm
 @date     12/10/2025
*\ ================================== */

#ifndef BANKER_DEBUGGER_HPP
#define BANKER_DEBUGGER_HPP

#include <fstream>
#include <iostream>
#include <string>
#include <mutex>
#include <chrono>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <sstream>

namespace banker::debug
{
    inline std::mutex& debug_mutex()
    {
        static std::mutex mtx;
        return mtx;
    }

    inline std::ofstream& debug_file()
    {
        static std::ofstream file([]
        {
            const auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            const std::tm tm = *std::localtime(&t);

            std::ostringstream oss;
            oss << "debug/";
            oss << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");
            oss << ".debug.txt";

            return std::ofstream(oss.str(), std::ios::app);
        }());

        return file;
    }

    template<typename... Args>
    inline void log(Args&&... args)
    {
        std::lock_guard<std::mutex> lock(debug_mutex());
        ((debug_file() << std::forward<Args>(args)), ...) << std::endl;
    }
}

#endif //BANKER_DEBUGGER_HPP