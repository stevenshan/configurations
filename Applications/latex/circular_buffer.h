#ifndef __CIRCULAR_BUFFER__
#define __CIRCULAR_BUFFER__

#include <stdlib.h>
#include <stdbool.h>

#include "latex.h"

typedef struct circular_buffer_t {
    char buffer[CMD_HISTORY_LEN][BUFFER_LEN];
    int start;
    int end;
    int cursor;
} circular_buffer;

circular_buffer new_circular_buffer();
void add_to_cbuff(circular_buffer *cbuffer, const char *str);

bool increment_cbuff(circular_buffer *cbuffer);
bool decrement_cbuff(circular_buffer *cbuffer);

void reset_cbuff_cursor(circular_buffer *cbuffer);

char *get_cbuff_val(circular_buffer *cbuffer);

#endif
