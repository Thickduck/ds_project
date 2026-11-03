#include "marketplace_internal.h"

#include <stdlib.h>

typedef struct
{
    const User *user;
    size_t depth;
} QueueEntry;

bool marketplace_query_connected_items(const MarketplaceGraph *graph,
                                       int user_id,
                                       size_t max_hops,
                                       ItemCallback callback,
                                       void *context)
{
    const User *start;
    QueueEntry *queue;
    const User **visited;
    size_t head = 0;
    size_t tail = 1;
    size_t visited_count = 1;

    if (graph == NULL || callback == NULL)
    {
        return false;
    }

    start = marketplace_find_user_const(graph, user_id);
    if (start == NULL)
    {
        return false;
    }
    if (max_hops == 0 || graph->user_count == 0)
    {
        return true;
    }

    queue = malloc(graph->user_count * sizeof(*queue));
    visited = malloc(graph->user_count * sizeof(*visited));
    if (queue == NULL || visited == NULL)
    {
        free(queue);
        free(visited);
        return false;
    }

    queue[0] = (QueueEntry){start, 0};
    visited[0] = start;

    while (head < tail)
    {
        const User *current = queue[head].user;
        size_t depth = queue[head].depth;
        const Connection *connection;
        const Item *item;
        size_t index;

        head++;
        if (depth > 0)
        {
            for (item = current->items_for_sale; item != NULL;
                 item = item->next)
            {
                callback(item, context);
            }
        }
        if (depth >= max_hops)
        {
            continue;
        }

        for (connection = current->connections; connection != NULL;
             connection = connection->next)
        {
            const User *connected = connection->user;
            bool already_visited = false;

            for (index = 0; index < visited_count; index++)
            {
                if (visited[index] == connected)
                {
                    already_visited = true;
                    break;
                }
            }
            if (!already_visited)
            {
                visited[visited_count++] = connected;
                queue[tail++] = (QueueEntry){connected, depth + 1};
            }
        }
    }

    free(queue);
    free(visited);
    return true;
}
