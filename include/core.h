#ifndef CORE_H
#define CORE_H

#include <stddef.h>
#include "pos_types.h"

void pos_copy_text(char *destination, size_t destination_size,
                   const char *source);
Bool pos_is_all_digits(const char *text);
Bool pos_equals_ignore_case(const char *left, const char *right);
Bool pos_contains_ignore_case(const char *text, const char *pattern);

Bool item_is_occupied(const Database *database, int index);
Bool item_is_active(const Database *database, int index);
int item_find_empty_slot(const Database *database);
int item_find_by_code_number(const Database *database,
                             int code_number,
                             Bool active_only);
int item_find_by_exact_name(const Database *database,
                            const char *name,
                            Bool active_only,
                            int excluded_index);
int item_find_active_by_key(const Database *database, const char *key);
int item_find_archived_by_key(const Database *database, const char *key);
int item_free_quantity(const Item *item, int paid_quantity);
void item_format_activity(const Item *item, char *buffer, size_t buffer_size);
void item_print_details(const Item *item);
void item_print_table_header(void);
void item_print_table_row(const Item *item);

void item_sort_indices_by_code(const Database *database,
                               int indices[],
                               int count);
void item_sort_indices_by_revenue_desc(const Database *database,
                                       int indices[],
                                       int count);

#endif
