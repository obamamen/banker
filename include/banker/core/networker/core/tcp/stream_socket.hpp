//
// Created by moosm on 11/10/2025.
//

#ifndef BANKER_STREAM_SOCKET_HPP
#define BANKER_STREAM_SOCKET_HPP

#include <cstdint>
#include <deque>
#include <vector>

#include "send_buffer.hpp"
#include "banker/core/networker/core/packet.hpp"
#include "banker/core/networker/core/socket/socket.hpp"
#include "banker/core/networker/core/tcp/stream_handler.hpp"

namespace banker::networker
{
    class stream_socket
    {
        friend class stream_socket_host;
    public:
        /// @brief return value for some functions.
        /// - .sent_total to check the total packets send this call.
        /// - .sent_current to check if it could the packet given to the function.
        struct send_data
        {
            bool sent_current{false};
            size_t sent_total{0};
        };

        explicit stream_socket(socket&& socket) : _socket(std::move(socket)) {}
    public:
        stream_socket() = default;
        stream_socket(const stream_socket&) = delete;
        stream_socket& operator=(const stream_socket&) = delete;
        stream_socket(stream_socket&& socket) noexcept : _socket(std::move(socket._socket)) {}
        stream_socket& operator=(stream_socket&& socket) noexcept
            { _socket = std::move(socket._socket); return *this; }

        /// tries to connect.
        /// @param port port.
        /// @param host IP address.
        /// @return returns if connection was successful.
        bool connect(
            const u_short port,
            const std::string& host = "0.0.0.0") const
        {
            return _socket.connect(host,port);
        }


        /// @brief tries to send packet.
        /// it will first try to flush its queued packages, and then the current packet.
        /// @param packet packet to try to send, might get added to internal buffer if it couldn't send.
        /// @return see `send_data`.
        [[nodiscard]] send_data send_packet(const packet& packet)
        {
            return _send(packet);
        }

        /// @brief flushes the internal out buffer.
        /// call this if you don't have any packets to send, maybe in a tick loop.
        /// @return see `send_data`.
        [[nodiscard]] send_data flush_send_buffer()
        {
            return {false,_flush_send_buffer() };
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
            tcp::stream_socket(_socket,_receive_buffer);
        }

        /// @brief returns all the packets currently in the buffer.
        /// @note call 'tick()' before calling this function.
        /// @return vector of deserialized packets.
        [[nodiscard]] std::vector<packet> receive_packets()
        {
            std::vector<packet> packets{};
            while (true)
            {
                packet p = tcp::get_packet_from_stream(_receive_buffer);
                if (!p.is_valid()) break;
                packets.push_back(p);
            }
            return packets;
        }

        /// @brief returns the oldest packet currently in the buffer.
        /// @note call @ref tick() "tick()" before calling this function.
        /// @return single packet, check for validness.
        [[nodiscard]] packet receive_packet()
        {
            return tcp::get_packet_from_stream(_receive_buffer);
        }

    private:
        socket _socket{};
        std::vector<uint8_t>        _receive_buffer{};
        std::deque<tcp::out_buffer> _send_buff{};

        send_data _send(const packet& packet)
        {
            const auto t = _flush_send_buffer();
            if (!_send_buff.empty()) return {false, t};

            const bool s = _try_send(packet);

            return {s,t + s};
        }

        /// @brief tries to send the packet over socket.
        /// if it can't send, it adds the packet to the internal buffer.
        /// @param packet packet to send.
        /// @return if successful.
        bool _try_send(const packet& packet)
        {
            packet::header h = packet.generate_header_net();
            auto size = packet.get_data().size();
            const auto& data = packet.get_data();
            const int sent = _socket.sendv({
                {&h, sizeof(h)},
                {packet.get_data().data(), size}
            });

            if ( sent == size + sizeof(h) ) return true;

            tcp::out_buffer buffer(size + sizeof(h) - sent);

            if (sent < sizeof(h))
            {
                ::memcpy(buffer.data(), reinterpret_cast<uint8_t*>(&h) + sent, sizeof(h) - sent);
                ::memcpy(buffer.data() + sizeof(h) - sent, data.data(), data.size());
            }
            else
            {
                ::memcpy(buffer.data(), data.data() + (sent - sizeof(h)), data.size() - (sent - sizeof(h)));
            }

            _send_buff.push_back( std::move(buffer) );

            return false;
        }

        /// @brief flushes the send buffer.
        /// pops all sent packets and updates the offset for the packets that might be half sent.
        /// @return how many full packets did get send.
        size_t _flush_send_buffer()
        {
            auto total_send = 0;
            while ( !_send_buff.empty() )
            {
                auto& buffer = _send_buff.front();
                const auto size = buffer.size();

                const int sent = _socket.send(
                    buffer.data(),
                    buffer.size());

                if (sent == size) // packet could send fully
                {
                    total_send++;
                    _send_buff.pop_front();
                    continue;
                }
                if (sent < 0) break; // error / or cant write.

                if (sent < size) // partial send, erase what did send
                {
                    buffer.consume(sent);
                    break;
                }
            }
            return total_send;
        }
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

        /// @brief calls .accept , and convert it into a stream_socket.
        /// @return the stream socket, needs to be checked for validness.
        stream_socket accept() const
        {
            socket client_socket = _socket.accept();
            if (client_socket.set_blocking(false))
            {
                auto _ =client_socket.close();
                return {};
            }
            return stream_socket(std::move(client_socket));
        }
    private:
        socket _socket{};
    };
}

#endif //BANKER_STREAM_SOCKET_HPP