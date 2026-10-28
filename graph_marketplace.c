#include "graph_marketplace.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    const User *user;
    size_t depth;
} QueueEntry;

static User *find_user(const MarketplaceGraph *graph, int user_id)
{
    User *current;

    if (graph == NULL) {
        return NULL;
    }

    current = graph->users;
    while (current != NULL) {
        if (current->id == user_id) {
            return current;
        }
        current = current->next;
    }

    return NULL;
}

static bool connection_exists(const User *user, const User *target)
{
    const Connection *connection;

    connection = user->connections;
    while (connection != NULL) {
        if (connection->user == target) {
            return true;
        }
        connection = connection->next;
    }

    return false;
}

static bool item_id_exists(const MarketplaceGraph *graph, int item_id)
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

static void free_items(Item *items)
{
    Item *next;

    while (items != NULL) {
        next = items->next;
        free(items);
        items = next;
    }
}

static void free_connections(Connection *connections)
{
    Connection *next;

    while (connections != NULL) {
        next = connections->next;
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
    User *next;

    if (graph == NULL) {
        return;
    }

    user = graph->users;
    while (user != NULL) {
        next = user->next;
        free_items(user->items_for_sale);
        free_connections(user->connections);
        free(user);
        user = next;
    }

    graph->users = NULL;
    graph->user_count = 0;
}

bool marketplace_add_user(MarketplaceGraph *graph,
                          int user_id,
                          const char *name)
{
    User *user;

    if (graph == NULL || name == NULL || find_user(graph, user_id) != NULL) {
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
    return find_user(graph, user_id);
}

bool marketplace_add_connection(MarketplaceGraph *graph,
                                int first_user_id,
                                int second_user_id)
{
    User *first;
    User *second;
    Connection *first_connection;
    Connection *second_connection;

    if (graph == NULL || first_user_id == second_user_id) {
        return false;
    }

    first = find_user(graph, first_user_id);
    second = find_user(graph, second_user_id);
    if (first == NULL || second == NULL ||
        connection_exists(first, second)) {
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

bool marketplace_add_item(MarketplaceGraph *graph,
                          int item_id,
                          int seller_id,
                          const char *item_name,
                          double price)
{
    return marketplace_add_item_quantity(graph, item_id, seller_id, item_name,
                                         price, 1);
}

bool marketplace_add_item_quantity(MarketplaceGraph *graph,
                                   int item_id,
                                   int seller_id,
                                   const char *item_name,
                                   double price,
                                   size_t quantity)
{
    User *seller;
    Item *item;

    if (graph == NULL || item_name == NULL ||
        quantity == 0 || item_id_exists(graph, item_id)) {
        return false;
    }

    seller = find_user(graph, seller_id);
    if (seller == NULL) {
        return false;
    }

    item = malloc(sizeof(*item));
    if (item == NULL) {
        return false;
    }

    item->id = item_id;
    (void)snprintf(item->name, sizeof(item->name), "%s", item_name);
    item->price = price;
    item->quantity = quantity;
    item->seller_id = seller_id;
    item->next = seller->items_for_sale;
    seller->items_for_sale = item;

    return true;
}

bool marketplace_purchase_item(MarketplaceGraph *graph,
                               int item_id,
                               size_t quantity)
{
    User *user;
    Item *item;

    if (graph == NULL || quantity == 0) {
        return false;
    }

    for (user = graph->users; user != NULL; user = user->next) {
        for (item = user->items_for_sale; item != NULL; item = item->next) {
            if (item->id == item_id) {
                if (item->quantity < quantity) {
                    return false;
                }
                item->quantity -= quantity;
                return true;
            }
        }
    }

    return false;
}

bool marketplace_list_items(const MarketplaceGraph *graph,
                            int seller_id,
                            ItemCallback callback,
                            void *context)
{
    const User *seller;
    const Item *item;

    if (graph == NULL || callback == NULL) {
        return false;
    }

    seller = find_user(graph, seller_id);
    if (seller == NULL) {
        return false;
    }

    for (item = seller->items_for_sale; item != NULL; item = item->next) {
        callback(item, context);
    }

    return true;
}

bool marketplace_query_connected_items(const MarketplaceGraph *graph,
                                       int user_id,
                                       size_t max_hops,
                                       ItemCallback callback,
                                       void *context)
{
    const User *start;
    QueueEntry *queue;
    const User **visited;
    size_t queue_capacity;
    size_t head;
    size_t tail;
    size_t visited_count;
    const User *current;
    const Connection *connection;
    const Item *item;
    size_t index;

    if (graph == NULL || callback == NULL) {
        return false;
    }

    start = find_user(graph, user_id);
    if (start == NULL) {
        return false;
    }

    if (max_hops == 0 || graph->user_count == 0) {
        return true;
    }

    queue_capacity = graph->user_count;
    queue = malloc(queue_capacity * sizeof(*queue));
    visited = malloc(queue_capacity * sizeof(*visited));
    if (queue == NULL || visited == NULL) {
        free(queue);
        free(visited);
        return false;
    }

    queue[0].user = start;
    queue[0].depth = 0;
    visited[0] = start;
    head = 0;
    tail = 1;
    visited_count = 1;

    while (head < tail) {
        current = queue[head].user;
        head++;

        if (queue[head - 1].depth > 0) {
            for (item = current->items_for_sale; item != NULL;
                 item = item->next) {
                callback(item, context);
            }
        }

        if (queue[head - 1].depth >= max_hops) {
            continue;
        }

        for (connection = current->connections;
             connection != NULL;
             connection = connection->next) {
            const User *connected_user = connection->user;
            bool already_visited = false;

            for (index = 0; index < visited_count; index++) {
                if (visited[index] == connected_user) {
                    already_visited = true;
                    break;
                }
            }

            if (!already_visited) {
                visited[visited_count++] = connected_user;
                queue[tail].user = connected_user;
                queue[tail].depth = queue[head - 1].depth + 1;
                tail++;
            }
        }
    }

    free(queue);
    free(visited);
    return true;
}
