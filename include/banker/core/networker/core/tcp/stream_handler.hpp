/* ================================== *\
 @file     stream_handler.hpp
 @project  banker
 @author   moosm
 @date     12/1/2025
*\ ================================== */

#ifndef BANKER_STREAM_HANDLER_HPP
#define BANKER_STREAM_HANDLER_HPP

#include <cassert>
#include <cstdint>
#include <vector>

#include "banker/core/networker/core/socket/socket.hpp"

namespace banker::networker::tcp
{
    /// @brief streams all bytes from the socket into buffer.
    /// @return if there is an actual error, (would block not included).
    bool inline stream_socket(
        const socket& socket,
        std::vector<uint8_t>& stream)
    {
        assert(socket.is_valid());

        uint8_t local_buffer[ 4096*4 ];
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
}

#endif //BANKER_STREAM_HANDLER_HPP