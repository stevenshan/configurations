#include <stdlib.h>
#include <string.h>
#include "linked_list.h"

static void init_dummy_node(node *n) {
    n->id = -1;
    n->key = NULL;
    n->value = NULL;
    n->next = NULL;
    n->prev = NULL;
}

linked_list *new_linked_list() {
    char* temp = malloc(sizeof(linked_list) + sizeof(node) * 2);
    if (temp == NULL) {
        return NULL;
    }

    linked_list *result = (linked_list*)temp;
    result->head = (node*)(temp + sizeof(linked_list));
    result->tail = (node*)(result->head + 1);

    init_dummy_node(result->head);
    init_dummy_node(result->tail);

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

    for (node *iter = L->head; iter != NULL; iter = free_get_next(iter)) {
        if (iter->key != NULL) {
            free(iter->key);
        }
        if (iter->value != NULL) {
            free(iter->value);
        }
    }

    free(L);
}

int add_node(linked_list *L, int id, char *key, void *value) {
    node *new_node = malloc(sizeof(node));
    if (new_node == NULL) {
        return 1;
    }
    init_dummy_node(new_node);

    node *head = L->head;
    head->id = id;
    head->key = key;
    head->value = value;
    head->prev = new_node;

    new_node->next = head;
    L->head = new_node;

    return 0;
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

void *delete_node_id(linked_list *L, int id) {
    for (node *iter = L->head; iter != NULL; iter = iter->next) {
        if (iter->id == id) {
            if (iter->key != NULL) {
                free(iter->key);
            }
            remove_node(iter);
            return iter->value;
        }
    }
    return NULL;
}

void *delete_node_key(linked_list *L, char *key) {
    if (key == NULL) {
        return NULL;
    }

    for (node *iter = L->head; iter != NULL; iter = iter->next) {
        if (iter->key != NULL && strcmp(key, iter->key) == 0) {
            if (iter->key != NULL) {
                free(iter->key);
            }
            remove_node(iter);
            return iter->value;
        }
    }
    return NULL;
}
