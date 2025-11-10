//
// Created by moosm on 11/10/2025.
//

#ifndef BANKER_PACKET_SERIALIZER_NET_HPP
#define BANKER_PACKET_SERIALIZER_NET_HPP

#include "banker/networker/core/packet.hpp"
#include "banker/networker/core/socket.hpp"

namespace banker::networker
{
    class tcp_packet_serializer
    {
    public:
        struct header
        {
            uint32_t size{0}; // network order
        };

        static bool tcp_send_packet(
            const socket& socket,
            const packet& packet)
        {
            auto s = packet.get_data().size();
            header header { .size = static_cast<uint32_t>(htonl(s)) };

            return socket.sendv(
            {
                {&header, sizeof(header)},
                {packet.get_data().data(), s}
            }) == static_cast<int>(sizeof(header) + s);
        }

        template<typename... PS>
        static bool tcp_send_packets_combined(
            const socket& socket,
            const PS&... packets)
        {
            constexpr size_t MAX_SLICES = sizeof...(PS) + 1;
            std::array<socket::iovec_c, MAX_SLICES> slices{};

            const size_t packet_len = (packets.get_data().size() + ... + 0);
            const size_t total_len =  packet_len + sizeof(header);

            header header {.size = static_cast<uint32_t>(htonl(packet_len))};
            slices[0] = { &header, sizeof(header) };

            size_t idx = 1;
            ((slices[idx++] = { packets.get_data().data(), packets.get_data().size() }), ...);

            const int sent = socket.sendv(slices.data(), slices.size());
            return sent == static_cast<int>(total_len);
        }
    };
}

#endif //BANKER_PACKET_SERIALIZER_NET_HPP