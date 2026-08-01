#ifndef CATALOG_H
#define CATALOG_H

#include "pos_types.h"

void catalog_add_product(Database *database);
void catalog_list_products(const Database *database);
void catalog_search_product(const Database *database);
void catalog_edit_product(Database *database);
void catalog_restock_product(Database *database);
void catalog_archive_product(Database *database);
void catalog_restore_product(Database *database);

#endif
