/* ================================== *\
 @file     tcp_operations.hpp
 @project  banker
 @author   moosm
 @date     12/16/2025
*\ ================================== */

#ifndef BANKER_TCP_OPERATIONS_HPP
#define BANKER_TCP_OPERATIONS_HPP

#include <cstdint>

#include "banker/core/networker/core/socket/socket.hpp"
#include "banker/core/networker/core/socket/socket.hpp"

namespace banker::networker::tcp
{
    /// @brief simple core results.
    /// can always check more in depth with: `get_last_socket_error()`
    enum class request_result : uint8_t
    {
        /// @brief no harming error.
        ok,

        /// @brief peer disconnected.
        graceful_close,

        /// @brief harming error.
        error
    };

    /// @brief requests a tcp read from the socket.
    /// @param socket a valid socket.
    /// @param buffer buffer with at least `buffer_size` of space.
    /// @param buffer_size allowed space.
    /// @param result request result , should be referenced.
    /// @return amount of bytes read.
    inline size_t request_read(
        socket& socket,
        void* buffer,
        const size_t buffer_size,
        request_result* result = nullptr)
    {
        BANKER_ASSERT( socket.is_valid() == true );

        const int r =
            socket.recv(buffer, buffer_size);

        if ( r < 0 )
        {
            BANKER_SAFE(result) = request_result::error;

            if (get_last_socket_error() == socket_error_code::would_block)
                BANKER_SAFE(result) = request_result::ok;

            return 0;
        }

        if ( r == 0)
        {
            BANKER_SAFE(result) = request_result::graceful_close;
            return 0;
        }

        BANKER_SAFE(result) = request_result::ok;
        return static_cast<size_t>(r);
    }

    /// @brief requests a tcp write from the socket.
    /// @param socket a valid socket.
    /// @param data data to write.
    /// @param bytes how many bytes to write.
    /// @param result request result , should be referenced.
    /// @return amount of bytes written.
    inline size_t request_write(
        socket& socket,
        void* data,
        const size_t bytes,
        request_result* result = nullptr)
    {
        BANKER_ASSERT( socket.is_valid() == true );

        const int r =
            socket.send(data, bytes);

        if ( r < 0 )
        {
            BANKER_SAFE(result) = request_result::error;

            if (get_last_socket_error() == socket_error_code::would_block)
                BANKER_SAFE(result) = request_result::ok;

            return 0;
        }

        if ( r == 0)
        {
            BANKER_SAFE(result) = request_result::graceful_close;
            return 0;
        }

        BANKER_SAFE(result) = request_result::ok;
        return static_cast<size_t>(r);
    }

    /// @brief requests a tcp write from the socket (vectorized).
    /// @param socket a valid socket.
    /// @param data data to write.
    /// @param count how many vectors to write.
    /// @param result request result , should be referenced.
    /// @return amount of bytes written.
    inline size_t request_write_vectorized(
        socket& socket,
        const socket::iovec_c* data,
        const size_t count,
        request_result* result = nullptr)
    {
        BANKER_ASSERT( socket.is_valid() == true );

        const int r =
            socket.sendv(data, count);

        if ( r < 0 )
        {
            BANKER_SAFE(result) = request_result::error;

            if (get_last_socket_error() == socket_error_code::would_block)
                BANKER_SAFE(result) = request_result::ok;

            return 0;
        }

        if ( r == 0)
        {
            BANKER_SAFE(result) = request_result::graceful_close;
            return 0;
        }

        BANKER_SAFE(result) = request_result::ok;
        return static_cast<size_t>(r);
   }

    /// @brief a small wrapper around `request_write_vectorized` for partial writes and ease of use.
    /// @param socket a valid socket.
    /// @param data data to write.
    /// @param count how many vectors to write.
    /// @param partial_offset byte offset into the first vector (data[0]) ,
    ///     will be modified to 0 if everything got send,
    ///     or the amount of bytes written from the last buffer that didn't get fully sent.
    /// @param result request result , should be referenced.
    /// @return amount of vector fully written.
    inline size_t request_write_vectorized_auto(
        socket& socket,
        socket::iovec_c* data,
        const size_t count,
        size_t& partial_offset,
        request_result* result = nullptr)
    {
        BANKER_ASSERT( socket.is_valid() == true );

        socket::iovec_c source_head{};
        if ( count > 0 && partial_offset != 0 )
        {
            source_head = data[0];
            data[0].data =
                static_cast<const char*>(data[0].data) + partial_offset;
            data[0].len -= partial_offset;
        }

        request_result r{};
        const size_t bytes = request_write_vectorized(
            socket,
            data,
            count,
            &r);

        if ( count > 0 && partial_offset != 0 )
        {
            data[0] = source_head;
        }

        if ( r != request_result::ok)
        {
            BANKER_SAFE(result) = r;
            return 0;
        }

        auto remaining_bytes =
            static_cast<size_t>(bytes);

        size_t buffers_sent = 0;

        while ( remaining_bytes > 0 && buffers_sent < count )
        {
            const socket::iovec_c& iovec = data[buffers_sent];
            const size_t available = iovec.len;

            if ( remaining_bytes >= available )
            {
                remaining_bytes -= available;
                buffers_sent++;
            }
            else
            {
                partial_offset = remaining_bytes;
                BANKER_SAFE(result) = request_result::ok;
                return buffers_sent;
            }
        }

        partial_offset = 0;
        BANKER_SAFE(result) = request_result::ok;
        return buffers_sent;
    }
}

#endif //BANKER_TCP_OPERATIONS_HPP