/* ================================== *\
 @file     packet_stream_core.hpp
 @project  banker
 @author   moosm
 @date     12/1/2025
*\ ================================== */

#ifndef BANKER_PACKET_STREAMER_CIRE_HPP
#define BANKER_PACKET_STREAMER_CIRE_HPP

#include <cstdint>

#include "banker/core/networker/core/packet/packet.hpp"
#include "banker/core/networker/core/tcp/stream_core.hpp"

namespace banker::networker
{
    class packet_stream_core
    {
    public:
        packet_stream_core()                                    = default;
        ~packet_stream_core()                                   = default;

        packet_stream_core(const packet_stream_core&)           = delete;
        packet_stream_core& operator=(const packet_stream_core&)= delete;

        packet_stream_core(packet_stream_core&&)                = default;
        packet_stream_core& operator=(packet_stream_core&&)     = default;

        void tick(socket& socket)
        {
            _stream_core.flush_send_buffer(socket);
        }

        void write_packet(socket& socket, packet& packet)
        {
            packet::header header = packet.generate_header_net();
            socket::iovec_c v[2]
            {
                {&header, sizeof(header)},
                {static_cast<const void *>(packet.get_data().data()), packet.get_data().size_bytes()}
            };
            _stream_core.write_v(socket, v);
        }

        packet read_packet(socket& socket)
        {
            _stream_core.tick_receive(socket);
            return packet::deserialize(_stream_core.receive());
        }

        static socket generate_client_socket(
            const std::string& ip,
            const unsigned short port)
        {
            return stream_core::generate_client_socket(ip, port);
        }

        static socket generate_server_socket(
            const std::string& ip,
            const unsigned short port,
            const int backlog = 1024)
        {
            return stream_core::generate_server_socket(ip, port, backlog);
        }

    private:
        stream_core _stream_core;
    };
}

#endif //BANKER_PACKET_STREAMER_CIRE_HPP