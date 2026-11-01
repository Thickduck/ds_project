#include "marketplace_internal.h"

#include <stdio.h>
#include <stdlib.h>

User *marketplace_find_user_mutable(MarketplaceGraph *graph, int user_id)
{
    User *user;

    if (graph == NULL) {
        return NULL;
    }
    for (user = graph->users; user != NULL; user = user->next) {
        if (user->id == user_id) {
            return user;
        }
    }
    return NULL;
}

const User *marketplace_find_user_const(const MarketplaceGraph *graph,
                                        int user_id)
{
    const User *user;

    if (graph == NULL) {
        return NULL;
    }
    for (user = graph->users; user != NULL; user = user->next) {
        if (user->id == user_id) {
            return user;
        }
    }
    return NULL;
}

bool marketplace_connection_exists(const User *user, const User *target)
{
    const Connection *connection;

    for (connection = user->connections; connection != NULL;
         connection = connection->next) {
        if (connection->user == target) {
            return true;
        }
    }
    return false;
}

bool marketplace_item_id_exists(const MarketplaceGraph *graph, int item_id)
{
    const User *user;
    const Item *item;

    for (user = graph->users; user != NULL; user = user->next) {
        for (item = user->items_for_sale; item != NULL; item = item->next) {
            if (item->id == item_id) {
                return true;
            }
        }
    }
    return false;
}

void marketplace_free_items(Item *items)
{
    while (items != NULL) {
        Item *next = items->next;
        free(items);
        items = next;
    }
}

void marketplace_free_connections(Connection *connections)
{
    while (connections != NULL) {
        Connection *next = connections->next;
        free(connections);
        connections = next;
    }
}

void marketplace_graph_init(MarketplaceGraph *graph)
{
    if (graph != NULL) {
        graph->users = NULL;
        graph->user_count = 0;
    }
}

void marketplace_graph_destroy(MarketplaceGraph *graph)
{
    User *user;

    if (graph == NULL) {
        return;
    }
    user = graph->users;
    while (user != NULL) {
        User *next = user->next;
        marketplace_free_items(user->items_for_sale);
        marketplace_free_connections(user->connections);
        free(user);
        user = next;
    }
    graph->users = NULL;
    graph->user_count = 0;
}

bool marketplace_add_user(MarketplaceGraph *graph, int user_id,
                          const char *name)
{
    User *user;

    if (graph == NULL || name == NULL ||
        marketplace_find_user_mutable(graph, user_id) != NULL) {
        return false;
    }
    user = malloc(sizeof(*user));
    if (user == NULL) {
        return false;
    }
    user->id = user_id;
    (void)snprintf(user->name, sizeof(user->name), "%s", name);
    user->items_for_sale = NULL;
    user->connections = NULL;
    user->next = graph->users;
    graph->users = user;
    graph->user_count++;
    return true;
}

User *marketplace_find_user(MarketplaceGraph *graph, int user_id)
{
    return marketplace_find_user_mutable(graph, user_id);
}

bool marketplace_add_connection(MarketplaceGraph *graph, int first_user_id,
                                int second_user_id)
{
    User *first;
    User *second;
    Connection *first_connection;
    Connection *second_connection;

    if (graph == NULL || first_user_id == second_user_id) {
        return false;
    }
    first = marketplace_find_user_mutable(graph, first_user_id);
    second = marketplace_find_user_mutable(graph, second_user_id);
    if (first == NULL || second == NULL ||
        marketplace_connection_exists(first, second)) {
        return false;
    }

    first_connection = malloc(sizeof(*first_connection));
    second_connection = malloc(sizeof(*second_connection));
    if (first_connection == NULL || second_connection == NULL) {
        free(first_connection);
        free(second_connection);
        return false;
    }
    first_connection->user = second;
    first_connection->next = first->connections;
    first->connections = first_connection;
    second_connection->user = first;
    second_connection->next = second->connections;
    second->connections = second_connection;
    return true;
}
