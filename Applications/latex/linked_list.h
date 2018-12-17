#ifndef __LINKED_LIST__
#define __LINKED_LIST__

typedef struct node_t node;
struct node_t {
    int id;
    char *key;
    void *value;
    node *next;
    node *prev;
};

typedef struct linked_list_t {
    node *head;
    node *tail;
} linked_list;

linked_list *new_linked_list();
void destroy_linked_list(linked_list *L);

int add_node(linked_list *L, int id, char *key, void *value);
void *delete_node_id(linked_list *L, int id);
void *delete_node_key(linked_list *L, char *key);

#endif
