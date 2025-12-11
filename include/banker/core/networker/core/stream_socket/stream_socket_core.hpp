/* ================================== *\
 @file     stream_socket_core.hpp
 @project  banker
 @author   moosm
 @date     12/11/2025
*\ ================================== */

#ifndef BANKER_STREAM_SOCKET_CORE_HPP
#define BANKER_STREAM_SOCKET_CORE_HPP

#include <cstdint>
#include <deque>
#include <vector>

#include "banker/core/networker/core/stream_socket/stream_transmit_buffer.hpp"

namespace banker::networker
{
    class stream_socket_core
    {
    public:
        class data
        {
            std::vector<uint8_t>                _receive_buffer{};
            std::deque<stream_transmit_buffer>  _out_buffers{};
        };


    };
}

#endif //BANKER_STREAM_SOCKET_CORE_HPP