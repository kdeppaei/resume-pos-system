#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "checkout.h"
#include "core.h"
#include "input.h"
#include "storage.h"
#include "transaction.h"

static int find_cart_line(const CartLine cart[],
                          int cart_count,
                          int item_index) {
    int i;

    for (i = 0; i < cart_count; i++) {
        if (cart[i].item_index == item_index) {
            return i;
        }
    }

    return -1;
}

static void print_cart(const Database *database,
                       const CartLine cart[],
                       int cart_count) {
    const Item *item;
    int paid_quantity;
    int free_quantity;
    long long subtotal;
    int i;

    printf("\n");
    printf("Code | Name                         | Paid | Free | Unit price | Subtotal\n");
    printf("-----+------------------------------+------+------+------------+----------\n");

    for (i = 0; i < cart_count; i++) {
        item = &database->items[cart[i].item_index];
        paid_quantity = cart[i].paid_quantity;
        free_quantity = item_free_quantity(item, paid_quantity);
        subtotal = (long long)paid_quantity * item->price;

        printf("%s | %-28.28s | %-4d | %-4d | NT$%-8d | NT$%" POS_LL "d\n",
               item->code,
               item->name,
               paid_quantity,
               free_quantity,
               item->price,
               subtotal);
    }

    printf("--------------------------------------------------------------------------\n");
    printf("Total: NT$%" POS_LL "d\n",
           cart_calculate_total(database, cart, cart_count));
}

void checkout_run(Database *database) {
    CartLine cart[MAX_CART_LINES];
    Transaction transaction;
    Database *backup;
    char key[128];
    int cart_count;
    int item_index;
    int quantity;
    int cart_line_index;
    int new_paid_quantity;
    int new_free_quantity;
    int units_needed;
    int i;
    Item *item;

    cart_count = 0;

    printf("\nCheckout mode\n");
    printf("Enter 0 as the item key to finish the cart.\n");

    while (TRUE) {
        input_read_line("\nItem number or exact name: ", key, sizeof(key));

        if (strcmp(key, "0") == 0) {
            break;
        }

        item_index = item_find_active_by_key(database, key);
        if (item_index == -1) {
            printf("The product doesn't exist.\n");
            continue;
        }

        item = &database->items[item_index];
        item_print_details(item);

        quantity = input_read_int_range("Paid quantity (1-100000): ",
                                        1, 100000);
        cart_line_index = find_cart_line(cart, cart_count, item_index);
        new_paid_quantity = quantity;

        if (cart_line_index != -1) {
            new_paid_quantity += cart[cart_line_index].paid_quantity;
        }

        new_free_quantity = item_free_quantity(item, new_paid_quantity);
        units_needed = new_paid_quantity + new_free_quantity;

        if (units_needed > item->stock) {
            printf("Not enough stock. Available: %d, required: %d.\n",
                   item->stock, units_needed);
            continue;
        }

        if (cart_line_index != -1) {
            cart[cart_line_index].paid_quantity = new_paid_quantity;
        } else {
            if (cart_count >= MAX_CART_LINES) {
                printf("The cart is full.\n");
                break;
            }

            cart[cart_count].item_index = item_index;
            cart[cart_count].paid_quantity = quantity;
            cart_count++;
        }

        printf("Added to cart.\n");
        print_cart(database, cart, cart_count);
    }

    if (cart_count == 0) {
        printf("Checkout cancelled: cart is empty.\n");
        return;
    }

    print_cart(database, cart, cart_count);

    if (!input_confirm("Confirm payment? [Y/N]: ")) {
        printf("Checkout cancelled.\n");
        return;
    }

    backup = (Database *)malloc(sizeof(Database));
    if (backup == NULL) {
        printf("Checkout cancelled: not enough memory for rollback protection.\n");
        return;
    }

    memcpy(backup, database, sizeof(Database));
    transaction_build(database, &transaction, cart, cart_count);

    for (i = 0; i < cart_count; i++) {
        item = &database->items[cart[i].item_index];
        new_free_quantity = item_free_quantity(item, cart[i].paid_quantity);
        units_needed = cart[i].paid_quantity + new_free_quantity;

        item->stock -= units_needed;
        item->paid_units_sold += cart[i].paid_quantity;
        item->free_units_given += new_free_quantity;
        item->revenue +=
            (long long)cart[i].paid_quantity * item->price;
    }

    database->total_revenue += transaction.total;
    transaction_append(database, &transaction);

    if (!database_save(database)) {
        memcpy(database, backup, sizeof(Database));
        free(backup);
        printf("Checkout rolled back because data could not be saved.\n");
        return;
    }

    free(backup);
    transaction_print_receipt(&transaction);
}
