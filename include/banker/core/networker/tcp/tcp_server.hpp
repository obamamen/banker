/* ================================== *\
 @file     tcp_server.hpp
 @project  banker
 @author   moosm
 @date     11/18/2025
*\ ================================== */

#ifndef BANKER_TCP_SERVER_HPP
#define BANKER_TCP_SERVER_HPP

#include <cstdint>

#include "banker/core/networker/core/socket/socket.hpp"
#include "banker/core/networker/core/tcp/packet_stream_core.hpp"

namespace banker::networker
{
    class tcp_server
    {
    public:
        using client_id = uint64_t;

    private:
        struct internal_client
        {
            client_id id;
            socket socket;
            packet_stream_core packet_stream_core;
        };

    public:
        tcp_server()                                = default;
        ~tcp_server()                               = default;

        tcp_server(const tcp_server&)               = delete;
        tcp_server& operator=(const tcp_server&)    = delete;

        tcp_server(tcp_server&&)                    = default;
        tcp_server& operator=(tcp_server&&)         = default;

        explicit tcp_server(
            const uint16_t port,
            const std::string& ip = "0.0.0.0",
            const int backlog = 1024)
        {
            this->create();
            this->bind(port, ip);
            this->listen(backlog);
        }

        bool is_ready() const { return _ready; }

        bool create()
        {
            const bool valid = packet_stream_host_core::create(
                _host);

            return valid;
        }

        bool bind(
            const uint16_t port,
            const std::string & ip = "0.0.0.0")
        {
            const bool valid = packet_stream_host_core::bind(
                _host,
                port,
                ip);

            return valid;
        }

        bool listen(
            const int backlog = 1024)
        {
            const bool valid = packet_stream_host_core::listen(
                _host,
                backlog);

            if (valid == true) _ready = true;

            return valid;
        }

    private:
        socket _host{};
        std::vector<internal_client> _internal_clients{};

        std::unordered_map<client_id, size_t> _client_to_internal{};

        bool _ready = false;
    };
}

#endif //BANKER_TCP_SERVER_HPP