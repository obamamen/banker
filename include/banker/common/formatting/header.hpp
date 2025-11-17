//
// Created by moosm on 11/5/2025.
//

#ifndef BANKER_SIMPLE_HEADER_HPP
#define BANKER_SIMPLE_HEADER_HPP

#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <map>
#include <unordered_map>

namespace banker::common::formatting
{
    template<typename T>
    std::ostream& operator<<(
        std::ostream& os,
        const std::vector<T>& v)
    {
        os << "[";
        for (size_t i = 0; i < v.size(); ++i)
        {
            os << v[i];
            if (i + 1 != v.size()) os << ", ";
        }
        os << "]";
        return os;
    }

    template<typename Map>
    std::ostream& map_print(std::ostream& os, const Map& m)
    {
        os << "{";
        for (auto it = m.begin(); it != m.end(); ++it)
        {
            os << it->first << ": " << it->second;
            if (std::next(it) != m.end()) os << ", ";
        }
        os << "}";
        return os;
    }

    template<typename K, typename V>
    std::ostream& operator<<(std::ostream& os, const std::map<K, V>& m)
    {
        return map_print(os, m);
    }

    template<typename K, typename V>
    std::ostream& operator<<(std::ostream& os, const std::unordered_map<K, V>& m)
    {
        return map_print(os, m);
    }

    template<typename Iterable>
    std::ostream& print_iterable(
        std::ostream& os,
        const Iterable& c)
    {
        os << "[";
        auto it = c.begin();
        while (it != c.end())
        {
            os << *it;
            if (++it != c.end()) os << ", ";
        }
        os << "]";
        return os;
    }

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
