#ifndef GRAPH_MARKETPLACE_H
#define GRAPH_MARKETPLACE_H

#include <stdbool.h>
#include <stddef.h>

#define MARKETPLACE_NAME_MAX 100

/* Forward declarations for the marketplace data structures. */
typedef struct User User;
typedef struct Item Item;
typedef struct Connection Connection;
typedef struct MarketplaceGraph MarketplaceGraph;

/*
 * Callback used by item-listing and recommendation functions.
 * The context pointer is supplied by the caller and may be NULL.
 */
typedef void (*ItemCallback)(const Item *item, void *context);

/*
 * An item listed for sale by a user. Quantity is the number of units currently
 * available in the marketplace listing.
 */
struct Item {
    int id;
    char name[MARKETPLACE_NAME_MAX];
    double price;
    size_t quantity;
    int seller_id;
    Item *next;
};

/*
 * An adjacency-list entry representing a connection to another user.
 */
struct Connection {
    User *user;
    Connection *next;
};

/*
 * A marketplace user and the user's items and graph connections.
 */
struct User {
    int id;
    char name[MARKETPLACE_NAME_MAX];
    Item *items_for_sale;
    Connection *connections;
    User *next;
};

/*
 * The marketplace graph.
 *
 * Users are graph vertices, and each user's connections form that vertex's
 * adjacency list. Connections are maintained bidirectionally.
 */
struct MarketplaceGraph {
    User *users;
    size_t user_count;
};

/* Initialize an empty marketplace graph. */
void marketplace_graph_init(MarketplaceGraph *graph);

/* Release all users, items, and connections owned by the graph. */
void marketplace_graph_destroy(MarketplaceGraph *graph);

/*
 * Add a user to the graph.
 *
 * Returns false if graph or name is NULL, the ID already exists, or memory
 * allocation fails.
 */
bool marketplace_add_user(MarketplaceGraph *graph,
                          int user_id,
                          const char *name);

/* Return the user with user_id, or NULL if the user does not exist. */
User *marketplace_find_user(MarketplaceGraph *graph, int user_id);

/*
 * Add a bidirectional connection between two existing users.
 *
 * Returns false for invalid users, equal IDs, duplicate connections, or
 * allocation failure.
 */
bool marketplace_add_connection(MarketplaceGraph *graph,
                                int first_user_id,
                                int second_user_id);

/*
 * List one unit of an item for sale by seller_id.
 *
 * Returns false if the seller or item name is invalid, the item ID already
 * exists, or memory allocation fails.
 */
bool marketplace_add_item(MarketplaceGraph *graph,
                          int item_id,
                          int seller_id,
                          const char *item_name,
                          double price);

/*
 * List quantity units of an item for sale by seller_id.
 *
 * Returns false if the seller, item name, or quantity is invalid, the item ID
 * already exists, or memory allocation fails.
 */
bool marketplace_add_item_quantity(MarketplaceGraph *graph,
                                   int item_id,
                                   int seller_id,
                                   const char *item_name,
                                   double price,
                                   size_t quantity);

/*
 * Remove quantity units from an item listing after a completed purchase.
 * Returns false if the item does not exist or has insufficient stock.
 */
bool marketplace_purchase_item(MarketplaceGraph *graph,
                               int item_id,
                               size_t quantity);

/*
 * Pass each item listed by seller_id to callback.
 * Returns false if graph, seller_id, or callback is invalid.
 */
bool marketplace_list_items(const MarketplaceGraph *graph,
                            int seller_id,
                            ItemCallback callback,
                            void *context);

/*
 * Pass each item sold by a reachable connected user to callback.
 *
 * Traversal starts at user_id and visits connections up to max_hops away.
 * The starting user's own items are excluded, and each item is reported only
 * once. Use SIZE_MAX for an unrestricted traversal.
 */
bool marketplace_query_connected_items(const MarketplaceGraph *graph,
                                       int user_id,
                                       size_t max_hops,
                                       ItemCallback callback,
                                       void *context);

#endif /* GRAPH_MARKETPLACE_H */
