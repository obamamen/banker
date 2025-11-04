//
// Created by moosm on 11/4/2025.
//

#ifndef BANKER_PACKET_HPP
#define BANKER_PACKET_HPP

#include <span>
#include <vector>
#include <cstdint>
#include <cstring>

namespace banker::networker
{
    class packet
    {
    public:
        /// @brief writes any copiable, string or vector to the packet.
        /// @tparam T the type to serialize.
        /// @param v the value to serialize.
        /// @return (ignore)
        template<typename T>
        std::enable_if_t<std::is_trivially_copyable_v<T>>
        write(T v) noexcept
        {
            const auto raw_v = reinterpret_cast<const uint8_t*>(&v);
            for (size_t b = 0; b < sizeof(T); b++)
            {
                _data.push_back(raw_v[b]);
            }
        }

        /// @brief clears the packet fully.
        void clear()
        {
            _data.clear();
        }

        /// @brief tries to read the value of T. will throw on packet_underflow.
        /// @details there is an internal byte offset, it wil read from there and move it forward.
        /// @details it will call read recusrivly for complex types, so the byte index will be moved none constantly.
        /// @details (there will be a compiler error if unreadable type)
        /// @tparam T the type to read. can be any copiable, std::string or std::vector. (can be combined: std::vector<std::string>)
        /// @return the value it has read.
        template<typename T>
        T read()
        {
            if constexpr (std::is_same_v<T, std::string>)
            {
                const auto len = read<uint32_t>();
                _can_read_check(len);
                std::string result(reinterpret_cast<const char*>(_data.data() + _read_offset), len);
                _read_offset += len;
                return result;
            }
            else if constexpr (is_vector<T>::value)
            {
                using elem_t = typename T::value_type;

                auto len = read<uint32_t>();

                T vec(len);
                for (uint32_t i = 0; i < len; ++i)
                    vec[i] = read<elem_t>();

                return vec;
            }
            else
            {
                static_assert(std::is_trivially_copyable_v<T>,
                              "T must be trivially copyable or std::string");

                _can_read_check(sizeof(T));
                T value;
                std::memcpy(&value, _data.data() + _read_offset, sizeof(T));
                _read_offset += sizeof(T);
                return value;
            }
        }

        /// @brief returns a view of the internal data
        /// @return const view of data
        /// @note try to use it as such:
        /// @code{.cpp}
        /// for (size_t i = 0; i < p.get_data().size(); ++i)
        /// {
        ///     auto b = p.get_data()[i];
        ///     std::cout << (int)(b) << ' ';
        /// }
        /// @endcode
        std::span<const uint8_t> get_data()
        {
            return std::span(_data);
        }

    private:
        std::vector<uint8_t> _data{};
        size_t _read_offset{};

        void _can_read_check(const size_t size) const
        {
            if ( _read_offset + size > _data.size() )
            {
                std::cerr << "packet underflow" << std::endl;
                throw std::runtime_error("packet overflow");
            }
        }

        template<typename T>
        struct is_vector : std::false_type {};

        template<typename T, typename Alloc>
        struct is_vector<std::vector<T, Alloc>> : std::true_type {};

    public:
        /// write string spec.
        /// see @ref banker::networker::packet::write for more info.
        /// @param v string to write from.
        void write(const std::string& v)
        {
            const auto len = static_cast<uint32_t>(v.size());
            this->write(len);
            _data.insert(_data.end(), v.begin(), v.end());
        }

        /// write vector<T> spec.
        /// see @ref banker::networker::packet::write for more info.
        /// @param vec vector to write from.
        template<typename T>
        void write(const std::vector<T>& vec)
        {
            const auto len = static_cast<uint32_t>(vec.size());
            write(len);

            for (const auto& elem : vec)
                write(elem);
        }
    };
}



#endif //BANKER_PACKET_HPP