//
// Created by moosm on 11/5/2025.
//

#ifndef BANKER_PACKET_TYPES_HPP
#define BANKER_PACKET_TYPES_HPP

namespace banker::networker::base_packets
{
    enum class packet_type_to_server
    {
        unknown = 0,
        public_key = 1,
        debug_msg, // std::string
        user_defined, // through the on_recieved
    };

    enum class packet_type_from_server
    {
        unknown = 0,
        public_key = 1,
        debug_msg, // std::string
        user_defined, // ...
    };
}

#endif //BANKER_PACKET_TYPES_HPP