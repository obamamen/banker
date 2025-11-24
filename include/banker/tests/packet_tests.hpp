//
// Created by moosm on 11/6/2025.
//

#ifndef BANKER_PACKET_TESTS_HPP
#define BANKER_PACKET_TESTS_HPP

#include "banker/debug_inspector.hpp"
#include "banker/core/crypto/format_bytes.hpp"
#include "banker/core/networker/core/packet/packet.hpp"
#include "banker/tester/tester.hpp"

BANKER_TEST_CASE(packets, 2_ints, "Creates a packet, puts 2 ints into it and tries getting the same 2 out.")
{
    banker::networker::packet pkt;
    int int_a = 1;
    int int_b = 1;
    BANKER_MSG("pkt size: ", pkt.get_data().size());
    BANKER_MSG("input a: ", int_a);
    BANKER_MSG("input b: ", int_b);

    BANKER_MSG("- now putting in 2 ints -");
    pkt.write(int_a);
    pkt.write(int_b);
    BANKER_MSG("pkt size: ", pkt.get_data().size());
    BANKER_MSG("packet: ", banker::format_bytes::span_to_binary(pkt.get_data(),8));
    BANKER_MSG("- now taking out 2 ints -");
    int out_a = pkt.read<int>();
    int out_b = pkt.read<int>();
    BANKER_MSG("output a: ", out_a);
    BANKER_MSG("output b: ", out_b);

    if (out_a != int_a && out_b != int_b)
    {
        BANKER_FAIL("Both are incorrect");
    }
    if (out_a != int_a)
    {
        BANKER_FAIL("The input_a is not correct");
    }
    if (out_b != int_b)
    {
        BANKER_FAIL("The input_b is not correct");
    }
}

BANKER_TEST_CASE(packets, string, "Creates a packet, tries to put a string into packed and then out.")
{
    banker::networker::packet pkt;
    std::string s = "Hello, World!";
    BANKER_MSG("pkt size: ", pkt.get_data().size());
    BANKER_MSG("input: ", s);

    BANKER_MSG("- now putting in string -");
    pkt.write(s);
    BANKER_MSG("pkt size: ", pkt.get_data().size());
    BANKER_MSG("packet: ", banker::format_bytes::to_hex_bytes(pkt.get_data().data(),pkt.get_data().size()));
    BANKER_MSG("- now taking out string -");
    auto out_a = pkt.read<std::string>();
    BANKER_MSG("output: ", out_a);

    if (s != out_a)
    {
        BANKER_FAIL("The 2 strings arent the same!");
    }
}

BANKER_TEST_CASE(packets, vector_string, "Creates a packet, tries to put multiple strings in and then out.")
{
    banker::networker::packet pkt;
    std::vector<std::string> vs = {"Hello","world","this is the 3rd"};
    BANKER_MSG("pkt size: ", pkt.get_data().size());
    BANKER_MSG("input: ");
    for (auto s : vs)
    {
        BANKER_MSG("  ", s);
    }
    BANKER_MSG("  ");
    BANKER_MSG("- now putting in vector<string> -");
    pkt.write(vs);
    BANKER_MSG("pkt size: ", pkt.get_data().size());
    BANKER_MSG("packet: ", banker::format_bytes::to_hex_bytes(pkt.get_data().data(),pkt.get_data().size()));
    BANKER_MSG("- now taking out vector<string> -");
    const auto out_a = pkt.read<std::vector<std::string>>();
    BANKER_MSG("output: ");
    for (auto s : out_a)
    {
        BANKER_MSG("  ", s);
    }

    if (vs.size() != out_a.size())
    {
        BANKER_FAIL("2 vectors arent same size");
    }
    for (int i = 0; i < vs.size(); i++)
    {
        if (vs[i] != out_a[i])
        {
            BANKER_FAIL("element[",i,"] is not correct");
        }
    }
}

BANKER_TEST_CASE(packets, packet_inception, "Creates a packet and embeds another packet into it + tries to read it again.")
{
    std::vector<uint8_t> stream{};
    {
        banker::networker::packet pkt;
        banker::networker::packet embed;

        std::string s = "Hello, World!";
        BANKER_MSG("<string>: ", s);

        embed.write( s );
        const std::vector<int> ints {1,2,3,4,5};
        BANKER_MSG("<vector<int>>: ", ints);

        embed.write(ints);
        BANKER_MSG("embed size: ", embed.get_data().size());
        pkt.write(embed);
        pkt.serialize_into_stream(stream);
    }

    {
        banker::networker::packet pkt = banker::networker::packet::deserialize(stream);
        bool valid = false;

        auto embedded = pkt.read< banker::networker::packet >(&valid);
        if (valid == false) BANKER_FAIL("Invalid packet (can't get the embed)");
        BANKER_MSG("embed size: ", embedded.get_data().size());

        std::string s = embedded.read<std::string>(&valid);
        if (valid == false) BANKER_FAIL("Invalid packet (can't get the string)");
        BANKER_MSG("<string>: ", s);

        const auto ints = embedded.read<std::vector<int>>(&valid);
        if (valid == false) BANKER_FAIL("Invalid packet (can't get the vector < int >)");
        BANKER_MSG("<vector<int>>: ", ints);
    }

}

#endif //BANKER_PACKET_TESTS_HPP