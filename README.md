# Connection Marketplace

A console-based C marketplace application demonstrating graph traversal, linked lists, dynamic memory management, inventory handling, shopping carts, and recommendation queries.

The application models a small student marketplace where users are connected through a social graph. Items listed by connected users can be discovered, purchased, stored in inventory, and sold.

## 1. Project Overview

The program represents:

- Users as vertices in a graph.
- User relationships as bidirectional graph edges.
- Items for sale as linked lists owned by users.
- Marketplace inventory as dynamically allocated records.
- Recommendations as a breadth-first search through user connections.
- Shopping carts and owned inventory as fixed-size arrays managed by the application layer.

The currently logged-in user is:

```text
You (Junior)
```

The application is interactive and runs entirely in the terminal. It does not use a database or save data to files. All marketplace data exists only while the program is running.

## 2. Learning Objectives

This project demonstrates:

1. Singly linked lists for users, items, and graph adjacency lists.
2. Graph representation using adjacency lists.
3. Breadth-first search for connected-item recommendations.
4. Dynamic memory allocation and cleanup.
5. Callback functions for item listing and recommendations.
6. Modular C programming with public and internal headers.
7. Input validation and error handling.
8. Resource management through explicit destruction functions.

## 3. Project Structure

| File | Purpose |
|---|---|
| `main.c` | Program entry point |
| `application.c` | Interactive menu, wallet, cart, checkout, inventory, and user interaction |
| `application.h` | Declaration of `application_run()` |
| `graph_marketplace.c` | User graph, lookup, connections, initialization, and cleanup |
| `graph_marketplace.h` | Public marketplace structures and API |
| `marketplace_items.c` | Item listing, purchasing, and seller item listing |
| `marketplace_recommendations.c` | Breadth-first recommendation traversal |
| `marketplace_internal.h` | Internal helper declarations |
| `Makefile` | Build and cleanup commands |

## 4. Requirements

The project requires:

- A C compiler supporting C11.
- `make`.
- A terminal.

The Makefile uses:

```text
-std=c11 -Wall -Wextra -Wpedantic
```

Compatible compilers include GCC, Clang, and Apple Clang.

## 5. Building the Program

Open a terminal in the project directory:

```bash
cd /Users/sriramkarthick/Desktop/Programming/college/ds_project
```

Build the application:

```bash
make
```

This produces an executable named `marketplace`.

Remove generated files:

```bash
make clean
```

Rebuild from scratch:

```bash
make clean
make
```

## 6. Running the Program

Start the application:

```bash
./marketplace
```

The program first asks for a starting wallet balance:

```text
Welcome! You are logged in as You (Junior).
Enter your starting wallet balance:
```

Enter a non-negative value, for example:

```text
5000
```

The main menu will then appear.

## 7. Initial Marketplace Data

The application creates four users:

| ID | Name |
|---:|---|
| 1 | Alice (Senior) |
| 2 | You (Junior) |
| 3 | Carol (Senior) |
| 4 | David (Junior) |

The graph connections are:

```text
Alice (Senior) <--> You (Junior) <--> Carol (Senior) <--> David (Junior)
```

Initial listings:

| Item ID | Item | Price | Quantity | Seller |
|---:|---|---:|---:|---|
| 101 | C Programming Book | ₹850.00 | 5 | Alice |
| 102 | Desk Lamp | ₹1200.00 | 3 | Carol |
| 103 | USB Keyboard | ₹650.00 | 4 | David |
| 104 | Scientific Calculator | ₹950.00 | 2 | Alice |
| 105 | Study Table | ₹4500.00 | 2 | Carol |
| 106 | Backpack | ₹1800.00 | 6 | David |
| 107 | Arduino Starter Kit | ₹2200.00 | 3 | Alice |
| 108 | Desk Organizer | ₹400.00 | 5 | Carol |

The current user is user ID `2`.

## 8. Menu Operations

### 8.1 Browse Connected Items

Choose:

```text
1. Browse connected items
```

Enter the number of connection hops to search.

With one hop, the program displays items belonging to users directly connected to the current user. With two hops, it also searches users connected through one intermediate user.

The current user's own items are excluded. The search uses breadth-first search and visits each user only once.

### 8.2 Add Item to Cart

Choose:

```text
2. Add item to cart
```

Enter an item ID and quantity. The program rejects:

- Unknown item IDs.
- Duplicate items already in the cart.
- Zero or negative quantities.
- Quantities greater than available stock.
- Additional items after the cart reaches 100 entries.

Adding an item to the cart does not reduce stock. Stock is reduced only after successful checkout.

### 8.3 View Cart

Choose:

```text
3. View cart
```

The cart displays each item's ID, name, unit price, available stock, seller, requested quantity, line total, and overall total.

The cart can contain at most 100 distinct item entries.

### 8.4 Add Money to Wallet

Choose:

```text
4. Add money to wallet
```

Enter a non-negative amount. The wallet exists only in memory and is not connected to a real payment system.

### 8.5 Checkout

Choose:

```text
5. Checkout
```

Checkout verifies that:

1. The cart is not empty.
2. Every requested quantity is still available.
3. The total does not exceed the wallet balance.
4. The purchase fits in owned inventory.

On success, marketplace quantities are reduced, purchased items are added to inventory, the total is removed from the wallet, and the cart is emptied.

### 8.6 Show Owned Inventory

Choose:

```text
6. Show owned inventory
```

This displays items purchased during the current session. Owned inventory is separate from marketplace listings and can contain at most 100 distinct item entries.

### 8.7 Sell an Owned Item

Choose:

```text
7. Sell an owned item
```

The selling value is:

```text
sale value = original price × quantity × 0.70
```

The user receives 70% of the original price. Selling reduces owned inventory and increases the wallet balance.

Selling does not add the item back to the marketplace.

### 8.8 Show Current Balance

Choose:

```text
8. Show current balance
```

The current wallet balance is printed.

### 8.9 Exit

Choose:

```text
0. Exit
```

The program frees all dynamically allocated users, items, connections, queues, and visited arrays before terminating.

## 9. Data Structures

### User

```c
struct User {
    int id;
    char name[MARKETPLACE_NAME_MAX];
    Item *items_for_sale;
    Connection *connections;
    User *next;
};
```

Users are stored in a singly linked list.

### Item

```c
struct Item {
    int id;
    char name[MARKETPLACE_NAME_MAX];
    double price;
    size_t quantity;
    int seller_id;
    Item *next;
};
```

Each seller's items are stored as a singly linked list. Item IDs are globally unique.

### Connection

```c
struct Connection {
    User *user;
    Connection *next;
};
```

Connections form adjacency lists. Adding a connection creates one record in each user's adjacency list.

### MarketplaceGraph

```c
struct MarketplaceGraph {
    User *users;
    size_t user_count;
};
```

The graph owns all users, items, and connections.

## 10. Public API

The public API is declared in `graph_marketplace.h`.

Important functions include:

```c
void marketplace_graph_init(MarketplaceGraph *graph);
void marketplace_graph_destroy(MarketplaceGraph *graph);

bool marketplace_add_user(
    MarketplaceGraph *graph,
    int user_id,
    const char *name
);

User *marketplace_find_user(
    MarketplaceGraph *graph,
    int user_id
);

bool marketplace_add_connection(
    MarketplaceGraph *graph,
    int first_user_id,
    int second_user_id
);

bool marketplace_add_item_quantity(
    MarketplaceGraph *graph,
    int item_id,
    int seller_id,
    const char *item_name,
    double price,
    size_t quantity
);

bool marketplace_purchase_item(
    MarketplaceGraph *graph,
    int item_id,
    size_t quantity
);

bool marketplace_list_items(
    const MarketplaceGraph *graph,
    int seller_id,
    ItemCallback callback,
    void *context
);

bool marketplace_query_connected_items(
    const MarketplaceGraph *graph,
    int user_id,
    size_t max_hops,
    ItemCallback callback,
    void *context
);
```

Functions return `true` on success and `false` for invalid input, missing data, duplicate data, insufficient stock, or allocation failure.

## 11. Recommendation Algorithm

The recommendation function uses breadth-first search.

It maintains:

- A queue of users waiting to be processed.
- A depth value representing hops from the starting user.
- A visited array to avoid duplicate visits.

For every reachable user except the starting user, the callback is called for each listed item.

Conceptually:

```text
Start at current user
|
|-- Visit direct connections
|     |
|     |-- Report their items
|     |
|     |-- Visit next-level connections
|
|-- Stop after max_hops
```

`SIZE_MAX` can be used by API clients to request an unrestricted traversal.

## 12. Complexity Analysis

Let:

- `V` = number of users.
- `E` = number of connections.
- `I` = number of items.

| Operation | Complexity |
|---|---|
| User lookup | `O(V)` |
| Add item | `O(V + I)` |
| Purchase item | `O(V + I)` |
| Recommendation search | Approximately `O(V² + E + I)` |

The quadratic component of recommendation search comes from linear scans of the visited-user array.

## 13. Memory Management

Users, items, and connections are dynamically allocated.

The cleanup function is:

```c
marketplace_graph_destroy(&marketplace);
```

It frees each user's item list, connection list, and user record. Recommendation queues and visited arrays are freed before the recommendation function returns.

## 14. Suggested Demonstration

Build and run:

```bash
make clean
make
./marketplace
```

Start with:

```text
5000
```

Then:

1. Choose `1` and search one hop.
2. Choose `1` and search two hops.
3. Choose `2`, then add item `101`, quantity `1`.
4. Choose `3` to view the cart.
5. Choose `5` to checkout.
6. Choose `6` to view owned inventory.
7. Choose `7`, then sell item `101`, quantity `1`.
8. Choose `8` to view the updated balance.
9. Choose `0` to exit.

## 15. Recommended Test Cases

### Valid wallet input

Input:

```text
1000
```

Expected: the wallet starts with ₹1000.00.

### Invalid wallet input

Input:

```text
abc
```

Expected: the program requests a valid non-negative amount.

### Invalid quantity

Input:

```text
0
```

Expected: the program requests a quantity greater than zero.

### Insufficient funds

Start with `100` and attempt to purchase item `101`.

Expected: checkout is rejected because the cart total exceeds the wallet balance.

### Insufficient stock

Attempt to add more units than are available.

Expected: the program reports the available quantity.

### Duplicate cart item

Add item `101` twice.

Expected: the second addition is rejected.

### Invalid item ID

Attempt to add item `999`.

Expected:

```text
No item with ID 999 was found.
```

### Recommendation depth

Use hop counts of `1`, `2`, and `3`.

Expected: increasingly distant users become eligible for recommendation results.

### Empty inventory

Select option `6` before purchasing anything.

Expected:

```text
You do not own any items yet.
```

### Empty cart

Select option `3` before adding anything.

Expected:

```text
Your cart is empty.
```

## 16. Design Limitations

The current implementation is intentionally educational.

- Data is not persisted between executions.
- There is only one logged-in user.
- Users cannot be added through the interactive menu.
- Items cannot be listed through the interactive menu.
- Selling an item does not return it to marketplace listings.
- The cart can contain at most 100 distinct item records.
- Owned inventory can contain at most 100 distinct item records.
- Monetary values use `double`, which is suitable for demonstration but not ideal for production financial software.
- User and item lookups use linear scans.
- There is no authentication or authorization.
- There is no real payment processing.
- There is no concurrent-user support.

## 17. Academic Evaluation Criteria

A professor can evaluate the project using the following criteria.

### Correctness

- Does the program compile using the provided Makefile?
- Are invalid inputs handled?
- Are item quantities updated correctly?
- Does checkout validate stock and wallet balance?
- Does selling update inventory and wallet balance?

### Data-structure usage

- Are users stored as a linked list?
- Are seller items stored as linked lists?
- Are graph connections represented by adjacency lists?
- Is breadth-first search used for recommendations?

### Memory safety

- Are successful allocations eventually freed?
- Are allocation failures handled?
- Is double freeing avoided?
- Does graph cleanup release the complete data structure?

### Modularity

- Are public functions declared in the public header?
- Are internal helpers separated from the public API?
- Are responsibilities divided among source files?

### Input validation

- Are malformed integers rejected?
- Are negative monetary amounts rejected?
- Are zero quantities rejected?
- Are nonexistent users and items handled safely?

### Maintainability

- Are compiler warnings enabled?
- Are names descriptive?
- Are functions separated by responsibility?
- Is formatting consistent?

## 18. Optional Compilation Without Make

The equivalent direct compilation command is:

```bash
cc -std=c11 -Wall -Wextra -Wpedantic \
    main.c \
    application.c \
    graph_marketplace.c \
    marketplace_items.c \
    marketplace_recommendations.c \
    -o marketplace
```

Run the executable:

```bash
./marketplace
```

## 19. Summary

This project is an educational connection-based marketplace implemented in C.

Its main technical features are:

- Linked-list-based storage.
- A bidirectional graph using adjacency lists.
- Breadth-first search recommendations.
- Dynamic allocation and cleanup.
- Cart and inventory management.
- Wallet and checkout operations.
- Modular source organization.
- Defensive input validation.

The project can be built with `make`, executed with `./marketplace`, and evaluated through interactive demonstrations and inspection of the public marketplace API.
