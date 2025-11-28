/* ================================== *\
 @file     stream_network_tests.hpp
 @project  banker
 @author   moosm
 @date     11/28/2025
*\ ================================== */

#ifndef BANKER_STREAM_NETWORK_TESTS_HPP
#define BANKER_STREAM_NETWORK_TESTS_HPP

#include <thread>

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

BANKER_TEST_CASE(stream_network_tests,hello_world,
    "Streams a simple string from server to client and vise versa.")
{
    constexpr short port = 4444;
    constexpr size_t millis_timeout = 2;

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
                server_stream.send_data(
                    server_client_socket,
                    reinterpret_cast<uint8_t*>( server_string.data() ),
                    server_string.size()+1);
            }
        }

        if (!client_sent && client_socket.is_valid())
        {
            client_stream.send_data(
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

#endif //BANKER_STREAM_NETWORK_TESTS_HPP