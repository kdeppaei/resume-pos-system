#include <stdio.h>

#include "catalog.h"
#include "checkout.h"
#include "input.h"
#include "reports.h"
#include "storage.h"

static void print_menu(void) {
    printf("\n");
    printf("====================================\n");
    printf("       Resume POS System V3.0\n");
    printf("====================================\n");
    printf("1.  Add product\n");
    printf("2.  List active products\n");
    printf("3.  Search product\n");
    printf("4.  Edit product\n");
    printf("5.  Restock product\n");
    printf("6.  Checkout\n");
    printf("7.  Archive product\n");
    printf("8.  Restore archived product\n");
    printf("9.  Sales summary\n");
    printf("10. Low-stock report\n");
    printf("11. Transaction history\n");
    printf("12. Export CSV reports\n");
    printf("0.  Save and exit\n");
    printf("====================================\n");
}

int main(void) {
    Database database;
    int choice;

    database_load(&database);

    while (TRUE) {
        print_menu();
        choice = input_read_int_range("Select: ", 0, 12);

        switch (choice) {
            case 1:
                catalog_add_product(&database);
                break;
            case 2:
                catalog_list_products(&database);
                break;
            case 3:
                catalog_search_product(&database);
                break;
            case 4:
                catalog_edit_product(&database);
                break;
            case 5:
                catalog_restock_product(&database);
                break;
            case 6:
                checkout_run(&database);
                break;
            case 7:
                catalog_archive_product(&database);
                break;
            case 8:
                catalog_restore_product(&database);
                break;
            case 9:
                reports_sales_summary(&database);
                break;
            case 10:
                reports_low_stock(&database);
                break;
            case 11:
                reports_transaction_history(&database);
                break;
            case 12:
                reports_export_csv(&database);
                break;
            case 0:
                if (database_save(&database)) {
                    printf("Data saved. Goodbye!\n");
                } else {
                    printf("Goodbye, but the latest data could not be saved.\n");
                }
                return 0;
            default:
                break;
        }
    }
}
