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
    /// @brief a variable struct that saves a variable state.
    /// @tparam T the type.
    template<typename T>
    struct variable
    {
        const char* name;
        const T& value;
    };

    /// @brief tries to turn any T into a string.
    /// @tparam T the type.
    /// @param value the value.
    /// @return the created string.
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

    /// @brief if no variables it just doesn't edit the stream.
    inline void build_stream(std::ostringstream&) {}

    /// @brief (SHOULD IGNORE) the c++ mess of templated recursion
    /// @tparam First ...
    /// @tparam Rest ...
    /// @param oss ...
    /// @param first ...
    /// @param rest ...
    template<typename First, typename... Rest>
    void build_stream(std::ostringstream& oss, const First& first, const Rest&... rest)
    {
        oss << first.name << "=" << to_string_safe(first.value);
        if constexpr (sizeof...(rest) > 0) oss << ", ";
        build_stream(oss, rest...);
    }

    /// @brief turns an X amount of variables into [name=value, name2=value2, ...]
    /// @tparam Vars the variable types.
    /// @param vars the actual variables.
    /// @return the created string.
    /// @note usage:
    /// @code{.cpp}
    /// std::cout << INSPECT(INSPECT_V(port), INSPECT_V(ip));
    /// >>> [port=8080, ip=127.0.0.1]
    /// @endcode
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
///< wraps a variable with its name and value for use with INSPECT().
///< @code{.cpp}
///< INSPECT_V(port) -> "port=127.0.0.1" @endcode

#define INSPECT(...) banker::debug_inspector::vars(__VA_ARGS__)
///< converts one or more INSPECT_V(x) items into a formatted string.
///< @code{.cpp}
///< INSPECT(INSPECT_V( a ), INSPECT_V( b )) -> "[a=1, b=2]" @endcode

#endif //BANKER_DEBUG_INSPECTOR_HPP