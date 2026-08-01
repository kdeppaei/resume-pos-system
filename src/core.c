#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core.h"
#include "input.h"

void pos_copy_text(char *destination,
                   size_t destination_size,
                   const char *source) {
    if (destination == NULL || destination_size == 0) {
        return;
    }

    if (source == NULL) {
        destination[0] = '\0';
        return;
    }

    strncpy(destination, source, destination_size - 1);
    destination[destination_size - 1] = '\0';
}

Bool pos_is_all_digits(const char *text) {
    int i;

    if (text == NULL || text[0] == '\0') {
        return FALSE;
    }

    for (i = 0; text[i] != '\0'; i++) {
        if (!isdigit((unsigned char)text[i])) {
            return FALSE;
        }
    }

    return TRUE;
}

Bool pos_equals_ignore_case(const char *left, const char *right) {
    if (left == NULL || right == NULL) {
        return FALSE;
    }

    while (*left != '\0' && *right != '\0') {
        if (tolower((unsigned char)*left) !=
            tolower((unsigned char)*right)) {
            return FALSE;
        }

        left++;
        right++;
    }

    return *left == '\0' && *right == '\0';
}

Bool pos_contains_ignore_case(const char *text, const char *pattern) {
    const char *text_cursor;
    const char *left;
    const char *right;

    if (text == NULL || pattern == NULL) {
        return FALSE;
    }

    if (pattern[0] == '\0') {
        return TRUE;
    }

    for (text_cursor = text; *text_cursor != '\0'; text_cursor++) {
        left = text_cursor;
        right = pattern;

        while (*left != '\0' && *right != '\0' &&
               tolower((unsigned char)*left) ==
               tolower((unsigned char)*right)) {
            left++;
            right++;
        }

        if (*right == '\0') {
            return TRUE;
        }
    }

    return FALSE;
}

Bool item_is_occupied(const Database *database, int index) {
    return database != NULL &&
           index >= 0 && index < MAX_ITEMS &&
           database->items[index].occupied;
}

Bool item_is_active(const Database *database, int index) {
    return item_is_occupied(database, index) &&
           database->items[index].active;
}

int item_find_empty_slot(const Database *database) {
    int i;

    for (i = 0; i < MAX_ITEMS; i++) {
        if (!database->items[i].occupied) {
            return i;
        }
    }

    return -1;
}

int item_find_by_code_number(const Database *database,
                             int code_number,
                             Bool active_only) {
    char normalized_code[5];
    int i;

    if (database == NULL || code_number < 1 || code_number > 9999) {
        return -1;
    }

    sprintf(normalized_code, "%04d", code_number);

    for (i = 0; i < MAX_ITEMS; i++) {
        if (!item_is_occupied(database, i)) {
            continue;
        }

        if (active_only && !database->items[i].active) {
            continue;
        }

        if (strcmp(database->items[i].code, normalized_code) == 0) {
            return i;
        }
    }

    return -1;
}

int item_find_by_exact_name(const Database *database,
                            const char *name,
                            Bool active_only,
                            int excluded_index) {
    int i;

    if (database == NULL || name == NULL) {
        return -1;
    }

    for (i = 0; i < MAX_ITEMS; i++) {
        if (i == excluded_index || !item_is_occupied(database, i)) {
            continue;
        }

        if (active_only && !database->items[i].active) {
            continue;
        }

        if (pos_equals_ignore_case(database->items[i].name, name)) {
            return i;
        }
    }

    return -1;
}

static int find_by_key(const Database *database,
                       const char *key,
                       int wanted_active) {
    int code_number;
    int i;

    if (database == NULL || key == NULL || key[0] == '\0') {
        return -1;
    }

    if (pos_is_all_digits(key)) {
        if (!input_parse_int(key, &code_number)) {
            return -1;
        }

        i = item_find_by_code_number(database, code_number, FALSE);
        if (i == -1) {
            return -1;
        }

        return database->items[i].active == wanted_active ? i : -1;
    }

    for (i = 0; i < MAX_ITEMS; i++) {
        if (!item_is_occupied(database, i)) {
            continue;
        }

        if (database->items[i].active != wanted_active) {
            continue;
        }

        if (pos_equals_ignore_case(database->items[i].name, key)) {
            return i;
        }
    }

    return -1;
}

int item_find_active_by_key(const Database *database, const char *key) {
    return find_by_key(database, key, TRUE);
}

int item_find_archived_by_key(const Database *database, const char *key) {
    return find_by_key(database, key, FALSE);
}

int item_free_quantity(const Item *item, int paid_quantity) {
    if (item == NULL || item->activity_type != 'B' ||
        item->activity_x <= 0 || item->activity_y <= 0 ||
        paid_quantity <= 0) {
        return 0;
    }

    return (paid_quantity / item->activity_x) * item->activity_y;
}

void item_format_activity(const Item *item,
                          char *buffer,
                          size_t buffer_size) {
    if (item == NULL || buffer == NULL || buffer_size == 0) {
        return;
    }

    if (item->activity_type == 'B') {
        sprintf(buffer,
                "Buy %d get %d free",
                item->activity_x,
                item->activity_y);
    } else {
        pos_copy_text(buffer, buffer_size, "No activity");
    }
}

void item_print_details(const Item *item) {
    char activity[64];

    if (item == NULL) {
        return;
    }

    item_format_activity(item, activity, sizeof(activity));

    printf("\nProduct information\n");
    printf("-------------------\n");
    printf("Code       : %s\n", item->code);
    printf("Name       : %s\n", item->name);
    printf("Status     : %s\n", item->active ? "Active" : "Archived");
    printf("Stock      : %d\n", item->stock);
    printf("Price      : NT$%d\n", item->price);
    printf("Activity   : %s\n", activity);
    printf("Paid sold  : %" POS_LL "d\n", item->paid_units_sold);
    printf("Free given : %" POS_LL "d\n", item->free_units_given);
    printf("Revenue    : NT$%" POS_LL "d\n", item->revenue);
}

void item_print_table_header(void) {
    printf("\n");
    printf("Code | Name                         | Stock    | Price       | Activity\n");
    printf("-----+------------------------------+----------+-------------+-----------------------\n");
}

void item_print_table_row(const Item *item) {
    char activity[64];

    item_format_activity(item, activity, sizeof(activity));

    printf("%s | %-28.28s | %-8d | NT$%-9d | %s\n",
           item->code,
           item->name,
           item->stock,
           item->price,
           activity);
}

static Bool comes_before_code(const Database *database,
                              int left_index,
                              int right_index) {
    int left_code;
    int right_code;

    left_code = atoi(database->items[left_index].code);
    right_code = atoi(database->items[right_index].code);
    return left_code <= right_code;
}

static Bool comes_before_revenue(const Database *database,
                                 int left_index,
                                 int right_index) {
    const Item *left;
    const Item *right;

    left = &database->items[left_index];
    right = &database->items[right_index];

    if (left->revenue != right->revenue) {
        return left->revenue > right->revenue;
    }

    return atoi(left->code) <= atoi(right->code);
}

static void merge_indices(const Database *database,
                          int indices[],
                          int work[],
                          int left,
                          int middle,
                          int right,
                          Bool by_revenue) {
    int i;
    int j;
    int k;
    Bool take_left;

    i = left;
    j = middle;
    k = left;

    while (i < middle && j < right) {
        if (by_revenue) {
            take_left = comes_before_revenue(database, indices[i], indices[j]);
        } else {
            take_left = comes_before_code(database, indices[i], indices[j]);
        }

        if (take_left) {
            work[k++] = indices[i++];
        } else {
            work[k++] = indices[j++];
        }
    }

    while (i < middle) {
        work[k++] = indices[i++];
    }

    while (j < right) {
        work[k++] = indices[j++];
    }

    for (k = left; k < right; k++) {
        indices[k] = work[k];
    }
}

static void merge_sort_indices(const Database *database,
                               int indices[],
                               int work[],
                               int left,
                               int right,
                               Bool by_revenue) {
    int middle;

    if (right - left <= 1) {
        return;
    }

    middle = left + (right - left) / 2;
    merge_sort_indices(database, indices, work, left, middle, by_revenue);
    merge_sort_indices(database, indices, work, middle, right, by_revenue);
    merge_indices(database, indices, work, left, middle, right, by_revenue);
}

void item_sort_indices_by_code(const Database *database,
                               int indices[],
                               int count) {
    int work[MAX_ITEMS];

    if (database == NULL || indices == NULL || count <= 1) {
        return;
    }

    merge_sort_indices(database, indices, work, 0, count, FALSE);
}

void item_sort_indices_by_revenue_desc(const Database *database,
                                       int indices[],
                                       int count) {
    int work[MAX_ITEMS];

    if (database == NULL || indices == NULL || count <= 1) {
        return;
    }

    merge_sort_indices(database, indices, work, 0, count, TRUE);
}
