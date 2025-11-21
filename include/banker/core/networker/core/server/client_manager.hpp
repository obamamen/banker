/* ================================== *\
 @file     client_manager.hpp
 @project  banker
 @author   moosm
 @date     11/21/2025
*\ ================================== */

#ifndef BANKER_CLIENT_MANAGER_HPP
#define BANKER_CLIENT_MANAGER_HPP

#include <cstdint>
#include <unordered_map>
#include <vector>
#include <cassert>

namespace banker::networker
{
    using client_id = uint64_t;
    static constexpr client_id invalid_client_id = static_cast<client_id>( ~0 );

    template<typename client_data_t>
    class client_manager
    {
    public:
        client_manager()                                    = default;
        ~client_manager()                                   = default;

        client_manager(const client_manager&)               = delete;
        client_manager& operator=(const client_manager&)    = delete;

        client_manager(client_manager&&)                    = default;
        client_manager& operator=(client_manager&&)         = default;

        client_id add_client(const client_data_t&& client_data)
        {
            const client_id id = _next_id++;

            _clients.push_back( std::move(client_data) );
            _index[id] = _clients.size() - 1;

            return id;
        }

        client_data_t* get_client(const client_id id)
        {
            if (!_index.contains(id)) return nullptr;

            assert(_index[id] < _clients.size());

            return &_clients[_index[id]];
        }

        void remove_client(const client_id id)
        {
            assert(_index.contains(id));
            assert(_index[id] < _clients.size());

            const auto it = _index.find(id);
            if (it == _index.end())
                return;

            size_t idx  = it->second;
            size_t last = _clients.size() - 1;

            if (idx != last)
            {
                std::swap(_clients[idx], _clients[last]);

                for (auto& [other_id, other_idx] : _index)
                    if (other_idx == last)
                    {
                        other_idx = idx;
                        break;
                    }

            }

            _clients.pop_back();
            _index.erase(it);
        }

    private:
        client_id _next_id{0};
        std::vector<client_data_t> _clients{};
        std::unordered_map<client_id, size_t> _index{};
    };
}

#endif //BANKER_CLIENT_MANAGER_HPP