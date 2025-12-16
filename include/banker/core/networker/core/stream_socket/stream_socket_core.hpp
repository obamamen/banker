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

#include "banker/core/networker/core/socket/socket.hpp"
#include "banker/core/networker/core/stream_socket/stream_transmit_buffer.hpp"
#include "banker/core/networker/core/tcp/tcp_operations.hpp"

namespace banker::networker
{
    class stream_socket_core
    {
    public:
        struct receive_state
        {
            std::vector<uint8_t>                receive_buffer{};
        };

        struct send_state
        {
            std::deque<stream_transmit_buffer>  out_buffers{};
            size_t                              offset{0};
        };

        static socket new_client_socket(
            const std::string& ip,
            const uint16_t port)
        {
            socket s{};

            if ( !s.create(
                socket::domain::inet, socket::type::stream) )   return socket{};
            if (!s.connect(ip, port) )                          return socket{};
            if (!s.set_blocking(false) )                        return socket{};

            return s;
        }

        static socket new_server_socket(
            const std::string& ip,
            const unsigned short port,
            const int backlog = 1024)
        {
            socket s{};

            if ( !s.create(
                socket::domain::inet, socket::type::stream) )   return socket{};
            if ( !s.set_reuse_address(true) )                   return socket{};
            if ( !s.bind(port, ip) )                            return socket{};
            if ( !s.listen(backlog) )                           return socket{};
            if ( !s.set_blocking(false) )                       return socket{};

            return s;
        }

        static socket new_accepted_socket(
            socket& acceptor)
        {
            socket s = acceptor.accept();

            if ( !s.set_blocking(false) )   return socket{};

            return s;
        }

        static void enqueue(
            send_state& state,
            const std::vector<uint8_t>& data)
        {
            state.out_buffers.emplace_back(data);
        }

        static void enqueue(
            send_state& state,
            std::vector<uint8_t>&& data)
        {
            state.out_buffers.emplace_back(std::move(data));
        }

        template<size_t group_byte_limit = 1024 * 16>
        static size_t receive(
            socket& socket,
            receive_state& state,
            tcp::request_result* request_result = nullptr,
            const size_t byte_limit = group_byte_limit)
        {
            BANKER_SAFE(request_result) = tcp::request_result::ok;

            size_t bytes_received = 0;
            uint8_t stack_buffer[ group_byte_limit ];

            int bytes = socket.recv(stack_buffer, group_byte_limit);
            while ( bytes > 0 && bytes_received < byte_limit )
            {
                bytes_received += static_cast<size_t>(bytes);
                state.receive_buffer.insert(
                    state.receive_buffer.end(),
                    stack_buffer,
                    stack_buffer + bytes);

                bytes = socket.recv(stack_buffer, group_byte_limit);
            }

            if (bytes < 0)
            {
                if (get_last_socket_error() == socket_error_code::would_block)
                    return bytes_received;

                BANKER_SAFE(request_result) = tcp::request_result::error;
            }

            if (bytes == 0)
            {
                BANKER_SAFE(request_result) = tcp::request_result::graceful_close;
            }

            return bytes_received;
        }

        static size_t flush_out_buffer(
            socket& socket,
            send_state& state,
            tcp::request_result* request_result = nullptr)
        {
            BANKER_SAFE(request_result) = tcp::request_result::ok;

            if (state.out_buffers.empty())
                return 0;

            std::vector<socket::iovec_c> io_vecs;
            io_vecs.reserve(state.out_buffers.size());

            io_vecs.emplace_back(state.out_buffers[0].to_iovec(state.offset));
            for (size_t i = 1; i < state.out_buffers.size(); ++i)
                io_vecs.emplace_back(state.out_buffers[i].to_iovec(0));

            const int bytes = socket.sendv(io_vecs.data(), io_vecs.size());
            if (bytes < 0)
            {
                if (get_last_socket_error() == socket_error_code::would_block)
                    return 0;

                state.out_buffers.clear();
                state.offset = 0;
                BANKER_SAFE(request_result) = tcp::request_result::error;
                return 0;
            }

            if (bytes == 0)
            {
                state.out_buffers.clear();
                state.offset = 0;
                BANKER_SAFE(request_result) = tcp::request_result::graceful_close;
            }

            auto remaining = static_cast<size_t>(bytes);
            size_t buffers_sent = 0;

            while ( remaining > 0 && !state.out_buffers.empty() )
            {
                auto& buf = state.out_buffers.front();
                size_t available = buf.size(state.offset);

                const size_t consumed = std::min(available, remaining);
                state.offset += consumed;
                remaining -= consumed;

                if (state.offset >= buf.size(0))
                {
                    state.out_buffers.pop_front();
                    state.offset = 0;
                    buffers_sent++;
                }
            }

            return (buffers_sent);
        }
    };
}

#endif //BANKER_STREAM_SOCKET_CORE_HPP