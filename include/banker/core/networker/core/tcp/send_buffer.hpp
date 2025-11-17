/* ================================== *\
 @file     send_buffer.hpp
 @project  banker
 @author   moosm
 @date     11/11/2025
*\ ================================== */

#ifndef BANKER_SEND_BUFFER_HPP
#define BANKER_SEND_BUFFER_HPP

#include <cstdint>
#include <vector>

namespace banker::networker::tcp
{
    class out_buffer
    {
    public:
        out_buffer() = default;
        ~out_buffer() = default;
        out_buffer(const out_buffer&) = delete;
        out_buffer& operator=(const out_buffer&) = delete;
        out_buffer(out_buffer&&) = default;
        out_buffer& operator=(out_buffer&&) = default;

        explicit out_buffer(const size_t initial_size) : _buffer(initial_size)
        {
            _offset = 0;
        }

        explicit out_buffer(const uint8_t* data, const size_t size)
        {
            _buffer.insert(_buffer.end(), data, data + size);
        }

        void append(const uint8_t* data, const size_t size)
        {
            _buffer.insert(_buffer.end(), data, data + size);
        }

        void append(const std::vector<uint8_t>& data)
        {
            _buffer.insert(_buffer.end(), data.begin(), data.end());
        }

        void append_remaining(
            uint8_t* data,
            const size_t size,
            const size_t already_sent)
        {
            append(data + already_sent, size - already_sent);
        }

        uint8_t* data() const
        {
            return const_cast<uint8_t *>(_buffer.data() + _offset);
        }

        size_t size() const
        {
            return _buffer.size() - _offset;
        }

        void consume(const size_t bytes)
        {
            _offset += bytes;
            if (_offset >= _buffer.size())
            {
                _buffer.clear();
                _offset = 0;
            }
        }

        bool empty() const
        {
            return size() == 0;
        }

    private:
        std::vector<uint8_t> _buffer{};
        size_t _offset{0};
    };
}

#endif //BANKER_SEND_BUFFER_HPP