//
// Created by moosm on 10/31/2025.
//

#ifndef BANKER_SOCKET_HPP
#define BANKER_SOCKET_HPP
#include <numeric>

#ifdef _WIN32
    #include <winsock2.h> // core socket handler
    #include <ws2tcpip.h> // inet_pton(), inet_ntop(), ipv6 support, DNS info
    typedef SOCKET socket_t;
    constexpr socket_t BANKER_INVALID_SOCKET = INVALID_SOCKET;
    constexpr int socket_error = SOCKET_ERROR;
#else
    #include <sys/socket.h>     // socket(), bind(), listen(), accept()
    #include <netinet/in.h>     // sockaddr_in, AF_INET, htons
    #include <arpa/inet.h>      // inet_pton(), inet_ntop()
    #include <unistd.h>         // close(), read(), write()
    #include <fcntl.h>          // fcntl() (non blocking)
    #include <errno.h>          // errno
    #include <sys/time.h>       // timeval
    #include <sys/types.h>      // socklen_t fd_set
    typedef int socket_t;
    constexpr socket_t BANKER_INVALID_SOCKET = ( -1 );
    constexpr int socket_error = ( -1 );
#endif

namespace banker::networker
{
    class socket
    {
    public:
        constexpr static socket_t invalid_socket = BANKER_INVALID_SOCKET;

    public:
        [[nodiscard]] bool is_initialized() const { return _socket != invalid_socket; }

        [[nodiscard]] socket_t to_fd() const noexcept { return _socket; }

        explicit socket(const socket_t socket = BANKER_INVALID_SOCKET, const int domain = AF_INET)
            noexcept : _socket(socket), _domain(domain) { _initialize_platform(); }

        ~socket() noexcept
        {
            close();
        }

        socket(const socket&) = delete;
        socket& operator=(const socket&) = delete;

        socket(socket&& other) noexcept
            : _socket(other._socket), _domain(other._domain)
        {
            other._socket = invalid_socket;
        }

        socket& operator=(socket&& other) noexcept
        {
            if (this != &other)
            {
                close();
                _socket = other._socket;
                _domain = other._domain;
                other._socket = invalid_socket;
            }
            return *this;
        }

        bool create(
            const int domain = AF_INET,
            const int type = SOCK_STREAM,
            const int protocol = 0) noexcept
        {
            _socket = ::socket(domain, type, protocol);
            _domain = domain;
            return is_initialized();
        }

        bool connect(const std::string& host, const u_short port)
        {
            if (!is_initialized())
            {
                return false;
            }

            sockaddr_in local_addr{};
            local_addr.sin_family = AF_INET;
            local_addr.sin_port = htons(port);

#ifdef _WIN32
            local_addr.sin_addr.s_addr = inet_addr(host.c_str());
            if (local_addr.sin_addr.s_addr == INADDR_NONE) { return false; }
#else
            if (inet_pton(AF_INET, host.c_str(), &local_addr.sin_addr) <= 0) { return false; }
#endif

            return ::connect(_socket, reinterpret_cast<sockaddr *>(&local_addr), sizeof(local_addr)) != socket_error;
        }

        bool bind(const u_short port, const std::string& ip = "127.0.0.1") const
        {
            sockaddr_in local_addr{};
            local_addr.sin_family = _domain;
            local_addr.sin_port = htons(port);
#ifdef _WIN32
            local_addr.sin_addr.s_addr = inet_addr(ip.c_str());
#else
            inet_pton(_domain, addr.c_str(), &local_addr.sin_addr);
#endif
            return (::bind(_socket, reinterpret_cast<sockaddr*>(&local_addr), sizeof(local_addr)) >= 0);
        }

        bool set_blocking(bool blocking = false)
        {
            if (!is_initialized()) return false;

#ifdef _WIN32
            unsigned long mode = blocking ? 0 : 1;
            return (ioctlsocket(_socket, FIONBIO, &mode) == 0);
#else
            int flags = fcntl(_socket, F_GETFL, 0);
            if (flags == -1) return false;
            flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
            return (fcntl(_socket, F_SETFL, flags) == 0);
#endif
        }

        bool listen(const int backlog = 1) const
        {
            return ::listen(_socket, backlog) >= 0;
        }

        socket accept() const
        {
            sockaddr_storage client_addr{};
            socklen_t len = sizeof(client_addr);
            const socket_t client_fd = ::accept(_socket, reinterpret_cast<sockaddr*>(&client_addr), &len);
            if (client_fd == BANKER_INVALID_SOCKET)
                return{ socket{} };

            return socket(client_fd, _domain);
        }


        int send(const void* data, const size_t len)
        {
            const int n = ::send(_socket, static_cast<const char*>(data), static_cast<int>(len), 0);
            return n;
        }

        int recv(void* buffer, const size_t len) const
        {
            const int n = ::recv(_socket, static_cast<char*>(buffer), static_cast<int>(len), 0);
            return n;
        }

        void close()
        {
            if (is_initialized()) { close_socket(_socket); }
            _socket = BANKER_INVALID_SOCKET;
        }

        bool set_reuse_address()
        {
            int opt = 1;
#ifdef _WIN32
            return setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt)) == 0;
#else
            return setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == 0;
#endif
        }

        std::string to_string() const
        {
            return { std::to_string(_socket) };
        }

    private:
        socket_t _socket{ socket::invalid_socket };
        int _domain{ AF_INET };

        static void _initialize_platform()
        {
#ifdef _WIN32
            static std::atomic<bool> wsa_initialized = false;
            if ( !wsa_initialized )
            {
                wsa_initialized = true;
                WSADATA wsa_data;
                if ( WSAStartup( MAKEWORD(2, 2), &wsa_data ) != 0 )
                {
                    wsa_initialized = false;
                }
            }
#endif
        }

        static void close_socket(socket_t s) {
#ifdef _WIN32
            closesocket(s);
#else
            close(s);
#endif
        }
    };
}

#endif //BANKER_SOCKET_HPP