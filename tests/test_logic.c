#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core.h"
#include "storage.h"
#include "transaction.h"

static void seed_item(Database *database,
                      int index,
                      const char *code,
                      const char *name,
                      int price,
                      int stock) {
    Item *item;

    item = &database->items[index];
    memset(item, 0, sizeof(Item));
    item->occupied = TRUE;
    item->active = TRUE;
    pos_copy_text(item->code, sizeof(item->code), code);
    pos_copy_text(item->name, sizeof(item->name), name);
    item->price = price;
    item->stock = stock;
    item->activity_type = 'A';
}

static void test_promotion(void) {
    Item item;

    memset(&item, 0, sizeof(item));
    item.activity_type = 'B';
    item.activity_x = 2;
    item.activity_y = 1;

    assert(item_free_quantity(&item, 1) == 0);
    assert(item_free_quantity(&item, 2) == 1);
    assert(item_free_quantity(&item, 4) == 2);
    assert(item_free_quantity(&item, 5) == 2);
}

static void test_search_and_sort(void) {
    Database *database;
    int indices[3];

    database = (Database *)malloc(sizeof(Database));
    assert(database != NULL);
    database_initialize(database);

    seed_item(database, 0, "0030", "Milk Tea", 60, 10);
    seed_item(database, 1, "0002", "Bread", 35, 20);
    seed_item(database, 2, "0100", "Apple", 25, 30);

    assert(item_find_active_by_key(database, "2") == 1);
    assert(item_find_active_by_key(database, "bread") == 1);
    assert(pos_contains_ignore_case("Milk Tea", "tea") == TRUE);

    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;
    item_sort_indices_by_code(database, indices, 3);

    assert(indices[0] == 1);
    assert(indices[1] == 0);
    assert(indices[2] == 2);

    free(database);
}

static void test_transaction_ring_buffer(void) {
    Database *database;
    Transaction transaction;
    const Transaction *oldest;
    int i;

    database = (Database *)malloc(sizeof(Database));
    assert(database != NULL);
    database_initialize(database);
    memset(&transaction, 0, sizeof(transaction));

    for (i = 0; i < MAX_TRANSACTIONS + 5; i++) {
        transaction.id = database->next_transaction_id;
        transaction_append(database, &transaction);
    }

    assert(database->transaction_count == MAX_TRANSACTIONS);
    assert(database->next_transaction_id == MAX_TRANSACTIONS + 6);

    oldest = transaction_at(database, 0);
    assert(oldest != NULL);
    assert(oldest->id == 6);

    free(database);
}

static void test_transaction_snapshot(void) {
    Database *database;
    CartLine cart[1];
    Transaction transaction;

    database = (Database *)malloc(sizeof(Database));
    assert(database != NULL);
    database_initialize(database);
    seed_item(database, 0, "0001", "Coffee", 80, 20);
    database->items[0].activity_type = 'B';
    database->items[0].activity_x = 2;
    database->items[0].activity_y = 1;

    cart[0].item_index = 0;
    cart[0].paid_quantity = 4;
    transaction_build(database, &transaction, cart, 1);

    assert(transaction.total == 320);
    assert(transaction.lines[0].paid_quantity == 4);
    assert(transaction.lines[0].free_quantity == 2);
    assert(transaction.lines[0].unit_price == 80);
    assert(transaction.lines[0].subtotal == 320);

    pos_copy_text(database->items[0].name,
                  sizeof(database->items[0].name),
                  "Renamed Coffee");
    assert(strcmp(transaction.lines[0].name, "Coffee") == 0);

    free(database);
}

int main(void) {
    test_promotion();
    test_search_and_sort();
    test_transaction_ring_buffer();
    test_transaction_snapshot();

    printf("All POS logic tests passed.\n");
    return 0;
}
