/* ================================== *\
 @file     stream_transmit_buffer.hpp
 @project  banker
 @author   moosm
 @date     12/11/2025
*\ ================================== */

#ifndef BANKER_STREAM_TRANSMIT_BUFFER_HPP
#define BANKER_STREAM_TRANSMIT_BUFFER_HPP

#include <cstdint>
#include <memory>
#include <vector>

#include "banker/shared/compat.hpp"

namespace banker::networker
{
    class stream_transmit_buffer
    {
    public:
        stream_transmit_buffer()    = default;
        ~stream_transmit_buffer()   = default;

        stream_transmit_buffer(const stream_transmit_buffer&) = delete;
        stream_transmit_buffer& operator=(const stream_transmit_buffer&) = delete;

        stream_transmit_buffer(stream_transmit_buffer&& other) noexcept = default;
        stream_transmit_buffer& operator=(stream_transmit_buffer&& other) noexcept = default;

        explicit stream_transmit_buffer(const std::vector<uint8_t>& buffer)
        {
            _buffer = buffer;
        }

        explicit stream_transmit_buffer(std::vector<uint8_t>&& buffer) noexcept
        {
            _buffer = std::move(buffer);
        }

        explicit stream_transmit_buffer(const uint8_t* data, const size_t size) noexcept
        {
            _buffer = std::vector(data, data + size);
        }

        BANKER_NODISCARD uint8_t* data() const
        {
            return const_cast<uint8_t *>(_buffer.data() + _offset);
        }

        BANKER_NODISCARD size_t size() const
        {
            return _buffer.size() - _offset;
        }

        BANKER_NODISCARD size_t consume(const size_t bytes)
        {
            const auto available = size();

            if (bytes <= available)
            {
                _offset += bytes;
                return 0;
            }
            else
            {
                _offset += available;
                _offset = std::min(_offset, available);
                return bytes - available;
            }
        }

        BANKER_NODISCARD bool empty() const
        {
            return size() == 0;
        }

    private:
        std::vector<uint8_t> _buffer{};
        size_t _offset{0};
    };
}

#endif //BANKER_STREAM_TRANSMIT_BUFFER_HPP
