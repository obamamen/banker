/* ================================== *\
 @file     stream_server.hpp
 @project  banker
 @author   moosm
 @date     12/4/2025
*\ ================================== */

#ifndef BANKER_STREAM_SERVER_HPP
#define BANKER_STREAM_SERVER_HPP

#include "banker/core/networker/core/server/server_core.hpp"
#include "../../../../../old/tcp/stream_core.hpp"

namespace banker::networker
{
    class stream_server
    {
    public:
        stream_server()     = default;
        ~stream_server()    = default;

        stream_server(const stream_server&)             = delete;
        stream_server& operator=(const stream_server&)  = delete;

        stream_server(stream_server&&)                  = default;
        stream_server& operator=(stream_server&&)       = default;

        bool host(
            const std::string& ip,
            const u_short port,
            const int back_log = 1024)
        {
            _core.set_host(
                stream_core::generate_server_socket(
                    ip,
                    port,
                    back_log));

            return _core.get_host().is_valid();
        }

        const server_core<stream_core>& get_core()
        {
            return _core;
        }

        client_id accept_new()
        {
            return _core.accept_new();
        }

        void poll()
        {
            _core.poll();
        }

        bool next_poll_result(server_poll_result& result)
        {
            return _core.next_poll_result(result);
        }

        std::vector<uint8_t>& receive_from_client(const client_id id)
        {
            auto& d = _core.get_client_data(id);
            d.core.tick_receive(d.socket);
            return d.core.receive();
        }

    private:
        server_core<stream_core> _core{};
    };
}

#endif //BANKER_STREAM_SERVER_HPP