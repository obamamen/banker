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

namespace banker::networker
{
    class server_core
    {
    public:
        using client_id = uint64_t;
        static constexpr client_id invalid_client_id = static_cast<client_id>( ~0 );

        struct poll_event
        {
            client_id id = invalid_client_id;
            bool readable{false};
            bool writable{false};
            bool error{false};
        };

    private:
        struct client_info
        {
            client_id id;
            networker::socket sock;
            bool marked_for_disconnect{false};
        };

    public:
        server_core();
        ~server_core();

        server_core(const server_core&)             = delete;
        server_core& operator=(const server_core&)  = delete;

        server_core(server_core&&)                  = default;
        server_core& operator=(server_core&&)       = default;
    private:
        networker::socket _host{};
        std::map<client_id, client_info> _clients{};
    };
}

#endif //BANKER_SERVER_CORE_HPP