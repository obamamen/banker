/* ================================== *\
 @file     client_manager_tests.hpp
 @project  banker
 @author   moosm
 @date     11/21/2025
*\ ================================== */

#ifndef BANKER_CLIENT_MANAGER_TESTS_HPP
#define BANKER_CLIENT_MANAGER_TESTS_HPP

#include "banker/tester/tester.hpp"
#include "banker/core/networker/core/server/client_manager.hpp"

namespace banker::tests
{
    struct add_delete_data
    {
        std::string name;

        bool operator==(const add_delete_data& rhs) const {return name == rhs.name;}
    };

    inline void banker_msg_print_add_delete_data(std::vector<add_delete_data> d)
    {
        for (size_t i = 0; i < d.size(); ++i)
        {
            BANKER_MSG(
                "    ",
                "name: ",
                d.at(i).name,
                i == (d.size()-1) ? "":" " );
        }
    }
}

BANKER_TEST_CASE(client_manager, add_delete, "Adds some clients and delets some.")
{
    using namespace banker::tests;
    constexpr size_t client_count = 10;

    std::vector<banker::networker::client_id> clients_ids;
    clients_ids.resize(client_count);

    std::vector<add_delete_data> client_data;
    client_data.resize(client_count);

    banker::networker::client_manager<add_delete_data> cm{};

    for (size_t i = 0; i < client_count; i ++)
    {
        client_data[i].name = {"client_" + std::to_string(i)};
        add_delete_data copy = client_data[i];
        clients_ids[i] = cm.add_client( std::move(copy) );
    }
    BANKER_MSG("Generated ids: ", clients_ids);
    banker_msg_print_add_delete_data(client_data);
    std::vector<banker::networker::client_id> ids_to_delete{0,5,9};
    BANKER_MSG("ids to delete", ids_to_delete);

    for (size_t i = 0; i < ids_to_delete.size(); ++i)
    {
        cm.remove_client(ids_to_delete[i]);
        BANKER_MSG("trying to delete ", ids_to_delete[i]);
    }

    for (size_t i = 0; i < client_count; i ++)
    {
        add_delete_data* copy = cm.get_client(clients_ids[i]);
        BANKER_MSG(i," : ",(copy == nullptr) ? "null":copy->name);

        if (copy != nullptr)
        {
            if (copy->name != client_data[i].name)
                BANKER_FAIL("client data dont match up! (probably didnt delete well) ",
                    copy->name, " != ", client_data[i].name);

        }

        if (copy == nullptr && std::find(ids_to_delete.begin(), ids_to_delete.end(), clients_ids[i]) == ids_to_delete.end())
        {
            BANKER_FAIL(i," Couldn't be found , it should be found.");
        }
        if (copy != nullptr && std::find(ids_to_delete.begin(), ids_to_delete.end(), clients_ids[i]) != ids_to_delete.end())
        {
            BANKER_FAIL(i," Got found but should be deleted.");
        }
    }
    BANKER_MSG();
    std::string new_d_name = "new client";
    add_delete_data new_d = {new_d_name};
    BANKER_MSG("'", new_d.name,"'"," added.");
    auto id = cm.add_client(std::move(new_d));
    BANKER_MSG("id = ", id);
    add_delete_data* f = cm.get_client(id);
    if (!f) BANKER_FAIL("Couldn't find ", new_d_name,".");
    BANKER_MSG("id: ",id," name = ",f->name);
    if (new_d_name != f->name) BANKER_FAIL("Found client hasnt the same data (name).");
}

#endif //BANKER_CLIENT_MANAGER_TESTS_HPP
