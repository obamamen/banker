/* ================================== *\
 @file     server_core.hpp
 @project  banker
 @author   moosm
 @date     11/20/2025
*\ ================================== */

#ifndef BANKER_SERVER_CORE_HPP
#define BANKER_SERVER_CORE_HPP

#include <cstdint>

#include "banker/core/networker/core/socket/socket.hpp"
#include "banker/core/networker/core/server/client_manager.hpp"
#include "banker/core/networker/core/socket/polling.hpp"

namespace banker::networker
{
    struct server_poll_result
    {
        client_id client_id = invalid_client_id;
        poll_group::result result{};
    };

    template<typename core_t>
    class server_core
    {
    public:
        struct client_data
        {
            socket socket{socket::invalid_socket};
            core_t core;
        };

        using poll_result = server_poll_result;

    public:
        explicit server_core(socket&& host)
            : _host(std::move(host))
        {}

        server_core()                               = default;
        ~server_core()                              = default;

        server_core(const server_core&)             = delete;
        server_core& operator=(const server_core&)  = delete;

        server_core(server_core&&)                  = default;
        server_core& operator=(server_core&&)       = default;

        void set_host(socket&& host)
        {
            _host = std::move(host);
        }

        const socket& get_host() const
        {
            return _host;
        }

        client_id accept_new()
        {
            if (!_host.is_valid()) return invalid_client_id;
            socket s = _host.accept();
            if (!s.is_valid()) return invalid_client_id;

            _poll_group_dirty = true;

            return _clients.add_client({std::move(s),{}});
        }

        void poll()
        {
            if (_poll_group_dirty) _clean_poll_group();
            _poll_group.poll(10);
        }

        bool next_poll_result(poll_result& result)
        {
            const int index = _poll_group.next_result(result.result);

            if (index == -1) return false;
            result.client_id = _clients.get()[index];

            return true;
        }

        client_data& get_client_data(client_id id)
        {
            client_data* data = _clients.get_client(id);
            return (*data);
        }

    private:
        poll_group _poll_group;
        bool _poll_group_dirty{true};

        socket _host{};

        client_manager<client_data> _clients{};

    private:
        void _clean_poll_group()
        {
            _poll_group.reset();
            _poll_group.reserve( _clients.size() );
            const auto clients = _clients.get();

            for (const auto& client : clients)
            {
                _poll_group.add(
                    _clients.get_client(client)->socket);
            }

            _poll_group_dirty = false;
        }
    };
}

#endif //BANKER_SERVER_CORE_HPP