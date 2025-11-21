/* ================================== *\
 @file     robin_hash_tests.hpp
 @project  banker
 @author   moosm
 @date     11/21/2025
*\ ================================== */

#ifndef BANKER_ROBIN_HASH_TESTS_HPP
#define BANKER_ROBIN_HASH_TESTS_HPP

#include "banker/common/hash/robin_hash.hpp"
#include "banker/tester/tester.hpp"

BANKER_TEST_CASE(robin_hash, add_remove, "simply adds and removes and checks")
{
    banker::common::robin_map<uint64_t, uint64_t> rm{4};

    constexpr uint64_t to_add = 10;

    for (uint64_t i = 0; i < to_add; ++i)
    {
        rm.insert(i,i*2);
    }

    for (uint64_t i = 0; i < to_add; ++i)
    {
        uint64_t* pos = rm.find(i);
        if (pos == nullptr) BANKER_FAIL("key[",i,"] is invalid but should have data.");
        BANKER_MSG("key[",i,"] : ", *pos);
    }

    const uint64_t* pos = rm.find(to_add);
    if (pos != nullptr) BANKER_FAIL("key[",to_add,"] : ", *pos, " which can't beacuse i havent added that key.");

    const std::vector<uint64_t> large_keys =
    {
        1000000, 5000000, 11000000,129812739,192837192837,18923719283791283
    };
    BANKER_MSG("large_keys = {", large_keys, "}");

    auto m = [](const uint64_t x) -> uint64_t
    {
        const uint64_t r = x * 12038120398;
        const uint64_t r2 = (r * r) << 7;
        return r ^ r2;
    };

    for (const uint64_t k : large_keys)
    {
        const uint64_t v = m(k);
        BANKER_MSG("adding {",k,", ",v,"}");
        rm.insert(k,v);
    }

    for (const uint64_t k : large_keys)
    {
        uint64_t* gotten_v = rm.find(k);
        if (gotten_v == nullptr) BANKER_FAIL("key[",k,"] is invalid but should have data.");
        const uint64_t v = m(k);
        if (v != *gotten_v) BANKER_FAIL("value != found V from the map. should be: ", v, "; got: ", *gotten_v);

        BANKER_MSG("got {",k,", ",*gotten_v,"}");
        rm.erase(k);
        BANKER_MSG("erased {",k,"}");
    }

    for (const uint64_t k : large_keys)
    {
        uint64_t* gotten_v = rm.find(k);
        if (gotten_v != nullptr) BANKER_FAIL("key[",k,"] is valid but should be erased. value = ", *gotten_v);
    }
}

#endif //BANKER_ROBIN_HASH_TESTS_HPP