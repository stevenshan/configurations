#include <stdlib.h>
#include <string.h>
#include "linked_list.h"

static void init_dummy_node(node *n) {
    n->i = -1;
    n->id = -1;
    n->next = NULL;
    n->prev = NULL;
    n->access_time = 0;

    for (size_t i = 0; i < BUFFER_LEN; i++) {
        n->key[i] = '\0';
    }
}

static void accessed(linked_list *L, node *n) {
    L->time += 1;
    n->access_time = L->time;
}

linked_list *new_linked_list() {
    char* temp = malloc(sizeof(linked_list) + sizeof(node) * 2);
    if (temp == NULL) {
        return NULL;
    }

    linked_list *result = malloc(sizeof(linked_list));

    if (result == NULL) {
        return NULL;
    }

    result->head = malloc(sizeof(node));

    if (result->head == NULL) {
        free(result);
        return NULL;
    }

    result->tail = malloc(sizeof(node));

    if (result->tail == NULL) {
        free(result->head);
        free(result);
        return NULL;
    }

    init_dummy_node(result->head);
    init_dummy_node(result->tail);

    result->head->next = result->tail;
    result->tail->prev = result->head;

    return result;
}

static node *free_get_next(node *iter) {
    if (iter != NULL) {
        node *result = iter->next;
        free(iter);
        return result;
    }
    return NULL;
}

void destroy_linked_list(linked_list *L) {
    if (L == NULL) {
        return;
    }

    for (node *iter = L->head; iter != NULL; iter = free_get_next(iter));

    free(L);
}

size_t linked_list_len(linked_list *L) {
    size_t len = 0;
    for (node *iter = L->head; iter != NULL; iter = iter->next) {
        len += 1;
    }
    return len - 2;
}

node *add_node(linked_list *L, int i, int id, const char *key) {
    if (L == NULL) {
        return NULL;
    }

    node *new_node = malloc(sizeof(node));
    if (new_node == NULL) {
        return NULL;
    }
    init_dummy_node(new_node);

    node *head = L->head;
    head->i = i;
    head->id = id;
    if (key != NULL) {
        strncpy(head->key, key, BUFFER_LEN);
    }
    // head->next should already be set
    head->prev = new_node;

    accessed(L, head);

    new_node->next = head;
    L->head = new_node;

    return head;
}

static void remove_node(node *n) {
    node *prev = n->prev;
    node *next = n->next;

    if (prev != NULL) {
        prev->next = next;
    }

    if (next != NULL) {
        next->prev = prev;
    }
}

static void copy_node(node *dest, const node *src) {
    if (dest == NULL || src == NULL) {
        return;
    }

    dest->i = src->i;
    dest->id = src->id;
    for (size_t i = 0; i < BUFFER_LEN; i++) {
        dest->key[i] = src->key[i];
    }
}

bool delete_node_id(linked_list *L, int id, node *result) {
    for (node *iter = L->head; iter != NULL; iter = iter->next) {
        if (iter->id == id) {
            remove_node(iter);
            free(iter);

            copy_node(result, iter);

            return true;
        }
    }
    return false;
}

void map_list(linked_list *L, map_func_f *func) {
    if (L == NULL) {
        return;
    }

    for (node *iter = L->head; iter != NULL; iter = iter->next) {
        if (iter->id != -1) {
            func(iter->i, iter->id, iter->key);
        }
    }
}

bool id_in_linked_list(linked_list *L, int id) {
    if (L == NULL) {
        return false;
    }

    for (node *iter = L->head; iter != NULL; iter = iter->next) {
        if (iter->id == id) {
            accessed(L, iter);
            return true;
        }
    }
    return false;
}

node reduce_list(linked_list *L, reduce_func_f *func, node base) {
    node acc = base;
    if (L == NULL) {
        return acc;
    }

    for (node *iter = L->head->next; iter != L->tail; iter = iter->next) {
        acc = func(*iter, acc);
    }

    return acc;
}

node *find_i(linked_list *L, int i) {
    for (node *iter = L->head; iter != NULL; iter = iter->next) {
        if (LL_I(iter->i) == i) {
            accessed(L, iter);
            return iter;
        }
    }
    return NULL;
}

node *find_id(linked_list *L, int id) {
    for (node *iter = L->head; iter != NULL; iter = iter->next) {
        if (iter->id == id) {
            accessed(L, iter);
            return iter;
        }
    }
    return NULL;
}

node *get_recently_accessed(linked_list *L) {
    node *most_recent = NULL;
    size_t time = 0;

    if (L == NULL) {
        return most_recent;
    }

    for (node *iter = L->head->next; iter != L->tail; iter = iter->next) {
        if (iter->access_time >= time) {
            time = iter->access_time;
            most_recent = iter;
        }
    }

    return most_recent;
}

