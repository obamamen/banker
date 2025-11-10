//
// Created by moosm on 11/3/2025.
//

#ifndef BANKER_ERROR_HPP
#define BANKER_ERROR_HPP


#ifdef _WIN32
    #include <winsock2.h>
#else
    #include <cerrno>
#endif

namespace banker::networker
{
    /// @brief system nonspecific socket error code
    enum class socket_error_code
    {
        none,               // operation succeeded
        no_data,            // (would block)
        refused,            // connection refused
        reset,              // connection reset
        timeout,            // operation timed out
        host_unreachable,   // host/network unreachable
        net_down,           // network down
        addr_in_use,        // address already in use (bind() failure)
        interrupted,        // system call interrupted
        unknown             // unknown/error
    };

    /// @brief gets the last error code regarding sockets.
    /// @return collected and created socket error code.
    inline socket_error_code get_last_socket_error()
    {
#ifdef _WIN32
        switch ( WSAGetLastError() )
        {
            case 0:                 return socket_error_code::none;
            case WSAEWOULDBLOCK:    return socket_error_code::no_data;
            case WSAECONNREFUSED:   return socket_error_code::refused;
            case WSAECONNRESET:     return socket_error_code::reset;
            case WSAETIMEDOUT:      return socket_error_code::timeout;
            case WSAEHOSTUNREACH:   return socket_error_code::host_unreachable;
            case WSAENETDOWN:       return socket_error_code::net_down;
            case WSAEADDRINUSE:     return socket_error_code::addr_in_use;
            case WSAEINTR:          return socket_error_code::interrupted;
            default:                return socket_error_code::unknown;
        }
#else
        switch ( errno )
            {
            case 0:                 return socket_error_code::none;
            case EWOULDBLOCK:
#ifdef EAGAIN
            case EAGAIN:
#endif
                                    return socket_error_code::no_data;
            case ECONNREFUSED:      return socket_error_code::refused;
            case ECONNRESET:        return socket_error_code::reset;
            case ETIMEDOUT:         return socket_error_code::timeout;
            case EHOSTUNREACH:      return socket_error_code::host_unreachable;
            case ENETDOWN:          return socket_error_code::net_down;
            case EADDRINUSE:        return socket_error_code::addr_in_use;
            case EINTR:             return socket_error_code::interrupted;
            default:                return socket_error_code::unknown;
        }
#endif
    }

    /// turn a socket error code into a human-readable string.
    /// @param err the error in question.
    /// @return the const string, not advisable to change value.
    inline const char* to_string(const socket_error_code err)
    {
        switch (err)
        {
            case socket_error_code::none:            return "none";
            case socket_error_code::no_data:         return "no_data";
            case socket_error_code::refused:         return "refused";
            case socket_error_code::reset:           return "reset";
            case socket_error_code::timeout:         return "timeout";
            case socket_error_code::host_unreachable:return "host_unreachable";
            case socket_error_code::net_down:        return "net_down";
            case socket_error_code::addr_in_use:     return "addr_in_use";
            case socket_error_code::interrupted:     return "interrupted";
            case socket_error_code::unknown:         return "unknown";
        }
        return "unknown";
    }
}

#endif //BANKER_ERROR_HPP