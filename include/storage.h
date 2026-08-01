#ifndef STORAGE_H
#define STORAGE_H

#include "pos_types.h"

void database_initialize(Database *database);
Bool database_validate(const Database *database);
Bool database_save(const Database *database);
void database_load(Database *database);

#endif
