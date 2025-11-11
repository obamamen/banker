//
// Created by moosm on 11/5/2025.
//

#ifndef BANKER_STREAM_HANDLING_HPP
#define BANKER_STREAM_HANDLING_HPP
#include <vector>

#include <assert.h>

#include "banker/core/networker/core/packet.hpp"
#include "banker/core/networker/core/socket/socket.hpp"

namespace banker::networker::tcp
{
    struct packets_with_error
    {
        std::vector<packet> packets;
        int error;
    };

    int inline send_packet(
        const socket& socket,
        const packet& packet)
    {
        assert(socket.is_valid());

        const auto serialized = packet.serialize_to_stream();
        const int sent = socket.send(serialized.data(), serialized.size());
        return sent;
    }

    /// @brief tries to get the first valid packet from the stream, return a non-valid packet if failed.
    ///
    inline packet get_packet_from_stream(
        std::vector<uint8_t>& stream)
    {
        return packet::deserialize(stream);
    }

    /// @brief streams all bytes from the socket into buffer.
    /// @return if there is an actual error, (would block not included).
    bool inline stream_socket(
        const socket& socket,
        std::vector<uint8_t>& stream)
    {
        assert(socket.is_valid());

        uint8_t local_buffer[4096*4];
        int bytes = socket.recv(local_buffer, sizeof(local_buffer));
        while (bytes > 0)
        {
            stream.insert(stream.end(), local_buffer, local_buffer + bytes);
            bytes = socket.recv(local_buffer, sizeof(local_buffer));
        }

        // should only get here if there is no data (left), error or disconnect
        return bytes == -1 &&
            get_last_socket_error() == socket_error_code::would_block;
    }

    int inline send_packet_with_type(
        const packet& pkt,
        const uint8_t type,
        const socket& socket)
    {
        assert(socket.is_valid());

        packet wrapper = {};
        wrapper.write(static_cast<uint8_t>(type));
        wrapper.insert_bytes(pkt.get_data());

        const auto serialized = wrapper.serialize_to_stream();
        const int sent = socket.send(serialized.data(), serialized.size());
        return sent;
    }

    [[nodiscard]] inline packets_with_error get_available_packets(
        const socket& socket,
        std::vector<uint8_t>& stream)
    {
        if (!socket.is_valid()) return {};

        uint8_t local_buffer[4096*8];

        const int bytes = socket.recv(local_buffer, sizeof(local_buffer));

        if (bytes <= 0) return { {}, bytes };

        packets_with_error re = {{},bytes};
        std::vector<packet>& packets = re.packets;

        stream.insert(stream.end(), local_buffer, local_buffer + bytes);

        while (true)
        {
            auto pkt = packet::deserialize(stream);

            if (!pkt.is_valid())
                break;

            packets.push_back(pkt);
        }

        return re;
    }
}

#endif //BANKER_STREAM_HANDLING_HPP