//
// Created by moosm on 11/6/2025.
//

#ifndef BANKER_TESTER_HPP
#define BANKER_TESTER_HPP

#include <functional>
#include <iostream>
#include <string>
#include <algorithm>

namespace banker::tester
{
    inline thread_local std::vector<std::string> current_test_messages;

    inline void append_test_message(const std::string& msg)
    {
        current_test_messages.push_back(msg);
    }

    inline void clear_test_buffer()
    {
        current_test_messages.clear();
    }

    inline void flush_test_buffer(const std::string &prepend = "        ")
    {
        for (const auto& msg : banker::tester::current_test_messages)
        {
            std::cout << prepend << msg << "\n";
        }
    }

    struct test_case
    {
        std::string name = "<UNNAMED TEST CASE>";
        std::function<void()> test;
        std::string description = "";

        explicit test_case(
            const std::string &name,
            const std::function<void()> &test = nullptr,
            const std::string &description = "")
        {
            this->name = name;
            this->test = test;
            this->description = description;
        }
    };

    struct test_group
    {
        std::string name = "<UNNAMED TEST GROUP>";
        std::vector<test_case> tests;
    };

    struct failed_test
    {
        test_group group;
        test_case test;
        std::string error_description;

        failed_test(const test_group &group, const test_case &test, const std::string &error_description)
            : group(group),
            test(test),
            error_description(error_description)
        {

        }
    };

    inline std::vector<test_group>& test_groups()
    {
        static std::vector<test_group> registry;
        return registry;
    }

#define INTERNAL_BANKER_FUNCTION_SIGNATURE(group_name, test_name) inline void ___test_##group_name##_##test_name##___(void)

#define BANKER_TEST_CASE(group_name, test_name, ...) \
    void ___test_##group_name##_##test_name##_(); \
    struct ___reg_##group_name##_##test_name##_ \
    { \
        ___reg_##group_name##_##test_name##_() \
        { \
            auto& groups = banker::tester::test_groups(); \
            auto it = std::find_if(groups.begin(), groups.end(), [&](auto& g) \
            { \
                return g.name == #group_name; \
            }); \
            if (it == groups.end()) \
            { \
                groups.push_back({#group_name, {}}); \
                it = groups.end() - 1; \
            } \
            it->tests.push_back(banker::tester::test_case( \
            #test_name, \
            ___test_##group_name##_##test_name##_, \
            ##__VA_ARGS__ \
            )); \
        } \
    } ___instance_##group_name##_##test_name##_; \
    void ___test_##group_name##_##test_name##_()


#define BANKER_REQUIRE(cond) \
    do \
    { \
        if (!(cond)) throw std::runtime_error("Assertion failed: " #cond); \
    } \
    while(0)

#define BANKER_MSG(...) \
    do { \
        banker::tester::append_test_message(banker::common::formatting::format(__VA_ARGS__)); \
    } while(0)

#define BANKER_CHECK(cond) \
    do { \
        if (!(cond)) { \
            banker::tester::append_test_message("Check failed: " #cond); \
        } \
    } while(0)

#define BANKER_FAIL(msg) \
    do { \
        banker::tester::append_test_message(msg); \
        throw std::runtime_error(msg); \
    } while(0)

    template<typename T>
    void groups(std::ostream& stream, T group)
    {
        stream << group;
    }

    template<typename T, typename... Ts>
    void groups(std::ostream& stream, T first, Ts... rest)
    {
        stream << first;
        ((stream << ", " << rest), ...);
    }

    inline void run_test(
        std::initializer_list<std::string> requested_groups,
        bool only_failed)
    {
        const auto& all_groups = test_groups();
        int total_tests = 0;
        int passed = 0;
        int failed = 0;

        std::vector<failed_test> failed_tests;

        std::stringstream ss;
        ss << "  ";
        if (!requested_groups.size())
        {
            ss << "Running all test groups";
        }
        else
        {
            ss << "Running test groups: [";

            bool first = true;
            for (const auto& g : requested_groups)
            {
                if (!first) ss << ", ";
                ss << g;
                first = false;
            }

            ss << "]";
        }
        ss << "  ";

        banker::common::formatting::print_divider(80,'=',ss.str());
        std::cout << std::endl;

        auto should_run_group = [&](const std::string& group_name)
        {
            if (requested_groups.size() == 0)
                return true;

            return std::ranges::any_of(
                requested_groups,
                [&](const std::string& g){ return g == group_name; }
            );
        };

        for (const auto& group : all_groups)
        {
            if (!should_run_group(group.name))
                continue;

            std::cout << "[GROUP] " << group.name << "\n";

            for (const auto& test_case : group.tests)
            {
                clear_test_buffer();
                if (!only_failed) std::cout << std::endl;

                ++total_tests;

                try
                {
                    test_case.test();
                    if (!only_failed)
                    {
                        std::cout << "    [TEST] " << group.name << "::" << test_case.name;
                        if (!test_case.description.empty())
                            std::cout << " -> " << test_case.description;
                        std::cout << "\n";
                        flush_test_buffer();
                        std::cout << "    [RESULT] passed\n";
                    }
                    ++passed;
                }
                catch (const std::exception& e)
                {
                    failed_tests.push_back({group,test_case,e.what()});
                    std::cout << "!!! [TEST] " << group.name << "::" << test_case.name;
                    if (!test_case.description.empty())
                        std::cout << " -> " << test_case.description;
                    std::cout << "\n";
                    flush_test_buffer();
                    std::cout << "!!! [RESULT] failed: (" << e.what() << ") !!!\n";
                    ++failed;
                }
                catch (...)
                {
                    failed_tests.push_back({group,test_case,"???"});
                    std::cout << "!!!  [TEST] " << group.name << "::" << test_case.name;
                    if (!test_case.description.empty())
                        std::cout << " -> " << test_case.description;
                    std::cout << "\n";
                    flush_test_buffer();
                    std::cout << "!!! [RESULT] failed !!!\n";
                    ++failed;
                }
            }
        }

        std::cout << "\n\n";
        banker::common::formatting::print_divider(40, '=', "  [RESULTS]  ");
        std::cout << "[Passed: " << passed << ", Failed: " << failed << "]\n";
        banker::common::formatting::print_divider(40, '-');

        if (failed == 0)
            std::cout << "ALL TESTS PASSED\n";
        else
        {
            std::cout << "FAILED TESTS\n";
            for (auto c : failed_tests)
            {
                std::cout << "- "
                    << c.group.name << "::" << c.test.name
                    << " (" << c.error_description << ")"
                    << std::endl;
            }
        }


        banker::common::formatting::print_divider(40, '=', "");
    }

    inline void run_test(auto groups, bool only_failed = false)
    {
        run_test(groups, only_failed);
    }

    inline void run_test(const bool only_failed = false)
    {
        run_test({}, only_failed);
    }
}

#endif //BANKER_TESTER_HPP