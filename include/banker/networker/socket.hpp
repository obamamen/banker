//
// Created by moosm on 10/31/2025.
//

#ifndef BANKER_SOCKET_HPP
#define BANKER_SOCKET_HPP

#ifdef _WIN32
    #include <winsock2.h>
    typedef SOCKET socket_t;
    constexpr socket_t invalid_socket = INVALID_SOCKET;
#else
    #include <sys/socket.h>
    typedef int socket_t;
    constexpr socket_t invalid_socket = INVALID_SOCKET;
#endif

namespace banker::networker
{
    using socket = socket_t;
}

#endif //BANKER_SOCKET_HPP