//
// Created by moosm on 11/5/2025.
//

#ifndef BANKER_PACKET_TYPES_HPP
#define BANKER_PACKET_TYPES_HPP

#include <cstdint>
#include <string_view>

namespace banker::networker::base_packets
{
    /// @brief internal pack type, from client to server, used for abstract encryption and ...

    // type notation means what type the argument is, could be merged with ().
    // () notation means the size of the given argument. count_in_bytes(4b int), msg(...b string)
    // [] notation means the argument list.
    enum class packet_type_to_server : uint8_t
    {
        unknown = 0,                        // []

        user_defined = 1,                   // encrypted [ ...(...b) ]

        request_public_key,                 // plaintext []
        requested_public_key,               // plaintext [ public_key(key) ]

        request_nonce,                      // encrypted [ nonce_size_bytes(4b, int) ]
    };

    /// @brief internal pack type, from server to client, used for abstract encryption and ...
    ///
    enum class packet_type_from_server : uint8_t
    {
        unknown = 0,

        user_defined = 1,                   // encrypted [ (...b) ]

        request_public_key,                 // plaintext [ (0b)]
        requested_public_key,               // plaintext [ (sizeof key) ]

        requested_nonce,
    };

    inline std::string_view packet_type_from_server_to_string(
        const packet_type_from_server pt)
    {
        switch (pt)
        {
            case packet_type_from_server::unknown:               return "unknown";
            case packet_type_from_server::user_defined:          return "user_defined";
            case packet_type_from_server::request_public_key:    return "request_public_key";
            case packet_type_from_server::requested_public_key:  return "requested_public_key";
            case packet_type_from_server::requested_nonce:       return "requested_nonce";
            default:                                             return "<invalid>";
        }
    }

    inline std::string_view packet_type_to_server_to_string(
        const packet_type_to_server pt)
    {
        switch (pt)
        {
            case packet_type_to_server::unknown:               return "unknown";
            case packet_type_to_server::user_defined:          return "user_defined";
            case packet_type_to_server::request_public_key:    return "request_public_key";
            case packet_type_to_server::requested_public_key:  return "requested_public_key";
            case packet_type_to_server::request_nonce:         return "request_nonce";
            default:                                           return "<invalid>";
        }
    }
}

#endif //BANKER_PACKET_TYPES_HPP