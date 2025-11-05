//
// Created by moosm on 11/5/2025.
//

#ifndef BANKER_PACKET_HANDLING_HPP
#define BANKER_PACKET_HANDLING_HPP
#include <vector>

#include "../../../../../../../../Program Files/JetBrains/CLion 2025.2.3/bin/mingw/x86_64-w64-mingw32/include/assert.h"
#include "banker/networker/core/packet.hpp"
#include "banker/networker/core/socket.hpp"

namespace banker::networker::common
{
    struct packets_with_error
    {
        std::vector<packet> packets;
        int error;
    };

    int inline send_packet_with_type(
        const packet& pkt,
        const uint8_t type,
        const socket& socket)
    {
        assert(socket.is_valid());

        packet wrapper = {};
        wrapper.write(static_cast<uint8_t>(type));
        wrapper.insert_bytes(pkt.get_data());

        const auto serialized = wrapper.serialize();
        const int sent = socket.send(serialized.data(), serialized.size());
        return sent;
    }

    [[nodiscard]] inline packets_with_error get_available_packets(
        const socket& socket,
        std::vector<uint8_t>& buffer)
    {
        if (!socket.is_valid()) return {};

        uint8_t local_buffer[4096];

        const int bytes = socket.recv(local_buffer, sizeof(local_buffer));

        if (bytes <= 0) return { {}, bytes };

        packets_with_error re = {{},bytes};
        std::vector<packet>& packets = re.packets;

        buffer.insert(buffer.end(), local_buffer, local_buffer + bytes);

        while (true)
        {
            auto pkt = packet::deserialize(buffer);

            if (pkt.get_data().empty())
                break;

            packets.push_back(pkt);
        }

        return re;
    }
}

#endif //BANKER_PACKET_HANDLING_HPP