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
        client_manager() = default;

        client_id add_client(client_data_t&& data)
        {
            const client_id id = _next_id++;

            _clients.emplace_back(std::move(data));
            const size_t idx = _clients.size() - 1;

            _id_to_index[id] = idx;
            _index_to_id.push_back(id);

            return id;
        }

        client_data_t* get_client(const client_id id)
        {
            const auto it = _id_to_index.find(id);
            if (it == _id_to_index.end()) return nullptr;
            return &_clients[it->second];
        }

        void remove_client(const client_id id)
        {
            auto it = _id_to_index.find(id);
            if (it == _id_to_index.end()) return;
            size_t idx = it->second;

            const size_t last = _clients.size() - 1;
            if (idx != last)
            {
                std::swap(_clients[idx], _clients[last]);
                const client_id moved_id = _index_to_id[last];
                _id_to_index[moved_id] = idx;
                _index_to_id[idx] = moved_id;
            }

            _clients.pop_back();
            _index_to_id.pop_back();
            _id_to_index.erase(id);
        }

    private:
        client_id _next_id{0};

        std::vector<client_data_t>              _clients;
        std::vector<client_id>                  _index_to_id;
        std::unordered_map<client_id, size_t>   _id_to_index;
    };
}

#endif //BANKER_CLIENT_MANAGER_HPP