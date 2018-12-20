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

    size_t access_time;
};

typedef struct linked_list_t {
    node *head;
    node *tail;

    size_t time;
} linked_list;

linked_list *new_linked_list();
void destroy_linked_list(linked_list *L);
size_t linked_list_len(linked_list *L);

int add_node(linked_list *L, int i, int id, const char *key);
bool delete_node_id(linked_list *L, int id, node *result);

typedef void map_func_f(int, int, char*);
void map_list(linked_list *L, map_func_f *func);

bool id_in_linked_list(linked_list *L, int id);

typedef node reduce_func_f(node, node);
node reduce_list(linked_list *L, reduce_func_f *func, node base);

node *find_i(linked_list *L, int i);
node *find_id(linked_list *L, int id);

node *get_recently_accessed(linked_list *L);

#endif
