#pragma once

namespace UAC
{

//参考自linux内核源码中大量使用的缩主型链表设计 Simple doubly linked list implementation
//该列表是一个“循环-双向-链表”:root doubly linked
//使用该链表时，保持表头没缩主

typedef struct rlist_head 
{
	struct rlist_head *next,*prev;
}rlist_head_t;

inline void __rlist_join(rlist_head_t *prev,rlist_head_t *next)
{
	prev->next = next;
	next->prev = prev;
}

inline void rlist_init(rlist_head_t *head)
{
	__rlist_join(head,head);
}
inline bool rlist_empty(rlist_head_t *head)
{
	return head->next == head;
}

inline void rlist_add_head(rlist_head_t *a,rlist_head_t *head)
{
	__rlist_join(a,head->next);
	__rlist_join(head,a);
}
inline void rlist_add_tail(rlist_head_t *a,rlist_head_t *head)
{
	__rlist_join(head->prev,a);
	__rlist_join(a,head);
}
inline void rlist_del(rlist_head_t *entry)
{
	__rlist_join(entry->prev,entry->next);
	entry->prev = 0;
	entry->next = 0;
}
//__builtin_prefetch 函数接收三个参数
//数据的地址 
//rw 参数，使用它指明预抓取数据是为了执行读操作，还是执行写操作 
//locality 参数，使用它指定在使用数据之后数据应该留在缓存中，还是应该清除
//void __builtin_prefetch( const void *addr, int rw, int locality );
#ifdef _WIN32
inline void __builtin_prefetch( const void *addr, int rw, int locality ){}
#endif


//g++ 编译器会警告使用0地址指针
/*#define rlist_entry(ptr,type,member) \*/
/*	((type*)((char*)(ptr)-(char*)&((type*)0)->member))*/

#define rlist_entry(ptr,type,member) \
	((type*)((char*)(ptr)-(char*)&((type*)8)->member+8))


//*********************************
//从头到尾循环
#define rlist_for_each(pos, head)         \
	for(pos=(head)->next; pos!=(head); pos=pos->next) 

//预取入寄存器可提高效率
#define rlist_for_each_prefetch(pos, head)         \
	for(pos=(head)->next, __builtin_prefetch(pos->next,0,1) ; pos!=(head); pos=pos->next, __builtin_prefetch(pos->next,0,1)) 

//支持循环过程中删除pos，不能预取入
#define rlist_for_each_safe(pos, n, head) \
	for(pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)


//*********************************
//从尾到头循环
#define rlist_for_each_prev(pos, head) \
	for(pos = (head)->prev; pos != (head); pos = pos->prev)

#define rlist_for_each_prev_prefetch(pos, head) \
    for(pos = (head)->prev, __builtin_prefetch(pos->prev,0,1);  pos != (head); pos = pos->prev, __builtin_prefetch(pos->prev,0,1))

//支持循环过程中删除pos，不能预取入
#define rlist_for_each_prev_safe(pos, p, head) \
	for(pos = (head)->prev, p = pos->prev; pos != (head); pos = p, p = pos->prev)


}


