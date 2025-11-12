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
        packet_stream_core(packet_stream_core&&) = default;
        packet_stream_core& operator=(packet_stream_core&&) = default;

        bool tick_receive(const socket& socket)
        {
            return tcp::stream_socket(socket,_receive_buffer);
        }

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

        [[nodiscard]] packet receive_packet()
        {
            return tcp::get_packet_from_stream(_receive_buffer);
        }

        send_data send(const socket& socket, const packet& packet)
        {
            const auto t = flush_send_buffer(socket);
            const bool s = try_send(socket,packet);
            if (!_send_buffer.empty()) return {false, t};

            return {s,t + s};
        }

        bool try_send(
            const socket& socket,
            const packet& packet)
        {
            packet::header h = packet.generate_header_net();
            auto size = packet.get_data().size();
            const auto& data = packet.get_data();
            const int sent = socket.sendv({
                {&h, sizeof(h)},
                {packet.get_data().data(), size}
            });

            BANKER_LIKELY if ( sent == size + sizeof(h) ) return true;
            // if the entire packet could be sent.

            tcp::out_buffer buffer(size + sizeof(h) - sent);
            // create buffer if cant send entire packet, based on how much was already sent.

            BANKER_UNLIKELY if (sent < sizeof(h)) // did the header get sent
            {
                ::memcpy(buffer.data(), reinterpret_cast<uint8_t*>(&h) + sent, sizeof(h) - sent);
                ::memcpy(buffer.data() + sizeof(h) - sent, data.data(), data.size());
                // copy only part that didn't sent of the header and copy all data [unlikely].
            }
            else
            {
                ::memcpy(buffer.data(), data.data() + (sent - sizeof(h)), data.size() - (sent - sizeof(h)));
                // copy only part that didnt sent.
            }

            _send_buffer.push_back( std::move(buffer) );

            return false;
        }

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
    };
}

#endif //BANKER_PACKET_STREAM_CORE_HPP