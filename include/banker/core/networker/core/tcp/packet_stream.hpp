/* ================================== *\
 @file     packet_stream.hpp
 @project  banker
 @author   moosm
 @date     11/10/2025
*\ ================================== */

#ifndef BANKER_STREAM_SOCKET_HPP
#define BANKER_STREAM_SOCKET_HPP

#include <cstdint>
#include <deque>
#include <vector>

#include "banker/core/networker/core/socket/socket.hpp"

#include "banker/core/networker/core/tcp/packet_stream_core.hpp"
#include "banker/core/networker/core/tcp/stream_handler.hpp"

#include "banker/core/networker/core/packet.hpp"

namespace banker::networker
{
    class packet_stream
    {
        friend class stream_socket_host;
    private:
        explicit packet_stream(socket&& socket) : _socket(std::move(socket)) {}
    public:
        packet_stream() = default;
        packet_stream(const packet_stream&) = delete;
        packet_stream& operator=(const packet_stream&) = delete;
        packet_stream(packet_stream&& socket) noexcept : _socket(std::move(socket._socket)) {}
        packet_stream& operator=(packet_stream&& socket) noexcept
            { _socket = std::move(socket._socket); return *this; }

        /// tries to connect.
        /// @param port port.
        /// @param host IP address.
        /// @return returns if connection was successful.
        bool connect(
            const u_short port,
            const std::string& host = "0.0.0.0")
        {
            auto c = _socket.create();
            if (c == false) return false;
            return _socket.connect(host, port);
        }


        /// @brief tries to send packet.
        /// it will first try to flush its queued packages, and then the current packet.
        /// @param packet packet to try to send, might get added to internal buffer if it couldn't send.
        /// @return see `send_data`.
        [[nodiscard]] packet_stream_core::send_data send_packet(const packet& packet)
        {
            return core.send(_socket, packet);
        }

        /// @brief flushes the internal out buffer.
        /// call this if you don't have any packets to send, maybe in a tick loop.
        /// @return see `send_data`.
        [[nodiscard]] packet_stream_core::send_data flush_send_buffer()
        {
            return {false,core.flush_send_buffer(_socket) };
        }

        /// @brief checks to see if underlying socket is valid.
        /// @return
        bool is_valid() const
        {
            return _socket.is_valid();
        }

        /// @brief updates the receive buffer with incoming the TCP stream.
        void tick()
        {
            core.tick_receive(_socket);
        }

        /// @brief returns all the packets currently in the buffer.
        /// @note call 'tick()' before calling this function.
        /// @return vector of deserialized packets.
        [[nodiscard]] std::vector<packet> receive_packets()
        {
            return core.receive_packets();
        }

        /// @brief returns the oldest packet currently in the buffer.
        /// @note call @ref tick() "tick()" before calling this function.
        /// @return single packet, check for validness.
        [[nodiscard]] packet receive_packet()
        {
            return core.receive_packet();
        }

    private:
        socket _socket{};
        packet_stream_core core{};
    };

    class stream_socket_host
    {
    public:
        stream_socket_host() = default;
        ~stream_socket_host() = default;
        stream_socket_host(const stream_socket_host&) = delete;
        stream_socket_host& operator=(const stream_socket_host&) = delete;
        stream_socket_host(stream_socket_host&&) = default;
        stream_socket_host& operator=(stream_socket_host&&) = default;

        bool create_and_bind(
            const uint16_t port,
            const std::string& ip = "0.0.0.0")
        {
            if (!_socket.create(AF_INET, SOCK_STREAM)) return false;

            if (!_socket.set_reuse_address(true)) return false;

            if (!_socket.set_blocking(false)) return false;

            if (!_socket.bind(port, ip)) return false;

            return _socket.listen(1024);
        }

        /// @brief calls .accept , and convert it into a packet_stream.
        /// @return the stream socket, needs to be checked for validness.
        packet_stream accept() const
        {
            socket client_socket = _socket.accept();
            if (!client_socket.is_valid()) return {};
            if (!client_socket.set_blocking(false))
            {
                auto _ = client_socket.close();
                return {};
            }
            return packet_stream(std::move(client_socket));
        }
    private:
        socket _socket{};
    };
}

#endif //BANKER_STREAM_SOCKET_HPP