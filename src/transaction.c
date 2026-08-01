#include <stdio.h>
#include <string.h>
#include <time.h>

#include "transaction.h"
#include "core.h"

long long cart_calculate_total(const Database *database,
                               const CartLine cart[],
                               int cart_count) {
    long long total;
    int i;

    total = 0;

    for (i = 0; i < cart_count; i++) {
        total += (long long)cart[i].paid_quantity *
                 database->items[cart[i].item_index].price;
    }

    return total;
}

static void get_timestamp(char timestamp[20]) {
    time_t current_time;
    struct tm *local_time;

    current_time = time(NULL);
    local_time = localtime(&current_time);

    if (local_time == NULL) {
        pos_copy_text(timestamp, 20, "Unknown time");
        return;
    }

    strftime(timestamp, 20, "%Y-%m-%d %H:%M:%S", local_time);
}

void transaction_build(const Database *database,
                       Transaction *transaction,
                       const CartLine cart[],
                       int cart_count) {
    const Item *item;
    TransactionLine *line;
    int i;

    memset(transaction, 0, sizeof(Transaction));
    transaction->id = database->next_transaction_id;
    transaction->line_count = cart_count;
    transaction->total = cart_calculate_total(database, cart, cart_count);
    get_timestamp(transaction->timestamp);

    for (i = 0; i < cart_count; i++) {
        item = &database->items[cart[i].item_index];
        line = &transaction->lines[i];

        pos_copy_text(line->code, sizeof(line->code), item->code);
        pos_copy_text(line->name, sizeof(line->name), item->name);
        line->paid_quantity = cart[i].paid_quantity;
        line->free_quantity =
            item_free_quantity(item, cart[i].paid_quantity);
        line->unit_price = item->price;
        line->subtotal =
            (long long)line->paid_quantity * line->unit_price;
    }
}

void transaction_append(Database *database,
                        const Transaction *transaction) {
    int physical_index;

    if (database->transaction_count < MAX_TRANSACTIONS) {
        physical_index =
            (database->transaction_start + database->transaction_count) %
            MAX_TRANSACTIONS;
        database->transaction_count++;
    } else {
        physical_index = database->transaction_start;
        database->transaction_start =
            (database->transaction_start + 1) % MAX_TRANSACTIONS;
    }

    database->transactions[physical_index] = *transaction;
    database->next_transaction_id++;
}

const Transaction *transaction_at(const Database *database,
                                  int logical_index) {
    int physical_index;

    if (database == NULL || logical_index < 0 ||
        logical_index >= database->transaction_count) {
        return NULL;
    }

    physical_index =
        (database->transaction_start + logical_index) % MAX_TRANSACTIONS;
    return &database->transactions[physical_index];
}

const Transaction *transaction_find_by_id(const Database *database, int id) {
    const Transaction *transaction;
    int i;

    for (i = 0; i < database->transaction_count; i++) {
        transaction = transaction_at(database, i);
        if (transaction != NULL && transaction->id == id) {
            return transaction;
        }
    }

    return NULL;
}

void transaction_print_receipt(const Transaction *transaction) {
    const TransactionLine *line;
    int i;

    printf("\n");
    printf("==============================================\n");
    printf("                 POS RECEIPT\n");
    printf("==============================================\n");
    printf("Transaction : %06d\n", transaction->id);
    printf("Time        : %s\n", transaction->timestamp);
    printf("----------------------------------------------\n");

    for (i = 0; i < transaction->line_count; i++) {
        line = &transaction->lines[i];

        printf("%s %s\n", line->code, line->name);
        printf("  %d x NT$%d = NT$%" POS_LL "d",
               line->paid_quantity,
               line->unit_price,
               line->subtotal);

        if (line->free_quantity > 0) {
            printf("  (+%d free)", line->free_quantity);
        }

        printf("\n");
    }

    printf("----------------------------------------------\n");
    printf("TOTAL: NT$%" POS_LL "d\n", transaction->total);
    printf("==============================================\n");
}
