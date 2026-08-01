#ifndef INPUT_H
#define INPUT_H

#include <stddef.h>
#include "pos_types.h"

void input_trim(char *text);
Bool input_read_line(const char *prompt, char *buffer, size_t size);
Bool input_parse_int(const char *text, int *value);
int input_read_int_range(const char *prompt, int min_value, int max_value);
Bool input_confirm(const char *prompt);

#endif
