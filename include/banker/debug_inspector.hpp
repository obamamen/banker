//
// Created by moosm on 11/3/2025.
//

#ifndef BANKER_DEBUG_INSPECTOR_HPP
#define BANKER_DEBUG_INSPECTOR_HPP

#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>

namespace banker::debug_inspector
{
    template<typename T>
    struct variable
    {
        const char* name;
        const T& value;
    };

    template<typename T>
    std::string to_string_safe(const T& value)
    {
        if constexpr (std::is_same_v<T, std::string>)
            return value;
        else if constexpr (std::is_same_v<T, const char*>)
            return std::string(value);
        else if constexpr (std::is_same_v<T, bool>)
            return value ? "true" : "false";
        else
            return std::to_string(value);
    }

    inline void build_stream(std::ostringstream&) {}

    template<typename First, typename... Rest>
    void build_stream(std::ostringstream& oss, const First& first, const Rest&... rest)
    {
        oss << first.name << "=" << to_string_safe(first.value);
        if constexpr (sizeof...(rest) > 0) oss << ", ";
        build_stream(oss, rest...);
    }

    template<typename... Vars>
    std::string vars(const Vars&... vars)
    {
        std::ostringstream oss;
        oss << "[";
        build_stream(oss, vars...);
        oss << "]";
        return oss.str();
    }
}

#define INSPECT_V(x) banker::debug_inspector::variable<decltype(x)>{#x, x}
#define INSPECT(...) banker::debug_inspector::vars(__VA_ARGS__)

#endif //BANKER_DEBUG_INSPECTOR_HPP