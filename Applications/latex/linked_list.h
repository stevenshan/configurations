#ifndef __LINKED_LIST__
#define __LINKED_LIST__

#include <stdbool.h>

#include "latex.h"

typedef struct node_t node;
struct node_t {
    int i;
    int id;
    char key[BUFFER_LEN];
    node *next;
    node *prev;
};

typedef struct linked_list_t {
    node *head;
    node *tail;
} linked_list;

linked_list *new_linked_list();
void destroy_linked_list(linked_list *L);
size_t linked_list_len(linked_list *L);

int add_node(linked_list *L, int i, int id, const char *key);
bool delete_node_id(linked_list *L, int id, node *result);

typedef void map_func_t(int, int, char*);
void map_list(linked_list *L, map_func_t *func);

bool id_in_linked_list(linked_list *L, int id);

typedef node reduce_func_t(node, node);
node reduce_list(linked_list *L, reduce_func_t *func, node base);

node *find_i(linked_list *L, int i);

#endif
