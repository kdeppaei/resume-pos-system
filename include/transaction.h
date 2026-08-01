#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "pos_types.h"

long long cart_calculate_total(const Database *database,
                               const CartLine cart[],
                               int cart_count);
void transaction_build(const Database *database,
                       Transaction *transaction,
                       const CartLine cart[],
                       int cart_count);
void transaction_append(Database *database,
                        const Transaction *transaction);
const Transaction *transaction_at(const Database *database,
                                  int logical_index);
const Transaction *transaction_find_by_id(const Database *database, int id);
void transaction_print_receipt(const Transaction *transaction);

#endif
