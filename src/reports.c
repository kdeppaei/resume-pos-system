#include <stdio.h>

#include "reports.h"
#include "core.h"
#include "input.h"
#include "transaction.h"

void reports_sales_summary(const Database *database) {
    int indices[MAX_ITEMS];
    int occupied_count;
    int active_count;
    int archived_count;
    int ranked_count;
    int display_count;
    int i;
    long long inventory_value;
    const Item *item;

    occupied_count = 0;
    active_count = 0;
    archived_count = 0;
    ranked_count = 0;
    inventory_value = 0;

    for (i = 0; i < MAX_ITEMS; i++) {
        if (!item_is_occupied(database, i)) {
            continue;
        }

        occupied_count++;

        if (database->items[i].active) {
            active_count++;
            inventory_value +=
                (long long)database->items[i].stock *
                database->items[i].price;
        } else {
            archived_count++;
        }

        if (database->items[i].revenue > 0) {
            indices[ranked_count++] = i;
        }
    }

    printf("\nSales summary\n");
    printf("-------------\n");
    printf("All products       : %d\n", occupied_count);
    printf("Active products    : %d\n", active_count);
    printf("Archived products  : %d\n", archived_count);
    printf("Stored transactions: %d / %d\n",
           database->transaction_count, MAX_TRANSACTIONS);
    printf("Total revenue      : NT$%" POS_LL "d\n", database->total_revenue);
    printf("Inventory value    : NT$%" POS_LL "d\n", inventory_value);

    if (ranked_count == 0) {
        printf("\nNo sales ranking is available yet.\n");
        return;
    }

    item_sort_indices_by_revenue_desc(database, indices, ranked_count);
    display_count = ranked_count < 5 ? ranked_count : 5;

    printf("\nTop products by revenue\n");
    printf("Rank | Code | Name                         | Paid sold | Revenue\n");
    printf("-----+------+------------------------------+-----------+-------------\n");

    for (i = 0; i < display_count; i++) {
        item = &database->items[indices[i]];
        printf("%-4d | %s | %-28.28s | %-9" POS_LL "d | NT$%" POS_LL "d\n",
               i + 1,
               item->code,
               item->name,
               item->paid_units_sold,
               item->revenue);
    }
}

void reports_low_stock(const Database *database) {
    int threshold;
    int indices[MAX_ITEMS];
    int count;
    int i;

    threshold = input_read_int_range("Show stock at or below (0-100000000): ",
                                     0, 100000000);
    count = 0;

    for (i = 0; i < MAX_ITEMS; i++) {
        if (item_is_active(database, i) &&
            database->items[i].stock <= threshold) {
            indices[count++] = i;
        }
    }

    item_sort_indices_by_code(database, indices, count);
    item_print_table_header();

    if (count == 0) {
        printf("No active products are at or below this threshold.\n");
        return;
    }

    for (i = 0; i < count; i++) {
        item_print_table_row(&database->items[indices[i]]);
    }

    printf("Low-stock products: %d\n", count);
}

void reports_transaction_history(const Database *database) {
    const Transaction *transaction;
    int first_index;
    int id;
    int i;

    printf("\nRecent transaction history\n");
    printf("ID     | Time                | Lines | Total\n");
    printf("-------+---------------------+-------+-------------\n");

    if (database->transaction_count == 0) {
        printf("(No transactions)\n");
        return;
    }

    first_index = database->transaction_count - 20;
    if (first_index < 0) {
        first_index = 0;
    }

    for (i = database->transaction_count - 1; i >= first_index; i--) {
        transaction = transaction_at(database, i);
        printf("%06d | %s | %-5d | NT$%" POS_LL "d\n",
               transaction->id,
               transaction->timestamp,
               transaction->line_count,
               transaction->total);
    }

    id = input_read_int_range(
        "Enter transaction ID to view, or 0 to return: ",
        0, 2147483647);

    if (id == 0) {
        return;
    }

    transaction = transaction_find_by_id(database, id);
    if (transaction == NULL) {
        printf("Transaction not found. It may have been rotated out.\n");
        return;
    }

    transaction_print_receipt(transaction);
}

static void write_csv_field(FILE *file, const char *text) {
    const char *cursor;

    fputc('"', file);

    for (cursor = text; *cursor != '\0'; cursor++) {
        if (*cursor == '"') {
            fputc('"', file);
        }
        fputc(*cursor, file);
    }

    fputc('"', file);
}

static Bool export_products_csv(const Database *database) {
    FILE *file;
    char activity[64];
    int i;

    file = fopen(PRODUCTS_CSV, "w");
    if (file == NULL) {
        perror("Could not create product CSV");
        return FALSE;
    }

    fprintf(file,
            "code,name,status,stock,price,activity,paid_units_sold,free_units_given,revenue\n");

    for (i = 0; i < MAX_ITEMS; i++) {
        if (!item_is_occupied(database, i)) {
            continue;
        }

        item_format_activity(&database->items[i], activity, sizeof(activity));
        write_csv_field(file, database->items[i].code);
        fputc(',', file);
        write_csv_field(file, database->items[i].name);
        fputc(',', file);
        write_csv_field(file,
                        database->items[i].active ? "Active" : "Archived");
        fprintf(file, ",%d,%d,",
                database->items[i].stock,
                database->items[i].price);
        write_csv_field(file, activity);
        fprintf(file, ",%" POS_LL "d,%" POS_LL "d,%" POS_LL "d\n",
                database->items[i].paid_units_sold,
                database->items[i].free_units_given,
                database->items[i].revenue);
    }

    if (fclose(file) != 0) {
        printf("Could not finish product CSV.\n");
        return FALSE;
    }

    return TRUE;
}

static Bool export_transactions_csv(const Database *database) {
    FILE *file;
    const Transaction *transaction;
    const TransactionLine *line;
    int i;
    int j;

    file = fopen(TRANSACTIONS_CSV, "w");
    if (file == NULL) {
        perror("Could not create transaction CSV");
        return FALSE;
    }

    fprintf(file,
            "transaction_id,timestamp,product_code,product_name,paid_quantity,free_quantity,unit_price,subtotal,transaction_total\n");

    for (i = 0; i < database->transaction_count; i++) {
        transaction = transaction_at(database, i);

        for (j = 0; j < transaction->line_count; j++) {
            line = &transaction->lines[j];

            fprintf(file, "%d,", transaction->id);
            write_csv_field(file, transaction->timestamp);
            fputc(',', file);
            write_csv_field(file, line->code);
            fputc(',', file);
            write_csv_field(file, line->name);
            fprintf(file, ",%d,%d,%d,%" POS_LL "d,%" POS_LL "d\n",
                    line->paid_quantity,
                    line->free_quantity,
                    line->unit_price,
                    line->subtotal,
                    transaction->total);
        }
    }

    if (fclose(file) != 0) {
        printf("Could not finish transaction CSV.\n");
        return FALSE;
    }

    return TRUE;
}

void reports_export_csv(const Database *database) {
    Bool products_ok;
    Bool transactions_ok;

    products_ok = export_products_csv(database);
    transactions_ok = export_transactions_csv(database);

    if (products_ok && transactions_ok) {
        printf("CSV export successful.\n");
        printf("Created: %s\n", PRODUCTS_CSV);
        printf("Created: %s\n", TRANSACTIONS_CSV);
    } else {
        printf("One or more CSV reports could not be exported.\n");
    }
}
