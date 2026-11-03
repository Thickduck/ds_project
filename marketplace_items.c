#include "marketplace_internal.h"

#include <stdio.h>
#include <stdlib.h>

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

    if (graph == NULL || item_name == NULL || quantity == 0 ||
        marketplace_item_id_exists(graph, item_id))
    {
        return false;
    }

    seller = marketplace_find_user_mutable(graph, seller_id);
    if (seller == NULL)
    {
        return false;
    }

    item = malloc(sizeof(*item));
    if (item == NULL)
    {
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

    if (graph == NULL || quantity == 0)
    {
        return false;
    }

    for (user = graph->users; user != NULL; user = user->next)
    {
        for (item = user->items_for_sale; item != NULL; item = item->next)
        {
            if (item->id == item_id)
            {
                if (item->quantity < quantity)
                {
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

    if (graph == NULL || callback == NULL)
    {
        return false;
    }

    seller = marketplace_find_user_const(graph, seller_id);
    if (seller == NULL)
    {
        return false;
    }

    for (item = seller->items_for_sale; item != NULL; item = item->next)
    {
        callback(item, context);
    }
    return true;
}
