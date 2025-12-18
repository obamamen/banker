/* ================================== *\
 @file     socket.hpp
 @project  banker
 @author   moosm
 @date     10/31/2025
*\ ================================== */

#ifndef BANKER_SOCKET_HPP
#define BANKER_SOCKET_HPP

#include <numeric>
#include <span>

#include "error.hpp"
#include "banker/debug_inspector.hpp"
#include "banker/shared/compat.hpp"

#ifdef _WIN32
    #include <winsock2.h> // core socket handler.
    #include <ws2tcpip.h> // inet_pton(), inet_ntop(), ipv6 support, DNS info.
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
    #include <sys/uio.h>        // for writev
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
        constexpr static int socket_error = BANKER_SOCKET_ERROR;

        /// @brief domain types.
        enum class domain : int
        {
            invalid = -1,

            inet = AF_INET,
            ipv4 = AF_INET,

            inet6 = AF_INET6,
            ipv6 = AF_INET6,
        };

        /// @brief socket types.
        enum class type : int
        {
            invalid = -1,

            stream = SOCK_STREAM,
        };

        /// @brief protocol types.
        enum class protocol : int
        {
            invalid = -1,

            /// @brief the OS selects based on socket::type, should be used.
            unspecified = 0,
            system_default = unspecified,

            tcp = IPPROTO_TCP,
        };

        enum class receive_result
        {
            valid,
            connection_lost,
            error,
        };

        /// @brief info about 1 way connection.
        struct connection_info
        {
            std::string ip_address{};
            uint16_t port{0};
            domain domain{domain::invalid};

            /// @brief turns any connection_info state into a string.
            /// @return string.
            [[nodiscard]] std::string to_string() const
            {
                if ( !this->is_valid() )
                {
                    return std::string{"(invalid)"};
                }

                if (domain == domain::inet6)
                    return "[" + ip_address + "]:" + std::to_string(port);

                return ip_address + ":" + std::to_string(port);
            }

            /// @brief checks if this state is valid.
            /// @return is valid.
            [[nodiscard]] bool is_valid() const
            {
                if (this->domain == domain::invalid) return false;
                if (this->ip_address.empty()) return false;

                return true;
            }
        };

        /// @brief struct use for vectorized IO, like sendv.
        struct iovec_c
        {
            /// @brief raw data pointer.
            const void* data;

            /// @brief how many bytes to include, starting from data.
            size_t len;
        };

    public:
        /// @brief if the socket is valid.
        /// @return true -> valid, false -> invalid.
        [[nodiscard]] bool is_valid() const { return _socket != invalid_socket; }

        /// @brief to file descriptor.
        /// @return returns the underlying socket_t.
        [[nodiscard]] socket_t to_fd() const noexcept { return _socket; }

        /// @brief socket ctor. can / should be used as default ctor.
        /// @param socket socket fd (defaults to invalid).
        /// @param domain which domain to use (AF_INET/ipv4 is default).
        explicit socket(const socket_t socket = BANKER_INVALID_SOCKET, const int domain = AF_INET)
            noexcept : _socket(socket), _domain(domain) { _initialize_platform(); }

        /// @brief socket dtor.
        /// @note attempts to close the socket multiple times if errors occur.
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

        /// @brief deleted copy constructor.
        socket(const socket&) = delete;

        /// @brief deleted copy assignment.
        socket& operator=(const socket&) = delete;

        /// @brief move constructor. transfers ownership of the socket.
        socket(socket&& other) noexcept
            : _socket(other._socket), _domain(other._domain)
        {
            other._socket = invalid_socket;
        }

        /// @brief move assignment operator. transfers ownership of the socket.
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

        /// @brief constructs the socket class.
        /// @param domain what domain to use.
        /// @param type which type to use.
        /// @param protocol what protocol to use.
        /// @return true -> succeeded, false -> failed.
        [[nodiscard]] bool create(
            const domain domain = domain::inet,
            const type type = type::stream,
            const protocol protocol = protocol::system_default) noexcept
        {
            _socket = ::socket(
                static_cast<int>(domain),
                static_cast<int>(type),
                static_cast<int>(protocol));

            _domain = static_cast<int>(domain);
            return is_valid();
        }

        /// @brief connects client socket to server.
        /// @param host the server hostname or IP address to connect to (e.g., "127.0.0.1").
        /// @param port the port number on the server to connect to
        /// @return true -> succeeded, false -> failed.
        [[nodiscard]] bool connect(
            const std::string& host,
            const u_short port)
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

            int connect =
                ::connect(_socket, reinterpret_cast<sockaddr *>(&local_addr), sizeof(local_addr));

            return (connect >= 0);
        }

        /// @brief binds the socket to a specific IP address and port on the local machine.
        /// @param port the local port to bind the socket to.
        /// @param ip the local IP address to bind to (default is "0.0.0.0").
        /// @return true -> succeeded, false -> failed.
        [[nodiscard]] bool bind(
            const u_short port,
            const std::string& ip = "0.0.0.0")
        {
            sockaddr_in local_addr{};
            local_addr.sin_family = static_cast<short>(_domain);
            local_addr.sin_port = htons(port);
#ifdef _WIN32
            local_addr.sin_addr.s_addr = inet_addr(ip.c_str());
#else
            inet_pton(_domain, ip.c_str(), &local_addr.sin_addr);
#endif
            return (::bind(_socket, reinterpret_cast<sockaddr*>(&local_addr), sizeof(local_addr)) >= 0);
        }

        /// @brief sets the sockets blocking flag.
        /// @param blocking if true -> blocks (socket default) if false -> non-blocking (function default).
        /// @return true -> succeeded, false -> failed.
        [[nodiscard]] bool set_blocking(bool blocking = false)
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

        /// @brief puts the socket into listening mode to accept incoming client connections.
        /// @param backlog the maximum number of pending connections the queue can hold.
        /// @return true -> succeeded, false -> failed.
        [[nodiscard]] bool listen(const int backlog = 1)
        {
            return ::listen(_socket, backlog) >= 0;
        }

        /// @brief accepts an incoming client connection.
        /// @return a new `socket` object representing the accepted client connection.
        ///         if no connection is available or an error occurs, the returned socket WILL be default initialized ( check with ::is_valid() ).
        [[nodiscard]] socket accept()
        {
            sockaddr_storage client_addr{};
            socklen_t len = sizeof(client_addr);
            const socket_t client_fd = ::accept(_socket, reinterpret_cast<sockaddr*>(&client_addr), &len);
            if (client_fd == BANKER_INVALID_SOCKET)
                return{ socket{} };

            return socket(client_fd, _domain);
        }

        /// @brief sends data through the socket to the host.
        /// @param data pointer to a byte in the buffer (should be start).
        /// @param len the number of bytes to send counted from the data pointer (should be buffer length).
        /// @return the number of bytes actually sent, if not ( 0 : disconnect, < 0 : error )
        [[nodiscard]] int send(const void* data, const size_t len)
        {
            const int n = ::send(_socket, static_cast<const char*>(data), static_cast<int>(len), 0);
            return n;
        }

        /// @brief sends multiple buffers in order to the host.
        /// @param buffers pointers to buffer pointers. (contig in memory)
        /// @param count count of valid buffers.
        /// @return the number of bytes actually sent, or a negative value if an error occurred.
        [[nodiscard]] int sendv(
            const iovec_c* buffers,
            const size_t count)
        {
            if (count == 0 || !buffers) return -1;
#ifdef _WIN32
            thread_local WSABUF main_buff[32];
            WSABUF* buf_ptr = nullptr;

            std::vector<WSABUF> heap_buff;

            if (count <= std::size(main_buff))
            {
                buf_ptr = main_buff;
            }
            else
            {
                heap_buff.resize(count);
                buf_ptr = heap_buff.data();
            }

            for (size_t i = 0; i < count; ++i)
            {
                buf_ptr[i].buf = const_cast<CHAR*>(static_cast<const CHAR*>(buffers[i].data));
                buf_ptr[i].len = static_cast<ULONG>(buffers[i].len);
            }

            DWORD sent = 0;
            int res = WSASend(_socket, buf_ptr, static_cast<DWORD>(count), &sent, 0, nullptr, nullptr);
            if (res != 0) return -1;
            return static_cast<int>(sent);
#else
            thread_local struct iovec main_buff[32];
            struct iovec* buf_ptr = nullptr;
            std::vector<iovec> heap_buff;

            if (count <= std::size(main_buff))
            {
                buf_ptr = main_buff;
            }
            else
            {
                heap_buff.resize(count);
                buf_ptr = heap_buff.data();
            }

            for (size_t i = 0; i < count; ++i)
            {
                buf_ptr[i].iov_base = buffers[i].data;
                buf_ptr[i].iov_len  = buffers[i].len;
            }

            ssize_t n = ::writev(_socket, buf_ptr, static_cast<int>(count));
            return static_cast<int>(n);
#endif
        }

        /// @brief cleaner way of using sendv, can be used with brace initializer.
        /// @tparam N amount of elements
        /// @param buffers (buffer ptr, buffer size)
        /// @return the number of bytes actually sent, or a negative value if an error occurred.
        template <size_t N>
        [[nodiscard]] int sendv(
            const std::pair<const void*, size_t> (&buffers)[N])
        {
            iovec_c c_buffers[N];
            for (size_t i = 0; i < N; i++) c_buffers[i] = { buffers[i].first, buffers[i].second };

            return sendv(c_buffers, N);
        }

        /// @brief receives data from the connected host.
        /// @param buffer pointer to the buffer where received data will be stored.
        /// @param len maximum number of bytes to receive into the buffer.
        /// @return the number of bytes actually received, 0 if the connection was closed,
        ///         or a negative value if an error occurred. (this will be the case in non-blocking mode so check for the would_block error)
        [[nodiscard]] int recv(void* buffer, const size_t len)
        {
            const int n = ::recv(_socket, static_cast<char*>(buffer), static_cast<int>(len), 0);
            return n;
        }

        /// @brief closes the socket and releases any system resources associated with it.
        /// @return the result of the underlying system call. (should almost never fail)
        ///       @code{.cpp}
        ///       if (socket.close() == socket::socket_error)
        ///       {
        ///           std::cerr << "Failed to close socket\n";
        ///       }
        ///       @endcode
        [[nodiscard]] int close()
        {
            int value = -1;
            if (is_valid()) { value = _close_socket(_socket); }
            _socket = BANKER_INVALID_SOCKET;
            return value;
        }

        /// @brief enables or disables the SO_REUSEADDR.
        /// This allows the socket to bind to a local address/port that might still be in the TIME_WAIT state,
        ///     which is especially useful for quickly restarting a server.
        /// @param enable which option to use.
        /// @return true -> succeeded, false -> failed.
        /// @note this function should be used right after socket creation.
        /// @code{.cpp}
        /// socket.create();
        /// if (!socket.set_reuse_address(true))
        /// {
        ///     std::cerr << "Failed to set SO_REUSEADDR\n";
        /// }
        /// socket.bind(8080);
        /// socket.listen();
        /// @endcode
        [[nodiscard]] bool set_reuse_address(const bool enable = true)
        {
            int opt = enable ? 1 : 0;
#ifdef _WIN32
            return setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt)) == 0;
#else
            return setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == 0;
#endif
        }

        [[nodiscard]] connection_info get_peer_info() const
        {
            if (!is_valid())
                return {};

            sockaddr_storage addr{};
            socklen_t addr_len = sizeof(addr);

            if (getpeername(_socket, reinterpret_cast<sockaddr*>(&addr), &addr_len) != 0)
                return {};

            connection_info info;
            if (!_extract_info_from_addr(addr, info)) return {};

            return info;
        }

        [[nodiscard]] connection_info get_local_info() const
        {
            if (!is_valid())
                return {};

            sockaddr_storage addr{};
            socklen_t addr_len = sizeof(addr);

            if (getsockname(_socket, reinterpret_cast<sockaddr*>(&addr), &addr_len) != 0)
                return {};

            connection_info info;
            if (!_extract_info_from_addr(addr, info)) return {};

            return info;
        }

        BANKER_NODISCARD bool is_readable(const int timeout_ms = 0) const
        {
            if (!is_valid()) return false;

            fd_set read_fds;
            FD_ZERO(&read_fds);
            FD_SET(_socket, &read_fds);

            timeval tv{};
            tv.tv_sec = timeout_ms / 1000;
            tv.tv_usec = (timeout_ms % 1000) * 1000;

            const int res = ::select(
                static_cast<int>(_socket + 1),
                &read_fds,
                nullptr,
                nullptr,
                timeout_ms >= 0 ? &tv : nullptr);

#ifdef _WIN32
            if (res == SOCKET_ERROR) return false;
#else
            if (res < 0) return false;
#endif

            return FD_ISSET(_socket, &read_fds) != 0;
        }

        BANKER_NODISCARD bool is_writable(const int timeout_ms = 0) const
        {
            fd_set write_fds;
            FD_ZERO(&write_fds);
            FD_SET(_socket, &write_fds);

            timeval tv{};
            tv.tv_sec = timeout_ms / 1000;
            tv.tv_usec = (timeout_ms % 1000) * 1000;

            const int res = ::select(
                static_cast<int>(_socket + 1),
                nullptr,
                &write_fds,
                nullptr,
                timeout_ms >= 0 ? &tv : nullptr);

#ifdef _WIN32
            if (res == SOCKET_ERROR) return false;
#else
            if (res < 0) return false;
#endif

            return FD_ISSET(_socket, &write_fds) != 0;
        }

        BANKER_NODISCARD bool has_error(const int timeout_ms = 0) const
        {
            fd_set error_fds;
            FD_ZERO(&error_fds);
            FD_SET(_socket, &error_fds);

            timeval tv{};
            tv.tv_sec = timeout_ms / 1000;
            tv.tv_usec = (timeout_ms % 1000) * 1000;

            const int res = ::select(
                static_cast<int>(_socket + 1),
                nullptr,
                nullptr,
                &error_fds,
                timeout_ms >= 0 ? &tv : nullptr);

#ifdef _WIN32
            if (res == SOCKET_ERROR) return false;
#else
            if (res < 0) return false;
#endif

            return FD_ISSET(_socket, &error_fds) == 0;
        }

    private:
        socket_t _socket{ socket::invalid_socket };
        int _domain{ AF_INET };

        static bool _extract_info_from_addr(
            const sockaddr_storage& addr,
            connection_info& info)
        {
            if (addr.ss_family == AF_INET)
            {
                const auto* addr_in = reinterpret_cast<const sockaddr_in*>(&addr);
                info.domain = socket::domain::inet;
                info.port = ntohs(addr_in->sin_port);

                char ip_str[INET_ADDRSTRLEN];
                if (!inet_ntop(AF_INET, &addr_in->sin_addr, ip_str, sizeof(ip_str)))
                    return false;

                info.ip_address = ip_str;
                return true;
            }
            else if (addr.ss_family == AF_INET6)
            {
                const auto* addr_in6 = reinterpret_cast<const sockaddr_in6*>(&addr);
                info.domain = socket::domain::inet6;
                info.port = ntohs(addr_in6->sin6_port);

                char ip_str[INET6_ADDRSTRLEN];
                if (!inet_ntop(AF_INET6, &addr_in6->sin6_addr, ip_str, sizeof(ip_str)))
                    return false;

                info.ip_address = ip_str;
                return true;
            }

            return false;
        }

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