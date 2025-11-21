/* ================================== *\
 @file     robin_hash.hpp
 @project  banker
 @author   moosm
 @date     11/21/2025
*\ ================================== */

#ifndef BANKER_ROBIN_HASH_HPP
#define BANKER_ROBIN_HASH_HPP

#include <cassert>
#include <cstdint>
#include <type_traits>
#include <vector>

namespace banker::common
{
    template<typename K, typename V, float load_factor = 0.7f>
    class robin_map
    {
        static_assert(std::is_integral_v<K>, "robin_hash only supports integral keys");
        static_assert(!std::is_signed_v<K>, "robin_hash only supports unsigned keys");
        static_assert(load_factor > 0 && load_factor < 1, "load_factor must be between 0 and 1");

    private:
        struct robin_node
        {
            K key = empty_key;
            V value{};
            uint8_t distance{};
        };

    public:
        robin_map() = default;
        ~robin_map() = default;
        robin_map(const robin_map&) = default;
        robin_map(robin_map&&) = default;
        robin_map& operator=(const robin_map&) = default;
        robin_map& operator=(robin_map&&) = default;

        explicit robin_map(const size_t initial_capacity = 128)
        {
            size_t cap = 1;
            while (cap < initial_capacity) cap <<= 1;
            table.resize(cap, robin_node{empty_key, V{}, 0});
            mask = cap - 1;
            count = 0;
        }

        static constexpr K empty_key = static_cast<K>( ~0 );

        void insert(K key, V value)
        {
            if ((count + 1) > table.size() * load_factor)
                _resize(table.size() * 2);

            size_t pos = key & mask;
            uint8_t dist = 0;

            while (true)
            {
                if (table[pos].key == empty_key)
                {
                    table[pos] = {key, std::move(value), dist};
                    ++count;
                    return;
                }

                if (table[pos].key == key)
                {
                    table[pos].value = std::move(value);
                    return;
                }

                if (table[pos].distance < dist)
                {
                    std::swap(key, table[pos].key);
                    std::swap(value, table[pos].value);
                    std::swap(dist, table[pos].distance);
                }

                pos = (pos + 1) & mask;
                ++dist;
                assert(dist < 255);
            }
        }

        V* find(K key)
        {
            size_t pos = key & mask;
            uint8_t dist = 0;

            while (table[pos].key != empty_key)
            {
                if (table[pos].key == key) return &table[pos].value;
                if (table[pos].distance < dist) return nullptr;

                pos = (pos + 1) & mask;
                ++dist;
            }
            return nullptr;
        }

        void erase(K key)
        {
            size_t pos = key & mask;
            uint8_t dist = 0;

            while (table[pos].key != empty_key)
            {
                if (table[pos].key == key)
                {
                    size_t next = (pos + 1) & mask;
                    while (table[next].key != empty_key && table[next].distance > 0)
                    {
                        table[pos] = table[next];
                        --table[pos].distance;
                        pos = next;
                        next = (next + 1) & mask;
                    }

                    table[pos].key = empty_key;
                    table[pos].distance = 0;
                    --count;
                    return;
                }

                if (table[pos].distance < dist) return;
                pos = (pos + 1) & mask;
                ++dist;
            }
        }

        void clear()
        {
            for (auto& e : table)
            {
                e.key = empty_key;
                e.distance = 0;
            }
            count = 0;
        }

        size_t size() const { return count; }

    private:
        std::vector<robin_node> table{};
        size_t mask{};
        size_t count{};

        void _resize(size_t new_size)
        {
            std::vector<robin_node> old_table = std::move(table);
            table.clear();
            table.resize(new_size, robin_node{empty_key, V{}, 0});
            mask = new_size - 1;
            count = 0;

            for (auto& e : old_table)
            {
                if (e.key != empty_key) insert(e.key, e.value);
            }
        }
    };
}

#endif //BANKER_ROBIN_HASH_HPP