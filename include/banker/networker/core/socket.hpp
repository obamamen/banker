//
// Created by moosm on 10/31/2025.
//

#ifndef BANKER_SOCKET_HPP
#define BANKER_SOCKET_HPP

#include <numeric>
#include "../../debug_inspector.hpp"

#ifdef _WIN32
    #include <winsock2.h> // core socket handler
    #include <ws2tcpip.h> // inet_pton(), inet_ntop(), ipv6 support, DNS info
    typedef SOCKET socket_t;
    constexpr socket_t BANKER_INVALID_SOCKET = INVALID_SOCKET;
    constexpr int BANKER_SOCKET_ERROR = SOCKET_ERROR;
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
    constexpr int BANKER_SOCKET_ERROR = ( -1 );
#endif

namespace banker::networker
{
    class socket
    {
    public:
        constexpr static socket_t invalid_socket = BANKER_INVALID_SOCKET;
        constexpr static socket_t socket_error = BANKER_SOCKET_ERROR;

    public:
        //! @brief if the socket is valid.
        //! @return true -> valid, false -> invalid.
        [[nodiscard]] bool is_valid() const { return _socket != invalid_socket; }

        //! @brief to file descriptor.
        //! @return returns the underlying socket_t.
        [[nodiscard]] socket_t to_fd() const noexcept { return _socket; }

        //! @brief socket ctor. can / should be used as default ctor.
        //! @param socket socket fd (defaults to invalid).
        //! @param domain which domain to use (AF_INET/ipv4 is default).
        explicit socket(const socket_t socket = BANKER_INVALID_SOCKET, const int domain = AF_INET)
            noexcept : _socket(socket), _domain(domain) { _initialize_platform(); }

        //! @brief socket dtor.
        //! @note attempts to close the socket multiple times if errors occur.
        ~socket() noexcept
        {
            // when socket.accept is called with no backlog it will return default ctor socket.
            // this will cause closing of invalid socket.
            if (is_valid() == false) return;


            for (int trie = 0; trie < 5; trie++)
            {
                if (close() == socket::socket_error)
                {
                    std::cerr << "[socket] Warning: failed to close socket on socket dtor" +
                        INSPECT(INSPECT_V(_socket),INSPECT_V(_domain),INSPECT_V(trie))
                    << std::endl;
                }
                else { break; }
            }
        }

        //! @brief deleted copy constructor.
        socket(const socket&) = delete;

        //! @brief deleted copy assignment.
        socket& operator=(const socket&) = delete;

        //! @brief move constructor. transfers ownership of the socket.
        socket(socket&& other) noexcept
            : _socket(other._socket), _domain(other._domain)
        {
            other._socket = invalid_socket;
        }

        //! @brief move assignment operator. transfers ownership of the socket.
        socket& operator=(socket&& other) noexcept
        {
            if (this != &other)
            {
                _socket = other._socket;
                _domain = other._domain;
                other._socket = invalid_socket;
            }
            return *this;
        }

        //! @brief constructs the socket class.
        //! @param domain which domain to use (AF_INET/ipv4 is default).
        //! @param type which type to use (SOCK_STREAM for TCP, SOCK_DGRAM for UDP)
        //! @param protocol what protocol to use (IPPROTO_TCP, IPPROTO_UDP)
        //!     Pass 0 to use the default protocol for the given type.
        //! @return true -> succeeded, false -> failed.
        [[nodiscard]] bool create(
            const int domain = AF_INET,
            const int type = SOCK_STREAM,
            const int protocol = 0) noexcept
        {
            _socket = ::socket(domain, type, protocol);
            _domain = domain;
            return is_valid();
        }

        //! @brief connects client socket to server.
        //! @param host the server hostname or IP address to connect to (e.g., "127.0.0.1").
        //! @param port the port number on the server to connect to
        //! @return true -> succeeded, false -> failed.
        [[nodiscard]] bool connect(const std::string& host, const u_short port) const
        {
            if (!is_valid())
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

        //! @brief binds the socket to a specific IP address and port on the local machine.
        //! @param port the local port to bind the socket to.
        //! @param ip the local IP address to bind to (default is "127.0.0.1").
        //! @return true -> succeeded, false -> failed.
        [[nodiscard]] bool bind(const u_short port, const std::string& ip = "127.0.0.1") const
        {
            sockaddr_in local_addr{};
            local_addr.sin_family = _domain;
            local_addr.sin_port = htons(port);
#ifdef _WIN32
            local_addr.sin_addr.s_addr = inet_addr(ip.c_str());
#else
            inet_pton(_domain, ip.c_str(), &local_addr.sin_addr);
#endif
            return (::bind(_socket, reinterpret_cast<sockaddr*>(&local_addr), sizeof(local_addr)) >= 0);
        }

        //! @brief sets the sockets blocking flag.
        //! @param blocking if true -> blocks (socket default) if false -> non-blocking (function default).
        //! @return true -> succeeded, false -> failed.
        [[nodiscard]] bool set_blocking(bool blocking = false) const
        {
            if (!is_valid()) return false;

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

        //! @brief puts the socket into listening mode to accept incoming client connections.
        //! @param backlog the maximum number of pending connections the queue can hold.
        //! @return true -> succeeded, false -> failed.
        [[nodiscard]] bool listen(const int backlog = 1) const
        {
            return ::listen(_socket, backlog) >= 0;
        }

        //! @brief accepts an incoming client connection.
        //! @return a new `socket` object representing the accepted client connection.
        //!         if no connection is available or an error occurs, the returned socket WILL be uninitialized.
        [[nodiscard]] socket accept() const
        {
            sockaddr_storage client_addr{};
            socklen_t len = sizeof(client_addr);
            const socket_t client_fd = ::accept(_socket, reinterpret_cast<sockaddr*>(&client_addr), &len);
            if (client_fd == BANKER_INVALID_SOCKET)
                return{ socket{} };

            return socket(client_fd, _domain);
        }

        //! @brief sends data through the socket to the host.
        //! @param data pointer to a byte in the buffer (should be start).
        //! @param len the number of bytes to send counted from the data pointer (should be buffer length).
        //! @return the number of bytes actually sent, or a negative value if an error occurred.
        [[nodiscard]] int send(const void* data, const size_t len) const
        {
            const int n = ::send(_socket, static_cast<const char*>(data), static_cast<int>(len), 0);
            return n;
        }

        //! @brief receives data from the connected host.
        //! @param buffer pointer to the buffer where received data will be stored.
        //! @param len maximum number of bytes to receive into the buffer.
        //! @return the number of bytes actually received, 0 if the connection was closed,
        //!         or a negative value if an error occurred.
        [[nodiscard]] int recv(void* buffer, const size_t len) const
        {
            const int n = ::recv(_socket, static_cast<char*>(buffer), static_cast<int>(len), 0);
            return n;
        }

        //! @brief closes the socket and releases any system resources associated with it.
        //! @return the result of the underlying system call. (should almost never fail)
        //!       @code
        //!       if (socket.close() == socket::socket_error)
        //!       {
        //!           std::cerr << "Failed to close socket\n";
        //!       }
        //!       @endcode
        [[nodiscard]] int close()
        {
            int value = -1;
            if (is_valid()) { value = _close_socket(_socket); }
            _socket = BANKER_INVALID_SOCKET;
            return value;
        }

        //! @brief enables or disables the SO_REUSEADDR.
        //! This allows the socket to bind to a local address/port that might still be in the TIME_WAIT state,
        //!     which is especially useful for quickly restarting a server.
        //! @param enable which option to use.
        //! @return true -> succeeded, false -> failed.
        //! @note this function should be used right after socket creation.
        //! @code
        //! socket.create();
        //! if (!socket.set_reuse_address(true))
        //! {
        //!     std::cerr << "Failed to set SO_REUSEADDR\n";
        //! }
        //! socket.bind(8080);
        //! socket.listen();
        //! @endcode
        [[nodiscard]] bool set_reuse_address(const bool enable = true) const
        {
            int opt = enable ? 1 : 0;
#ifdef _WIN32
            return setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt)) == 0;
#else
            return setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == 0;
#endif
        }

        //! @brief gets the string based representation of the internal socket. (don't try constructing `socket_t` from this)
        //! @return a newly created string that represents the internal socket.
        [[nodiscard]] std::string to_string() const
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

        static int _close_socket(socket_t s)
        {
#ifdef _WIN32
            return closesocket(s);
#else
            return ::close(s);
#endif
        }
    };
}

#endif //BANKER_SOCKET_HPP