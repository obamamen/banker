/* ================================== *\
 @file     dense_storage.hpp
 @project  banker
 @author   moosm
 @date     12/11/2025
*\ ================================== */

#ifndef BANKER_DENSE_STORAGE_HPP
#define BANKER_DENSE_STORAGE_HPP

#include <vector>

#include "banker/shared/compat.hpp"

namespace banker::networker
{
    template<typename T>
    class dense_storage
    {
    public:
        size_t add(T&& value)
        {
            _items.push_back(std::move(value));
            return _items.size() - 1;
        }

        void remove(size_t index)
        {
            if (index >= _items.size()) return;
            if (index != _items.size() - 1)
                _items[index] = std::move(_items.back());
            _items.pop_back();
        }

        T& operator[](size_t index) { return _items[index]; }
        T* data() { return _items.data(); }
        BANKER_NODISCARD size_t size() const { return _items.size(); }

        auto begin() { return _items.begin(); }
        auto end() { return _items.end(); }

    private:
        std::vector<T> _items;
    };
}

#endif //BANKER_DENSE_STORAGE_HPP