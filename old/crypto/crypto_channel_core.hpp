// /* ================================== *\
//  @file     crypto_channel_core.hpp
//  @project  banker
//  @author   moosm
//  @date     13-11-2025
// *\ ================================== */
//
// #ifndef BANKER_CRYPTO_CHANNEL_CORE_HPP
// #define BANKER_CRYPTO_CHANNEL_CORE_HPP
//
// #include "banker/core/crypto/crypter.hpp"
// #include "banker/core/networker/core/tcp/packet_stream_core.hpp"
//
// namespace banker::networker
// {
//     class crypto_core
//     {
//     public:
//         crypto_core()                                       = default;
//         ~crypto_core()                                      = default;
//
//         crypto_core(const crypto_core &)                    = delete;
//         crypto_core &operator=(const crypto_core &)         = delete;
//
//         crypto_core(crypto_core &&)                         = default;
//         crypto_core &operator=(crypto_core &&)              = default;
//
//         static bool encrypt_send(
//             const socket& socket,
//             packet& packet,
//             const crypter::key& shared_key,
//             packet_stream_core& stream,
//             const crypter::nonce& nonce)
//         {
//             std::cout << "packet plaintext: " << format_bytes::to_hex(packet.get_data()) << std::endl;
//             networker::packet hmac_packet{};
//             hmac_packet.write<crypter::mac>(crypter::mac{});
//
//             networker::packet* pre[2] = { &hmac_packet, &packet };
//             packet::header header = packet::generate_header_from(pre);
//
//             const std::span<uint8_t> extra(
//                 reinterpret_cast<uint8_t *>(&header),
//                 sizeof(header));
//
//             crypter::encrypt(
//                 shared_key,
//                 packet.get_data(),
//                 extra,
//                 nonce,
//                 *reinterpret_cast<crypter::mac *>(hmac_packet.get_data().data()));
//
//             std::cout << "packet encrypted: " << format_bytes::to_hex(packet.get_data()) << std::endl;
//             std::cout << "mac: " << format_bytes::to_hex(hmac_packet.get_data()) << std::endl;
//             networker::packet merged[2] = { std::move(hmac_packet), packet };
//             stream.send_merged(socket, merged);
//
//             return true;
//         }
//
//         static bool decrypt_packet(
//             packet& packet,
//             const crypter::key& shared_key,
//             const crypter::nonce& nonce)
//         {
//             auto header = packet.generate_header();
//
//             bool valid = true;
//             const auto hmac = packet.read<crypter::mac>(&valid);
//             if (!valid) return false;
//
//             std::cout << "mac: " << format_bytes::to_hex(hmac.bytes) << std::endl;
//
//             return crypter::decrypt(
//                 shared_key,
//                 packet.get_remaining_data(),
//                 { reinterpret_cast<uint8_t *>(&header),sizeof(header) },
//                 nonce,
//                 hmac);
//         }
//     };
// }
//
// #endif //BANKER_CRYPTO_CHANNEL_CORE_HPP