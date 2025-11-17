/* ================================== *\
 @file     packet_stream_core.hpp
 @project  banker
 @author   moosm
 @date     11/12/2025
*\ ================================== */

#ifndef BANKER_PACKET_STREAM_CORE_HPP
#define BANKER_PACKET_STREAM_CORE_HPP

#include <cstdint>
#include <deque>
#include <vector>

#include "banker/core/networker/core/tcp/stream_handler.hpp"
#include "banker/core/networker/core/tcp/send_buffer.hpp"
#include "banker/shared/compat.hpp"

namespace banker::networker
{
    class packet_stream_core
    {
    public:
        /// @brief return value for some functions.
        /// - .sent_total to check the total packets send this call.
        /// - .sent_current to check if it could the packet given to the function.
        struct send_data
        {
            bool sent_current{false};
            size_t sent_total{0};
        };

    public:
        packet_stream_core() = default;
        ~packet_stream_core() = default;
        packet_stream_core(const packet_stream_core&) = default;
        packet_stream_core& operator=(const packet_stream_core&) = default;
        packet_stream_core(packet_stream_core&&)  noexcept = default;
        packet_stream_core& operator=(packet_stream_core&&) = default;

        /// @brief streams the socket stream into internal buffer, should be called regularly.
        /// @param socket socket to receive from.
        /// @return returns if error, can be checked with 'get_last_socket_error()'
        bool tick_receive(const socket& socket)
        {
            return tcp::stream_socket(socket,_receive_buffer);
        }

        /// @brief tries to deserialize packets currently inside its receive buffer.
        /// @return owning vector of packets, might be non-valid packets or empty array.
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

        /// @brief tries to deserialize a packet from the receive buffer.
        /// @return packet, could be non-valid.
        [[nodiscard]] packet receive_packet()
        {
            return tcp::get_packet_from_stream(_receive_buffer);
        }

        /// @brief tries to send a packet.
        /// tries to flush current send_buffer, and then try to send packet.
        ///     on fail (unlikely) it will add current packet to send_buffer, so all packets will eventually come in order.
        /// @param socket socket to try to send into.
        /// @param packet packet to send.
        /// @return see 'send_data'.
        send_data send(const socket& socket, const packet& packet)
        {
            const auto t = flush_send_buffer(socket);
            const bool s = _try_send(socket, packet);
            if (!_send_buffer.empty()) return {false, t};

            return {s,t + s};
        }

        /// @brief tries to send packets merged.
        /// tries to flush current send_buffer, and then try to send packets.
        ///     on fail (unlikely) it will add current packets to send_buffer, so all packets will eventually come in order.
        /// @note packets will be MERGED into 1 packet,
        /// so sending 3 packets with strings will not result in 3 packets with strings butm
        /// 1 packet with 3 strings.
        /// handy if you know your data and want to hide some extra header whitout messing with copying.
        /// @param socket socket to send on.
        /// @param packets non owning view packets.
        /// @return see `send_data`.
        send_data send_merged(
            const socket& socket,
            const std::span<packet> packets)
        {
            const auto t = flush_send_buffer(socket);
            const bool s = _try_send(socket, packets);
            if (!_send_buffer.empty()) return {false, t};

            return {s,t + s};
        }

        void buffer(
            const packet& packet)
        {
            auto stream = packet.serialize_to_stream();
            _send_buffer.push_back(
                (tcp::out_buffer){
                    stream.data(),
                    stream.size()
                });
        }

        /// @brief tries to flush send buffer.
        /// tries to flush send buffer by sending into socket.
        /// @param socket socket to try to flush into.
        /// @return how many packets did get send.
        size_t flush_send_buffer(const socket& socket)
        {
            auto total_send = 0;
            while ( !_send_buffer.empty() )
            {
                auto& buffer = _send_buffer.front();
                const auto size = buffer.size();

                const int sent = socket.send(
                    buffer.data(),
                    buffer.size());

                if (sent == size) // packet could send fully
                {
                    total_send++;
                    _send_buffer.pop_front();
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
    private:
        std::vector<uint8_t>        _receive_buffer{};
        std::deque<tcp::out_buffer> _send_buffer{};

        bool _try_send(
            const socket& socket,
            const packet& packet)
        {
            packet::header h = packet.generate_header_net();
            const auto& data = packet.get_data();
            const int sent = socket.sendv({
                {&h, sizeof(h)},
                {data.data(), data.size()}
            });

            BANKER_LIKELY if (sent == sizeof(h) + data.size()) return true;

            tcp::out_buffer buffer(sizeof(h) + data.size() - sent);

            if (sent < sizeof(h))
            {
                buffer.append(
                    reinterpret_cast<uint8_t*>(&h) + sent,
                    sizeof(h) - sent);

                buffer.append(
                    data.data(),
                    data.size());
            }
            else
            {
                buffer.append(
                    data.data() + (sent - sizeof(h)),
                    data.size() - (sent - sizeof(h)));
            }

            _send_buffer.push_back(std::move(buffer));
            return false;
        }

        bool _try_send(
             const socket& socket,
             const std::span<packet> packets)
        {
            if (packets.empty()) return true;

            const auto h = packet::generate_header_from(packets);
            const auto size = h.size;

            auto h_net = packet::header_to_net(h);

            std::vector<socket::iovec_c> io_vector(1 + packets.size());
            // header + all packets.

            io_vector[0].data = reinterpret_cast<uint8_t*>(&h_net);
            io_vector[0].len  = sizeof(h_net);

            for (size_t i = 0; i < packets.size(); ++i)
            {
                io_vector[i+1].data = packets[i].get_data().data();
                io_vector[i+1].len  = packets[i].get_data().size();
            }

            int sent = socket.sendv(
                io_vector.data(),
                io_vector.size());

            // sent = 0; // for flush debugging
            BANKER_LIKELY if (sent == sizeof(h_net) + size) return true;

            tcp::out_buffer buffer(sizeof(h_net) + size - sent);
            const auto remaining = static_cast<size_t>(sent);

            size_t offset = remaining;
            for (const auto& vec : io_vector)
            {
                if (offset >= vec.len)
                {
                    offset -= vec.len;
                    continue;
                }

                buffer.append(static_cast<const uint8_t*>(vec.data) + offset,
                              vec.len - offset);
                // only applicable for partial packets:
                // only send from offset, which is calculated to be all the previously send data,
                // and checked with the packets' length.

                offset = 0;
                // for next packets send from 0 offset, meaning the entire packet.
            }

            _send_buffer.push_back(std::move(buffer));
            return false;
        }
    };

    class packet_stream_host_core
    {
    public:
        packet_stream_host_core() = default;
        ~packet_stream_host_core() = default;
        packet_stream_host_core(const packet_stream_host_core&) = default;
        packet_stream_host_core& operator=(const packet_stream_host_core&) = default;
        packet_stream_host_core(packet_stream_host_core&&) = default;
        packet_stream_host_core& operator=(packet_stream_host_core&&) = default;

        BANKER_NODISCARD static bool create(
            socket& socket)
        {
            if (!socket.create(AF_INET, SOCK_STREAM)) return false;

            if (!socket.set_reuse_address(true)) goto packet_stream_host_core_create_error;

            if (!socket.set_blocking(false)) goto packet_stream_host_core_create_error;

            return true;

        packet_stream_host_core_create_error:
            auto _ = socket.close();
            return false;
        }

        BANKER_NODISCARD static bool bind(
            const socket& socket,
            const uint16_t port,
            const std::string& ip = "0.0.0.0")
        {
            return socket.bind(port, ip);
        }

        BANKER_NODISCARD static bool listen(
            const socket& socket,
            const int backlog = 1024)
        {
            return socket.listen(backlog);
        }

        BANKER_NODISCARD static socket accept_incoming(
            const socket& socket)
        {
            networker::socket s = socket.accept();
            if (!s.is_valid()) return s;
            if (!s.set_blocking(false)) auto _ = s.close();

            return s;
        }
    };
}

#endif //BANKER_PACKET_STREAM_CORE_HPP