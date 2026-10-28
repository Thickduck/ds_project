#include "graph_marketplace.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CART_ITEMS 100
#define MAX_INVENTORY_ITEMS 100

typedef struct {
    const Item *items[MAX_CART_ITEMS];
    size_t quantities[MAX_CART_ITEMS];
    size_t count;
} Cart;

typedef struct {
    Item items[MAX_INVENTORY_ITEMS];
    size_t count;
} Inventory;

static const User *find_seller(const MarketplaceGraph *marketplace,
                               int seller_id)
{
    const User *user;

    for (user = marketplace->users; user != NULL; user = user->next) {
        if (user->id == seller_id) {
            return user;
        }
    }

    return NULL;
}

static void print_item(const Item *item, void *context)
{
    const MarketplaceGraph *marketplace = context;
    const User *seller = find_seller(marketplace, item->seller_id);
    const char *seller_name = seller != NULL ? seller->name : "Unknown";

    printf("  #%d %-22s ₹%.2f | available: %zu | seller: %s\n",
           item->id, item->name, item->price, item->quantity, seller_name);
}

static void print_section(const char *title)
{
    printf("\n%s\n", title);
    printf("----------------------------------------\n");
}

static bool read_line(const char *prompt, char *buffer, size_t buffer_size)
{
    printf("%s", prompt);
    fflush(stdout);

    if (fgets(buffer, (int)buffer_size, stdin) == NULL) {
        return false;
    }

    buffer[strcspn(buffer, "\n")] = '\0';
    return true;
}

static bool read_int(const char *prompt, int *value)
{
    char input[64];
    char extra;

    while (read_line(prompt, input, sizeof(input))) {
        if (sscanf(input, " %d %c", value, &extra) == 1) {
            return true;
        }
        printf("Please enter a valid whole number.\n");
    }

    return false;
}

static bool read_quantity(const char *prompt, size_t *quantity)
{
    int value;

    while (read_int(prompt, &value)) {
        if (value > 0) {
            *quantity = (size_t)value;
            return true;
        }
        printf("Please enter a quantity greater than zero.\n");
    }

    return false;
}

static bool read_money(const char *prompt, double *value)
{
    char input[64];
    char extra;
    char *end;

    while (read_line(prompt, input, sizeof(input))) {
        errno = 0;
        *value = strtod(input, &end);
        while (*end == ' ' || *end == '\t') {
            end++;
        }
        extra = *end;
        if (end != input && extra == '\0' && errno == 0 && *value >= 0.0) {
            return true;
        }
        printf("Please enter a valid non-negative amount.\n");
    }

    return false;
}

static const Item *find_item(const MarketplaceGraph *marketplace, int item_id)
{
    const User *user;
    const Item *item;

    for (user = marketplace->users; user != NULL; user = user->next) {
        for (item = user->items_for_sale; item != NULL; item = item->next) {
            if (item->id == item_id) {
                return item;
            }
        }
    }

    return NULL;
}

static bool cart_contains(const Cart *cart, const Item *item)
{
    size_t index;

    for (index = 0; index < cart->count; index++) {
        if (cart->items[index] == item) {
            return true;
        }
    }

    return false;
}

static void show_cart(const MarketplaceGraph *marketplace, const Cart *cart)
{
    size_t index;
    double total = 0.0;

    print_section("Shopping cart");
    if (cart->count == 0) {
        printf("Your cart is empty.\n");
        return;
    }

    for (index = 0; index < cart->count; index++) {
        print_item(cart->items[index], (void *)marketplace);
        printf("    quantity: %zu | line total: ₹%.2f\n",
               cart->quantities[index],
               cart->quantities[index] * cart->items[index]->price);
        total += cart->quantities[index] * cart->items[index]->price;
    }
    printf("Cart total: ₹%.2f\n", total);
}

static void show_inventory(const MarketplaceGraph *marketplace,
                           const Inventory *inventory)
{
    size_t index;

    print_section("Owned inventory");
    if (inventory->count == 0) {
        printf("You do not own any items yet.\n");
        return;
    }

    for (index = 0; index < inventory->count; index++) {
        print_item(&inventory->items[index], (void *)marketplace);
    }
}

static void browse_items(const MarketplaceGraph *marketplace, int user_id)
{
    int hops;

    if (!read_int("How many connection hops should be searched? ", &hops) ||
        hops < 1) {
        printf("Please choose at least one hop.\n");
        return;
    }

    print_section("Items available through your connections");
    if (!marketplace_query_connected_items(marketplace, user_id,
                                           (size_t)hops, print_item,
                                           (void *)marketplace)) {
        printf("Could not search recommendations.\n");
    }
}

static void add_to_cart(const MarketplaceGraph *marketplace, Cart *cart)
{
    int item_id;
    size_t quantity;
    const Item *item;

    if (cart->count == MAX_CART_ITEMS) {
        printf("Your cart is full.\n");
        return;
    }
    if (!read_int("Enter the item ID to add: ", &item_id)) {
        return;
    }

    item = find_item(marketplace, item_id);
    if (item == NULL) {
        printf("No item with ID %d was found.\n", item_id);
    } else if (cart_contains(cart, item)) {
        printf("That item is already in your cart.\n");
    } else if (item->quantity == 0) {
        printf("That item is currently out of stock.\n");
    } else if (!read_quantity("How many would you like? ", &quantity)) {
        return;
    } else if (quantity > item->quantity) {
        printf("Only %zu units are available.\n", item->quantity);
    } else {
        cart->items[cart->count] = item;
        cart->quantities[cart->count] = quantity;
        cart->count++;
        printf("Added %zu unit(s) of \"%s\" to your cart.\n",
               quantity, item->name);
    }
}

static bool inventory_add(Inventory *inventory, const Item *item,
                          size_t quantity)
{
    size_t index;

    for (index = 0; index < inventory->count; index++) {
        if (inventory->items[index].id == item->id) {
            inventory->items[index].quantity += quantity;
            return true;
        }
    }

    if (inventory->count == MAX_INVENTORY_ITEMS) {
        return false;
    }

    inventory->items[inventory->count] = *item;
    inventory->items[inventory->count].quantity = quantity;
    inventory->items[inventory->count].next = NULL;
    inventory->count++;
    return true;
}

static void checkout(MarketplaceGraph *marketplace, Cart *cart,
                     Inventory *inventory, double *balance)
{
    size_t index;
    double total = 0.0;

    if (cart->count == 0) {
        printf("Your cart is empty.\n");
        return;
    }
    for (index = 0; index < cart->count; index++) {
        if (cart->quantities[index] > cart->items[index]->quantity) {
            printf("\"%s\" no longer has enough stock.\n",
                   cart->items[index]->name);
            return;
        }
        total += cart->quantities[index] * cart->items[index]->price;
    }
    if (total > *balance) {
        printf("Insufficient funds. Cart total is ₹%.2f, balance is ₹%.2f.\n",
               total, *balance);
        return;
    }

    for (index = 0; index < cart->count; index++) {
        if (!inventory_add(inventory, cart->items[index],
                           cart->quantities[index])) {
            printf("Your inventory cannot hold this purchase.\n");
            return;
        }
    }
    for (index = 0; index < cart->count; index++) {
        if (!marketplace_purchase_item(marketplace, cart->items[index]->id,
                                       cart->quantities[index])) {
            printf("The purchase could not be completed because stock changed.\n");
            return;
        }
    }
    *balance -= total;
    cart->count = 0;
    printf("Checkout complete. You spent ₹%.2f.\n", total);
}

static void sell_item(Inventory *inventory, double *balance)
{
    int item_id;
    size_t index;
    size_t quantity;
    double sale_price;

    if (inventory->count == 0) {
        printf("You do not own any items to sell.\n");
        return;
    }
    if (!read_int("Enter the item ID to sell: ", &item_id)) {
        return;
    }

    for (index = 0; index < inventory->count; index++) {
        if (inventory->items[index].id == item_id) {
            break;
        }
    }
    if (index == inventory->count) {
        printf("You do not own an item with ID %d.\n", item_id);
        return;
    }
    if (!read_quantity("How many would you like to sell? ", &quantity)) {
        return;
    }
    if (quantity > inventory->items[index].quantity) {
        printf("You only own %zu unit(s) of that item.\n",
               inventory->items[index].quantity);
        return;
    }

    sale_price = inventory->items[index].price * 0.70 * quantity;
    inventory->items[index].quantity -= quantity;
    *balance += sale_price;
    printf("Sold %zu unit(s) for ₹%.2f (70%% of the original price).\n",
           quantity, sale_price);

    if (inventory->items[index].quantity == 0) {
        inventory->items[index] = inventory->items[inventory->count - 1];
        inventory->count--;
    }
}

static void print_menu(double balance)
{
    printf("\n========================================\n");
    printf("        CONNECTION MARKETPLACE\n");
    printf("========================================\n");
    printf("Wallet balance: ₹%.2f\n", balance);
    printf("1. Browse connected items\n");
    printf("2. Add item to cart\n");
    printf("3. View cart\n");
    printf("4. Add money to wallet\n");
    printf("5. Checkout\n");
    printf("6. Show owned inventory\n");
    printf("7. Sell an owned item\n");
    printf("8. Show current balance\n");
    printf("0. Exit\n");
}

int main(void)
{
    MarketplaceGraph marketplace;
    Cart cart = {0};
    Inventory inventory = {0};
    double balance;
    double amount;
    int choice;
    const int current_user_id = 2;

    marketplace_graph_init(&marketplace);
    if (!marketplace_add_user(&marketplace, 1, "Alice (Senior)") ||
        !marketplace_add_user(&marketplace, 2, "You (Junior)") ||
        !marketplace_add_user(&marketplace, 3, "Carol (Senior)") ||
        !marketplace_add_user(&marketplace, 4, "David (Junior)") ||
        !marketplace_add_connection(&marketplace, 1, 2) ||
        !marketplace_add_connection(&marketplace, 2, 3) ||
        !marketplace_add_connection(&marketplace, 3, 4) ||
        !marketplace_add_item_quantity(&marketplace, 101, 1,
                                       "C Programming Book", 850.00, 5) ||
        !marketplace_add_item_quantity(&marketplace, 102, 3,
                                       "Desk Lamp", 1200.00, 3) ||
        !marketplace_add_item_quantity(&marketplace, 103, 4,
                                       "USB Keyboard", 650.00, 4) ||
        !marketplace_add_item_quantity(&marketplace, 104, 1,
                                       "Scientific Calculator", 950.00, 2) ||
        !marketplace_add_item_quantity(&marketplace, 105, 3,
                                       "Study Table", 4500.00, 2) ||
        !marketplace_add_item_quantity(&marketplace, 106, 4,
                                       "Backpack", 1800.00, 6) ||
        !marketplace_add_item_quantity(&marketplace, 107, 1,
                                       "Arduino Starter Kit", 2200.00, 3) ||
        !marketplace_add_item_quantity(&marketplace, 108, 3,
                                       "Desk Organizer", 400.00, 5)) {
        fprintf(stderr, "Failed to initialize the marketplace.\n");
        marketplace_graph_destroy(&marketplace);
        return EXIT_FAILURE;
    }

    printf("Welcome! You are logged in as You (Junior).\n");
    if (!read_money("Enter your starting wallet balance: ", &balance)) {
        marketplace_graph_destroy(&marketplace);
        return EXIT_SUCCESS;
    }

    while (true) {
        print_menu(balance);
        if (!read_int("Choose an option: ", &choice)) {
            break;
        }

        switch (choice) {
        case 1:
            browse_items(&marketplace, current_user_id);
            break;
        case 2:
            add_to_cart(&marketplace, &cart);
            break;
        case 3:
            show_cart(&marketplace, &cart);
            break;
        case 4:
            if (read_money("Amount to add: ", &amount)) {
                balance += amount;
                printf("Wallet balance is now ₹%.2f.\n", balance);
            }
            break;
        case 5:
            checkout(&marketplace, &cart, &inventory, &balance);
            break;
        case 6:
            show_inventory(&marketplace, &inventory);
            break;
        case 7:
            sell_item(&inventory, &balance);
            break;
        case 8:
            printf("Current wallet balance: ₹%.2f\n", balance);
            break;
        case 0:
            marketplace_graph_destroy(&marketplace);
            printf("Goodbye!\n");
            return EXIT_SUCCESS;
        default:
            printf("Unknown option. Please choose from the menu.\n");
            break;
        }
    }

    marketplace_graph_destroy(&marketplace);
    return EXIT_SUCCESS;
}
