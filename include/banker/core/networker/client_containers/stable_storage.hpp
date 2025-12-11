/* ================================== *\
 @file     stable_storage.hpp
 @project  banker
 @author   moosm
 @date     12/11/2025
*\ ================================== */

#ifndef BANKER_STABLE_STORAGE_HPP
#define BANKER_STABLE_STORAGE_HPP

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "banker/shared/compat.hpp"

namespace banker::networker
{
    using stable_id = uint64_t;
    static constexpr stable_id invalid_id = ~0ULL;

    template<typename T>
    class stable_storage
    {
    public:
        stable_storage() = default;
        ~stable_storage() = default;

        stable_id add(T&& value)
        {
            const stable_id id = _next_id++;
            _items.push_back(std::move(value));
            const size_t index = _items.size() - 1;

            _id_to_index[id] = index;
            _index_to_id.push_back(id);
            return id;
        }

        void remove(const stable_id id)
        {
            const auto it = _id_to_index.find(id);
            if (it == _id_to_index.end()) return;

            size_t index = it->second;
            size_t last = _items.size() - 1;

            BANKER_LIKELY if (index != last)
            {
                _items[index] = std::move(_items[last]);
                const stable_id moved_id = _index_to_id[last];
                _id_to_index[moved_id] = index;
                _index_to_id[index] = moved_id;
            }

            _items.pop_back();
            _index_to_id.pop_back();
            _id_to_index.erase(id);
        }

        T* get(const stable_id id)
        {
            const auto it = _id_to_index.find(id);
            return (it != _id_to_index.end()) ? &_items[it->second] : nullptr;
        }

        T& raw_at(size_t index) { return _items[index]; }

        stable_id id_at(const size_t index) const { return _index_to_id[index]; }

        size_t size() const { return _items.size(); }

        stable_id index_to_id(const size_t index) const { return _index_to_id[index]; }

    private:
        stable_id _next_id{0};
        std::vector<T> _items;
        std::vector<stable_id> _index_to_id;
        std::unordered_map<stable_id, size_t> _id_to_index;
    };
}

#endif //BANKER_STABLE_STORAGE_HPP