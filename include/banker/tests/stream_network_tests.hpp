/* ================================== *\
 @file     stream_network_tests.hpp
 @project  banker
 @author   moosm
 @date     11/28/2025
*\ ================================== */

#ifndef BANKER_STREAM_NETWORK_TESTS_HPP
#define BANKER_STREAM_NETWORK_TESTS_HPP

#include <thread>

#include "banker/core/networker/core/packet_streaming/packet_stream_core.hpp"
#include "banker/core/networker/core/socket/socket.hpp"
#include "banker/core/networker/core/tcp/stream_core.hpp"
#include "banker/shared/program_macros.hpp"
#include "banker/tester/tester.hpp"

namespace stream_network_tests
{
    inline bool verify_string( std::vector<uint8_t>& buffer )
    {
        bool nully = false;
        for (size_t i = 0; i < buffer.size(); i++)
        {
            if (buffer[i] == 0)
            {
                nully = true;
            }
        }
        return nully;
    }
}

BANKER_TEST_CASE(stream_network_tests, hello_world,
    "Streams a simple string from server to client and vise versa.")
{
    constexpr short port = 4444;
    constexpr size_t millis_timeout = 1;

    std::string server_string = "Hello, mr. Client!";
    std::string client_string = "Hello, mr. Server!";

    banker::networker::socket server_socket = banker::networker::socket();
    BANKER_SHOULD(server_socket.create());
    BANKER_SHOULD(server_socket.set_reuse_address(true));
    BANKER_SHOULD(server_socket.set_blocking(false));
    BANKER_SHOULD(server_socket.bind(port));
    BANKER_SHOULD(server_socket.listen());

    banker::networker::socket server_client_socket = banker::networker::socket();
    banker::networker::stream_core server_stream = {};

    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    banker::networker::socket client_socket = banker::networker::socket();
    BANKER_SHOULD(client_socket.create());
    BANKER_SHOULD(client_socket.connect("127.0.0.1",port));
    BANKER_SHOULD(client_socket.set_blocking(false));

    banker::networker::stream_core client_stream = {};

    const auto start_time = std::chrono::high_resolution_clock::now();

    bool server_received = false;
    bool client_received = false;
    bool client_sent = false;

    while (!server_received || !client_received)
    {
        if (std::chrono::high_resolution_clock::now() - start_time > std::chrono::milliseconds(millis_timeout))
        {
            BANKER_FAIL("Took longer than ", millis_timeout, " ms.");
        }

        if (!server_client_socket.is_valid())
        {
            server_client_socket = server_socket.accept();
            if (server_client_socket.is_valid())
            {
                BANKER_MSG("Client connected!");
                server_stream.write(
                    server_client_socket,
                    reinterpret_cast<uint8_t*>( server_string.data() ),
                    server_string.size()+1);
            }
        }

        if (!client_sent && client_socket.is_valid())
        {
            client_stream.write(
                client_socket,
                reinterpret_cast<uint8_t*>( client_string.data() ),
                client_string.size()+1);
            client_sent = true;
        }

        server_stream.flush_send_buffer(server_client_socket);
        client_stream.flush_send_buffer(client_socket);

        if (server_client_socket.is_valid() && !server_received)
        {
            server_stream.tick_receive(server_client_socket);
            auto& server_buffer = server_stream.receive();

            if (!server_buffer.empty() && stream_network_tests::verify_string(server_buffer))
            {
                const std::string received = {
                    (char*)server_buffer.data(),
                    server_buffer.size()-1
                };
                BANKER_MSG("Server got: ", received);

                if (received != client_string)
                {
                    BANKER_FAIL("[server] Expected '", client_string, "' but got '", received, "'");
                }

                server_received = true;
            }
        }

        if (!client_received)
        {
            client_stream.tick_receive(client_socket);
            auto& client_buffer = client_stream.receive();

            if (!client_buffer.empty() && stream_network_tests::verify_string(client_buffer))
            {
                const std::string received = {
                    (char*)client_buffer.data(),
                    client_buffer.size()-1
                };
                BANKER_MSG("Client got: ", received);

                if (received != server_string)
                {
                    BANKER_FAIL("[client] Expected '", server_string, "' but got '", received, "'");
                }

                client_received = true;
            }
        }
    }
}

BANKER_TEST_CASE(packet_stream_network_tests, _1,
    "Streams a packet from server to client and vise versa.")
{
    using namespace banker;
    using namespace banker::networker;
    using networker::socket;

    constexpr short port = 4444;
    constexpr size_t millis_timeout = 1;

    std::string server_string = "Hello client!";
    std::string client_string = "Hello server!";

    std::string type_string_test = "<MSG>";

    socket server_socket = packet_stream_core::generate_server_socket("0.0.0.0", port);
    packet_stream_core client_1_packet_stream;
    socket client_1;

    socket client_socket = packet_stream_core::generate_client_socket("127.0.0.1", port);
    packet_stream_core client_packet_stream;

    const auto start_time = std::chrono::high_resolution_clock::now();

    bool client_done = false;
    bool server_done = false;

    while ( (!client_done) || (!server_done) )
    {
        if (std::chrono::high_resolution_clock::now() - start_time > std::chrono::milliseconds(millis_timeout))
        {
            BANKER_FAIL("Took longer than ", millis_timeout, " ms.");
        }

        { // server
            socket client = server_socket.accept();
            if (client.is_valid())
            {
                client_1 = std::move(client);

                BANKER_MSG("Client connected!");
                packet p;
                p.write(type_string_test);
                p.write(server_string);
                client_1_packet_stream.write_packet(client_1,p);
            }

            if ( client_1.is_valid() )
            {
                packet p = client_1_packet_stream.read_packet(client_1);
                if (p.is_valid())
                {
                    auto t = p.read<std::string>();
                    auto s = p.read<std::string>();
                    BANKER_MSG("[server] received ", t ," ", s);
                    BANKER_MSG("LOCAL: ", client_1.get_local_info().to_string());
                    BANKER_MSG("PEER:  ", client_1.get_peer_info().to_string());
                    BANKER_CHECK(s == client_string);

                    server_done = true;
                }

                client_1_packet_stream.tick(client_1);
            }
        }

        { // client
            if (client_socket.is_valid())
            {
                packet p = client_packet_stream.read_packet(client_socket);
                if (p.is_valid())
                {
                    auto t = p.read<std::string>();
                    auto s = p.read<std::string>();
                    BANKER_MSG("[client] received ", t ," ", s);

                    BANKER_MSG("LOCAL: ", client_socket.get_local_info().to_string());
                    BANKER_MSG("PEER:  ", client_socket.get_peer_info().to_string());
                    BANKER_CHECK(s == server_string);

                    client_done = true;

                    packet to_server;
                    to_server.write(type_string_test);
                    to_server.write(client_string);
                    client_packet_stream.write_packet(client_socket, to_server);
                }

                client_packet_stream.tick(client_socket);
            }
        }
    }

    BANKER_FAIL();
}

#endif //BANKER_STREAM_NETWORK_TESTS_HPP