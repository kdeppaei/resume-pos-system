#ifndef POS_TYPES_H
#define POS_TYPES_H

#define TRUE 1
#define FALSE 0

#define MAX_ITEMS 1000
#define MAX_NAME_BYTES 60
#define MAX_CART_LINES 50
#define MAX_TRANSACTIONS 500
#define MAX_TRANSACTION_LINES 50

#define DATABASE_VERSION 2
#define DATA_FILE "pos_data_v2.dat"
#define TEMP_FILE "pos_data_v2.tmp"
#define BACKUP_FILE "pos_data_v2.bak"
#define PRODUCTS_CSV "products_report.csv"
#define TRANSACTIONS_CSV "transactions_report.csv"

/* MinGW's legacy MSVCRT uses I64 for 64-bit printf values; POS_LL keeps
 * the GNU90 code warning-free on both Windows and POSIX toolchains. */
#ifdef _WIN32
#define POS_LL "I64"
#else
#define POS_LL "ll"
#endif

typedef int Bool;

typedef struct {
    int occupied;
    int active;
    char code[5];
    char name[MAX_NAME_BYTES + 1];
    int stock;
    int price;
    char activity_type;
    int activity_x;
    int activity_y;
    long long paid_units_sold;
    long long free_units_given;
    long long revenue;
} Item;

typedef struct {
    char code[5];
    char name[MAX_NAME_BYTES + 1];
    int paid_quantity;
    int free_quantity;
    int unit_price;
    long long subtotal;
} TransactionLine;

typedef struct {
    int id;
    char timestamp[20];
    int line_count;
    long long total;
    TransactionLine lines[MAX_TRANSACTION_LINES];
} Transaction;

typedef struct {
    int version;
    Item items[MAX_ITEMS];
    Transaction transactions[MAX_TRANSACTIONS];
    int transaction_start;
    int transaction_count;
    int next_transaction_id;
    long long total_revenue;
} Database;

typedef struct {
    int item_index;
    int paid_quantity;
} CartLine;

#endif
