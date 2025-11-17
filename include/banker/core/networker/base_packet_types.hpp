/* ================================== *\
 @file     base_packet_types.hpp
 @project  banker
 @author   moosm
 @date     11/17/2025
*\ ================================== */

#ifndef BANKER_BASE_PACKET_TYPES_HPP
#define BANKER_BASE_PACKET_TYPES_HPP

#include <cstdint>

namespace banker::networker::base_packet_types
{
    enum class from_server : uint8_t
    {
        handshake_response,     // [public key]
        user_defined,           // mac + encrypted embed packet
    };

    enum class to_server : uint8_t
    {
        handshake,              // [public key]
        user_defined,           // mac + encrypted embed packet
    };
}

#endif //BANKER_BASE_PACKET_TYPES_HPP