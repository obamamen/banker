// /* ================================== *\
//  @file     stream_core.hpp
//  @project  banker
//  @author   moosm
//  @date     11/28/2025
// *\ ================================== */
//
// #ifndef BANKER_STREAM_CORE_HPP
// #define BANKER_STREAM_CORE_HPP
//
// #include <deque>
//
// #include "banker/core/networker/core/tcp/out_buffer.hpp"
// #include "banker/core/networker/core/tcp/stream_handler.hpp"
// #include "banker/core/networker/core/socket/socket.hpp"
//
// namespace banker::networker
// {
//     class stream_core
//     {
//     public:
//         struct write_response
//         {
//             bool sent_current_request{false};
//             size_t total_send_buffers{0};
//         };
//     public:
//
//         static socket generate_client_socket(
//             const std::string& ip,
//             const unsigned short port)
//         {
//             socket s{};
//
//             if ( !s.create(
//                 socket::domain::inet, socket::type::stream) )   return socket{};
//             if (!s.connect(ip, port) )                          return socket{};
//             if (!s.set_blocking(false) )                        return socket{};
//
//             return s;
//         }
//
//         static socket generate_server_socket(
//             const std::string& ip,
//             const unsigned short port,
//             const int backlog = 1024)
//         {
//             socket s{};
//
//             if ( !s.create(
//                 socket::domain::inet, socket::type::stream) )   return socket{};
//             if ( !s.set_reuse_address(true) )                   return socket{};
//             if ( !s.bind(port, ip) )                            return socket{};
//             if ( !s.listen(backlog) )                           return socket{};
//             if ( !s.set_blocking(false) )                       return socket{};
//
//             return s;
//         }
//
//         bool tick_receive(socket& socket)
//         {
//             return tcp::stream_socket(
//                 socket,
//                 _receive_buffer);
//         }
//
//         std::vector<uint8_t>& receive()
//         {
//             return _receive_buffer;
//         }
//
//         write_response write(
//             socket& socket,
//             uint8_t* data,
//             const size_t size)
//         {
//             const auto t = flush_send_buffer(socket);
//             const auto s = _send(socket, socket::iovec_c{data,size});
//             return {s,t+s};
//         }
//
//         write_response write_v(
//             socket& socket,
//             const std::span<socket::iovec_c> buffers)
//         {
//             const auto t = flush_send_buffer(socket);
//             const auto s = _send(socket, buffers);
//             return {s,t+s};
//         }
//
//         size_t flush_send_buffer(socket& socket)
//         {
//             size_t total_send = 0;
//
//             while ( !_send_buffer.empty() )
//             {
//                 auto& buffer = _send_buffer.front();
//                 const auto size = static_cast<int>(buffer.size());
//
//                 const int sent = socket.send(
//                     buffer.data(),
//                     buffer.size());
//
//                 if (sent == size) // buffer could send fully
//                 {
//                     total_send++;
//                     _send_buffer.pop_front();
//                     continue;
//                 }
//
//                 if (sent < 0) break; // error / or cant write.
//
//                 if (sent < size) // partial send, erase what did send
//                 {
//                     buffer.consume( static_cast<size_t>(sent) );
//                     break;
//                 }
//             }
//
//             return total_send;
//         }
//
//     private:
//         std::vector<uint8_t>        _receive_buffer{};
//         std::deque<tcp::out_buffer> _send_buffer{};
//
//         bool _send(socket& socket, const socket::iovec_c buffer)
//         {
//             const int sent = socket.send(
//                 buffer.data,
//                 buffer.len);
//
//             if (sent < 0) return false;
//             BANKER_LIKELY if (sent == static_cast<int>(buffer.len)) return true;
//
//             _send_buffer.emplace_back(
//                 static_cast<const uint8_t*>(buffer.data) + sent,
//                 buffer.len - static_cast<size_t>(sent));
//
//             return false;
//         }
//
//         bool _send(socket& socket, const std::span< socket::iovec_c > buffers)
//         {
//             if (buffers.empty()) return false;
//
//             int total_request_size = 0;
//             for (const auto& buffer : buffers)
//                 { total_request_size += buffer.len; }
//
//             const int sent = socket.sendv(
//                 buffers.data(),
//                 buffers.size());
//
//             if (sent < 0) return false;
//             BANKER_LIKELY if (sent == total_request_size) return true;
//
//             size_t remaining = static_cast<size_t>(sent);
//
//             for (const auto& buffer : buffers)
//             {
//                 if (remaining >= buffer.len)
//                 {
//                     remaining -= buffer.len;
//                     continue;
//                 }
//
//                 if (remaining > 0)
//                 {
//                     _send_buffer.emplace_back(
//                         static_cast<const uint8_t*>(buffer.data) + remaining,
//                         buffer.len - remaining);
//                     remaining = 0;
//                 }
//                 else
//                 {
//                     _send_buffer.emplace_back(
//                         static_cast<const uint8_t*>(buffer.data),
//                         buffer.len);
//                 }
//             }
//
//             return false;
//         }
//     };
// }
//
// #endif //BANKER_STREAM_CORE_HPP