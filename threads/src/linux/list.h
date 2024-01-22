#ifndef LIST_H
#define LIST_H

struct list_head
{
    struct list_head *next;
    struct list_head *prev;
};

void list_init(struct list_head *list);
void list_add_tail(struct list_head *node, struct list_head *list);
void list_del(struct list_head *node);
int list_empty(const struct list_head *list);

#endif