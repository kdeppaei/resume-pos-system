#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "input.h"

static void discard_remaining_input(void) {
    int ch;

    while ((ch = getchar()) != '\n' && ch != EOF) {
    }
}

void input_trim(char *text) {
    char *start;
    size_t length;

    if (text == NULL) {
        return;
    }

    start = text;
    while (*start != '\0' && isspace((unsigned char)*start)) {
        start++;
    }

    if (start != text) {
        memmove(text, start, strlen(start) + 1);
    }

    length = strlen(text);
    while (length > 0 && isspace((unsigned char)text[length - 1])) {
        text[length - 1] = '\0';
        length--;
    }
}

Bool input_read_line(const char *prompt, char *buffer, size_t size) {
    size_t length;

    if (prompt != NULL) {
        printf("%s", prompt);
    }

    if (fgets(buffer, (int)size, stdin) == NULL) {
        return FALSE;
    }

    length = strlen(buffer);
    if (length > 0 && buffer[length - 1] == '\n') {
        buffer[length - 1] = '\0';
    } else {
        discard_remaining_input();
    }

    input_trim(buffer);
    return TRUE;
}

Bool input_parse_int(const char *text, int *value) {
    char *end_pointer;
    long parsed;

    if (text == NULL || text[0] == '\0' || value == NULL) {
        return FALSE;
    }

    errno = 0;
    parsed = strtol(text, &end_pointer, 10);

    while (isspace((unsigned char)*end_pointer)) {
        end_pointer++;
    }

    if (errno != 0 || *end_pointer != '\0' ||
        parsed < -2147483647L - 1L || parsed > 2147483647L) {
        return FALSE;
    }

    *value = (int)parsed;
    return TRUE;
}

int input_read_int_range(const char *prompt, int min_value, int max_value) {
    char input[128];
    int value;

    while (TRUE) {
        if (!input_read_line(prompt, input, sizeof(input))) {
            printf("\nInput ended. Program will exit.\n");
            exit(EXIT_SUCCESS);
        }

        if (input_parse_int(input, &value) &&
            value >= min_value && value <= max_value) {
            return value;
        }

        printf("Please enter an integer from %d to %d.\n",
               min_value, max_value);
    }
}

Bool input_confirm(const char *prompt) {
    char input[16];

    while (TRUE) {
        if (!input_read_line(prompt, input, sizeof(input))) {
            return FALSE;
        }

        if (input[0] == 'y' || input[0] == 'Y') {
            return TRUE;
        }

        if (input[0] == 'n' || input[0] == 'N') {
            return FALSE;
        }

        printf("Please enter Y or N.\n");
    }
}
