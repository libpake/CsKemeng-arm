/*  用于 各个 设备 链表 */

#ifndef _list_h_
#define _list_h_

struct list_head{
    struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name)     { &(name), &(name) }

#define LIST_HEAD(name) \
                struct list_head name = LIST_HEAD_INIT(name);

static inline void INIT_LIST_HEAD( struct list_head *list)
{
    list->next = list;
    list->prev = list;
}

static inline void __list_add(struct list_head *new,
                                                struct list_head *prev,
                                                struct list_head *next)
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

static inline void list_add( struct list_head *new, struct list_head *head)
{
    __list_add(new, head, head->next);
}

static inline void __list_del( struct list_head *prev, struct list_head *next)
{
    next->prev = prev;
    prev->next = next;
}
static inline void list_del(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
    entry->next = NULL;
    entry->prev = NULL;
}

#define offset_of(type, memb) \
	((unsigned long)(&((type *)0)->memb))

/**
 * list_entry - get the struct for this entry 
 * @obj: the &struct list_head point
 * @type: the type of struct this embedded in. 
 * @memb: the name of the list_struct within th struct  
 */
#define list_entry(obj, type, memb ) \
  ((type *)(((char *)obj) - offset_of(type, memb)))


/**
 * list_for_each  - 遍历所有list节点
 *  @pos:   用于遍历的变量指针
 *  @head:  链表的头针
 */ 
#define list_for_each(pos, head) \
    for( pos = (head)->next; pos!=(head); pos=pos->next) 

/**
 * list_for_each_prev   - 往前遍历list节点 
 * @pos:  用于遍历的变量指针; 
 * ＠head: 链表头指针 
 */
#define list_for_each_prev(pos, head ) \
    for( pos=(head)->prev; pos!=(head); pos=pos->prev) 

#endif
