#include <stdio.h>
#include <string.h>

#include "catalog.h"
#include "core.h"
#include "input.h"
#include "storage.h"

static void configure_activity(Item *item) {
    int choice;

    choice = input_read_int_range(
        "Activity [0] None [1] Buy X get Y free: ", 0, 1);

    if (choice == 1) {
        item->activity_type = 'B';
        item->activity_x =
            input_read_int_range("Buy X (1-1000): ", 1, 1000);
        item->activity_y =
            input_read_int_range("Get Y free (1-1000): ", 1, 1000);
    } else {
        item->activity_type = 'A';
        item->activity_x = 0;
        item->activity_y = 0;
    }
}

void catalog_add_product(Database *database) {
    int empty_slot;
    int code_number;
    int price;
    int stock;
    char name[MAX_NAME_BYTES + 1];
    Item new_item;

    empty_slot = item_find_empty_slot(database);
    if (empty_slot == -1) {
        printf("The product database is full.\n");
        return;
    }

    code_number = input_read_int_range("Item number (1-9999): ", 1, 9999);
    if (item_find_by_code_number(database, code_number, FALSE) != -1) {
        printf("This item number already exists, possibly as an archived item.\n");
        return;
    }

    while (TRUE) {
        input_read_line("Name: ", name, sizeof(name));

        if (name[0] == '\0') {
            printf("Name cannot be empty.\n");
            continue;
        }

        if (item_find_by_exact_name(database, name, TRUE, -1) != -1) {
            printf("This active product name already exists.\n");
            continue;
        }

        break;
    }

    price = input_read_int_range("Price (NT$1-1000000): ", 1, 1000000);
    stock = input_read_int_range("Initial stock (0-100000000): ",
                                 0, 100000000);

    memset(&new_item, 0, sizeof(new_item));
    new_item.occupied = TRUE;
    new_item.active = TRUE;
    sprintf(new_item.code, "%04d", code_number);
    pos_copy_text(new_item.name, sizeof(new_item.name), name);
    new_item.price = price;
    new_item.stock = stock;
    configure_activity(&new_item);

    database->items[empty_slot] = new_item;

    if (!database_save(database)) {
        memset(&database->items[empty_slot], 0,
               sizeof(database->items[empty_slot]));
        printf("Add cancelled because data could not be saved.\n");
        return;
    }

    printf("Add %s successful!\n", new_item.name);
}

void catalog_list_products(const Database *database) {
    int indices[MAX_ITEMS];
    int count;
    int i;

    count = 0;

    for (i = 0; i < MAX_ITEMS; i++) {
        if (item_is_active(database, i)) {
            indices[count++] = i;
        }
    }

    item_sort_indices_by_code(database, indices, count);
    item_print_table_header();

    if (count == 0) {
        printf("(No active products)\n");
        return;
    }

    for (i = 0; i < count; i++) {
        item_print_table_row(&database->items[indices[i]]);
    }
}

void catalog_search_product(const Database *database) {
    char key[128];
    int index;
    int code_number;
    int matches;
    int i;

    input_read_line("Enter item number or part of a name: ", key, sizeof(key));

    if (key[0] == '\0') {
        printf("Search text cannot be empty.\n");
        return;
    }

    if (pos_is_all_digits(key)) {
        if (!input_parse_int(key, &code_number)) {
            printf("Invalid item number.\n");
            return;
        }

        index = item_find_by_code_number(database, code_number, TRUE);
        if (index == -1) {
            printf("The product doesn't exist.\n");
            return;
        }

        item_print_details(&database->items[index]);
        return;
    }

    matches = 0;
    item_print_table_header();

    for (i = 0; i < MAX_ITEMS; i++) {
        if (item_is_active(database, i) &&
            pos_contains_ignore_case(database->items[i].name, key)) {
            item_print_table_row(&database->items[i]);
            matches++;
        }
    }

    if (matches == 0) {
        printf("No matching products.\n");
    } else {
        printf("Matches: %d\n", matches);
    }
}

void catalog_edit_product(Database *database) {
    char key[128];
    char new_name[MAX_NAME_BYTES + 1];
    int index;
    int choice;
    int old_value;
    Item backup;

    input_read_line("Enter item number or exact name: ", key, sizeof(key));
    index = item_find_active_by_key(database, key);

    if (index == -1) {
        printf("The product doesn't exist.\n");
        return;
    }

    item_print_details(&database->items[index]);
    printf("\n1. Change name\n");
    printf("2. Change price\n");
    printf("3. Set stock\n");
    printf("4. Change activity\n");
    printf("0. Cancel\n");

    choice = input_read_int_range("Select: ", 0, 4);
    if (choice == 0) {
        printf("Edit cancelled.\n");
        return;
    }

    backup = database->items[index];

    if (choice == 1) {
        input_read_line("New name: ", new_name, sizeof(new_name));

        if (new_name[0] == '\0') {
            printf("Name cannot be empty.\n");
            return;
        }

        if (item_find_by_exact_name(database, new_name, TRUE, index) != -1) {
            printf("Another active product already uses this name.\n");
            return;
        }

        pos_copy_text(database->items[index].name,
                      sizeof(database->items[index].name),
                      new_name);
    } else if (choice == 2) {
        old_value = database->items[index].price;
        database->items[index].price =
            input_read_int_range("New price (NT$1-1000000): ", 1, 1000000);
        printf("Price changed from NT$%d to NT$%d.\n",
               old_value, database->items[index].price);
    } else if (choice == 3) {
        old_value = database->items[index].stock;
        database->items[index].stock =
            input_read_int_range("New stock (0-100000000): ",
                                 0, 100000000);
        printf("Stock changed from %d to %d.\n",
               old_value, database->items[index].stock);
    } else {
        configure_activity(&database->items[index]);
    }

    if (!database_save(database)) {
        database->items[index] = backup;
        printf("Edit rolled back because data could not be saved.\n");
        return;
    }

    printf("Product updated successfully.\n");
}

void catalog_restock_product(Database *database) {
    char key[128];
    int index;
    int quantity;
    int original_stock;

    input_read_line("Enter item number or exact name: ", key, sizeof(key));
    index = item_find_active_by_key(database, key);

    if (index == -1) {
        printf("The product doesn't exist.\n");
        return;
    }

    item_print_details(&database->items[index]);
    quantity = input_read_int_range("Quantity to add (1-100000000): ",
                                    1, 100000000);

    if (database->items[index].stock > 100000000 - quantity) {
        printf("Stock would exceed the allowed maximum.\n");
        return;
    }

    original_stock = database->items[index].stock;
    database->items[index].stock += quantity;

    if (!database_save(database)) {
        database->items[index].stock = original_stock;
        printf("Restock rolled back because data could not be saved.\n");
        return;
    }

    printf("Restock successful. New stock: %d\n",
           database->items[index].stock);
}

void catalog_archive_product(Database *database) {
    char key[128];
    int index;

    input_read_line("Enter item number or exact name: ", key, sizeof(key));
    index = item_find_active_by_key(database, key);

    if (index == -1) {
        printf("The product doesn't exist.\n");
        return;
    }

    item_print_details(&database->items[index]);

    if (!input_confirm("Archive this product? [Y/N]: ")) {
        printf("Archive cancelled.\n");
        return;
    }

    database->items[index].active = FALSE;

    if (!database_save(database)) {
        database->items[index].active = TRUE;
        printf("Archive rolled back because data could not be saved.\n");
        return;
    }

    printf("Product archived. Historical sales data was preserved.\n");
}

void catalog_restore_product(Database *database) {
    char key[128];
    int index;

    input_read_line("Enter archived item number or exact name: ",
                    key, sizeof(key));
    index = item_find_archived_by_key(database, key);

    if (index == -1) {
        printf("The archived product doesn't exist.\n");
        return;
    }

    if (item_find_by_exact_name(database,
                                database->items[index].name,
                                TRUE,
                                index) != -1) {
        printf("Restore blocked: an active product uses the same name.\n");
        return;
    }

    item_print_details(&database->items[index]);

    if (!input_confirm("Restore this product? [Y/N]: ")) {
        printf("Restore cancelled.\n");
        return;
    }

    database->items[index].active = TRUE;

    if (!database_save(database)) {
        database->items[index].active = FALSE;
        printf("Restore rolled back because data could not be saved.\n");
        return;
    }

    printf("Product restored successfully.\n");
}
