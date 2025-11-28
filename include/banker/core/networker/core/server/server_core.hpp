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
        };

    private:

    public:
        server_core();
        ~server_core();

        server_core(const server_core&)             = delete;
        server_core& operator=(const server_core&)  = delete;

        server_core(server_core&&)                  = default;
        server_core& operator=(server_core&&)       = default;
    };
}

#endif //BANKER_SERVER_CORE_HPP