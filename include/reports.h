#ifndef REPORTS_H
#define REPORTS_H

#include "pos_types.h"

void reports_sales_summary(const Database *database);
void reports_low_stock(const Database *database);
void reports_transaction_history(const Database *database);
void reports_export_csv(const Database *database);

#endif
