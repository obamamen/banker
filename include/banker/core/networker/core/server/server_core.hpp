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

namespace banker::networker
{
    class server_core
    {
    public:
        struct core_client_data
        {
            socket socket{socket::invalid_socket};
            bool marked_for_disconnect{false};
        };

        struct poll_event
        {
            client_id id = invalid_client_id;
            bool readable{false};
            bool writable{false};
            bool error{false};
        };

    private:

    public:
        server_core();
        ~server_core();

        server_core(const server_core&)             = delete;
        server_core& operator=(const server_core&)  = delete;

        server_core(server_core&&)                  = default;
        server_core& operator=(server_core&&)       = default;

    private:
        networker::socket _host{};
        client_manager<core_client_data> _clients{};
    };
}

#endif //BANKER_SERVER_CORE_HPP