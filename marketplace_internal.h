#ifndef MARKETPLACE_INTERNAL_H
#define MARKETPLACE_INTERNAL_H

#include "graph_marketplace.h"

User *marketplace_find_user_mutable(MarketplaceGraph *graph, int user_id);
const User *marketplace_find_user_const(const MarketplaceGraph *graph,
                                        int user_id);
bool marketplace_connection_exists(const User *user, const User *target);
bool marketplace_item_id_exists(const MarketplaceGraph *graph, int item_id);
void marketplace_free_items(Item *items);
void marketplace_free_connections(Connection *connections);

#endif
