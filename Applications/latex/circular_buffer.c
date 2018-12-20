#include <string.h>

#include "circular_buffer.h"

circular_buffer new_circular_buffer() {
    circular_buffer result;

    result.start = 0;
    result.end = 0;
    result.cursor = -1;

    return result;
}

static size_t idx(int x) {
    int b = CMD_HISTORY_LEN;
    int m = x % b;
    if (m < 0) {
        m = (b < 0) ? m - b : m + b;
    }
    return m;
}

void add_to_cbuff(circular_buffer *cbuffer, const char *str) {
    if (cbuffer == NULL || str == NULL) {
        return;
    }

    strncpy(cbuffer->buffer[cbuffer->end], str, BUFFER_LEN);

    cbuffer->cursor = cbuffer->end;
    cbuffer->end = idx(cbuffer->end + 1);

    if (cbuffer->start == cbuffer->end) {
        cbuffer->start = idx(cbuffer->start + 1);
    }
}

bool increment_cbuff(circular_buffer *cbuffer) {
    if (cbuffer == NULL) {
        return false;
    }

    int new_position = idx(cbuffer->cursor + 1);
    if ((cbuffer->start < cbuffer->end && new_position < cbuffer->end) ||
            (cbuffer->start > cbuffer->end &&
             new_position != cbuffer->end)) {
        cbuffer->cursor = new_position;
        return true;
    }
    return false;
}

bool decrement_cbuff(circular_buffer *cbuffer) {
    if (cbuffer == NULL) {
        return false;
    }

    int new_position = idx(cbuffer->cursor - 1);
    if ((cbuffer->start < cbuffer->end && cbuffer->cursor > cbuffer->start) ||
            (cbuffer->start > cbuffer->end &&
             cbuffer->cursor != cbuffer->start)) {
        cbuffer->cursor = new_position;
        return true;
    }
    return false;
}

void reset_cbuff_cursor(circular_buffer *cbuffer) {
    cbuffer->cursor = idx(cbuffer->end - 1);
}

char *get_cbuff_val(circular_buffer *cbuffer) {
    if (cbuffer == NULL || cbuffer->cursor < 0) {
        return NULL;
    }

    return &(cbuffer->buffer[cbuffer->cursor][0]);
}

