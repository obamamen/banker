// /* ================================== *\
//  @file     packet_stream.hpp
//  @project  banker
//  @author   moosm
//  @date     11/10/2025
// *\ ================================== */
//
// #ifndef BANKER_STREAM_SOCKET_HPP
// #define BANKER_STREAM_SOCKET_HPP
//
// #include <cstdint>
// #include <deque>
// #include <vector>
//
// #include "banker/core/networker/core/socket/socket.hpp"
//
// #include "banker/core/networker/core/packet_streaming/packet_stream_core.hpp"
// #include "banker/core/networker/core/packet_streaming/packet_stream_handler.hpp"
//
// #include "banker/core/networker/core/packet/packet.hpp"
//
// namespace banker::networker
// {
//     class packet_stream
//     {
//         friend class packet_stream_host;
//     private:
//         explicit packet_stream(socket&& socket) : _socket(std::move(socket)) {}
//     public:
//         packet_stream() = default;
//         packet_stream(const packet_stream&) = delete;
//         packet_stream& operator=(const packet_stream&) = delete;
//         packet_stream(packet_stream&& socket) noexcept : _socket(std::move(socket._socket)) {}
//         packet_stream& operator=(packet_stream&& socket) noexcept
//             { _socket = std::move(socket._socket); return *this; }
//
//         /// tries to connect.
//         /// @param port port.
//         /// @param host IP address.
//         /// @return returns if connection was successful.
//         bool connect(
//             const u_short port,
//             const std::string& host = "0.0.0.0")
//         {
//             auto c = _socket.create();
//             if (c == false) return false;
//             return _socket.connect(host, port);
//         }
//
//
//         /// @brief tries to send packet.
//         /// it will first try to flush its queued packages, and then the current packet.
//         /// @param packet packet to try to send, might get added to internal buffer if it couldn't send.
//         /// @return see `packet_stream_core::send_data`.
//         [[nodiscard]] packet_stream_core::send_data send_packet(const packet& packet)
//         {
//             return core.send(_socket, packet);
//         }
//
//         /// @brief tries to send packets merged.
//         /// @note merge meaning it combines the packet.data() into 1 sendv and merges the packet::header 's.
//         /// @param packets non owning view packets.
//         /// @return see `packet_stream_core::send_data`.
//         [[nodiscard]] packet_stream_core::send_data send_packets_merged(
//             const std::span<packet> packets)
//         {
//             return core.send_merged(_socket, packets);
//         }
//
//         /// @brief flushes the internal out buffer.
//         /// call this if you don't have any packets to send, maybe in a tick loop.
//         /// @return see `send_data`.
//         [[nodiscard]] packet_stream_core::send_data flush_send_buffer()
//         {
//             return {false,core.flush_send_buffer(_socket) };
//         }
//
//         /// @brief checks to see if underlying socket is valid.
//         /// @return
//         [[nodiscard]] bool is_valid() const
//         {
//             return _socket.is_valid();
//         }
//
//         /// @brief updates the receive buffer with incoming the TCP stream.
//         void tick()
//         {
//             core.tick_receive(_socket);
//         }
//
//         /// @brief returns all the packets currently in the buffer.
//         /// @note call 'tick()' before calling this function.
//         /// @return vector of deserialized packets.
//         [[nodiscard]] std::vector<packet> receive_packets()
//         {
//             return core.receive_packets();
//         }
//
//         /// @brief returns the oldest packet currently in the buffer.
//         /// @note call @ref tick() "tick()" before calling this function.
//         /// @return single packet, check for validness.
//         [[nodiscard]] packet receive_packet()
//         {
//             return core.receive_packet();
//         }
//
//     private:
//         socket _socket{};
//         packet_stream_core core{};
//     };
//
//     class packet_stream_host
//     {
//     public:
//         packet_stream_host() = default;
//         ~packet_stream_host() = default;
//         packet_stream_host(const packet_stream_host&) = delete;
//         packet_stream_host& operator=(const packet_stream_host&) = delete;
//         packet_stream_host(packet_stream_host&&) = default;
//         packet_stream_host& operator=(packet_stream_host&&) = default;
//
//         /// creates, binds, and listens on the socket.
//         /// @param port
//         /// @param ip
//         /// @param backlog
//         /// @return
//         BANKER_NODISCARD bool create(
//             const uint16_t port,
//             const std::string& ip = "0.0.0.0",
//             const int backlog = 1024)
//         {
//             const bool created = packet_stream_host_core::create(
//                 _socket);
//
//             if (created == false) return false;
//
//             const bool bound = packet_stream_host_core::bind(
//                 _socket,
//                 port,
//                 ip);
//
//             if (bound == false) return false;
//
//             const bool listening = packet_stream_host_core::listen(
//                 _socket,
//                 backlog);
//
//             if (listening == false) return false;
//
//             return true;
//         }
//
//         BANKER_NODISCARD bool relisten(
//             const int backlog = 1024) const
//         {
//             return packet_stream_host_core::listen(
//                 _socket,
//                 backlog);
//         }
//
//         /// @brief calls .accept , and convert it into a packet_stream.
//         /// @return the stream socket, needs to be checked for validness.
//         BANKER_NODISCARD packet_stream accept() const
//         {
//             auto socket = packet_stream_host_core::accept_incoming(_socket);
//             return packet_stream{ std::move(socket) };
//         }
//     private:
//         socket _socket{};
//     };
// }
//
// #endif //BANKER_STREAM_SOCKET_HPP