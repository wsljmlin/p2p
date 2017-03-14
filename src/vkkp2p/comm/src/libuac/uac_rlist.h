#pragma once

namespace UAC
{

//�ο���linux�ں�Դ���д���ʹ�õ�������������� Simple doubly linked list implementation
//���б���һ����ѭ��-˫��-����:root doubly linked
//ʹ�ø�����ʱ�����ֱ�ͷû����

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
//__builtin_prefetch ����������������
//���ݵĵ�ַ 
//rw ������ʹ����ָ��Ԥץȡ������Ϊ��ִ�ж�����������ִ��д���� 
//locality ������ʹ����ָ����ʹ������֮������Ӧ�����ڻ����У�����Ӧ�����
//void __builtin_prefetch( const void *addr, int rw, int locality );
#ifdef _WIN32
inline void __builtin_prefetch( const void *addr, int rw, int locality ){}
#endif


//g++ �������ᾯ��ʹ��0��ַָ��
/*#define rlist_entry(ptr,type,member) \*/
/*	((type*)((char*)(ptr)-(char*)&((type*)0)->member))*/

#define rlist_entry(ptr,type,member) \
	((type*)((char*)(ptr)-(char*)&((type*)8)->member+8))


//*********************************
//��ͷ��βѭ��
#define rlist_for_each(pos, head)         \
	for(pos=(head)->next; pos!=(head); pos=pos->next) 

//Ԥȡ��Ĵ��������Ч��
#define rlist_for_each_prefetch(pos, head)         \
	for(pos=(head)->next, __builtin_prefetch(pos->next,0,1) ; pos!=(head); pos=pos->next, __builtin_prefetch(pos->next,0,1)) 

//֧��ѭ��������ɾ��pos������Ԥȡ��
#define rlist_for_each_safe(pos, n, head) \
	for(pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)


//*********************************
//��β��ͷѭ��
#define rlist_for_each_prev(pos, head) \
	for(pos = (head)->prev; pos != (head); pos = pos->prev)

#define rlist_for_each_prev_prefetch(pos, head) \
    for(pos = (head)->prev, __builtin_prefetch(pos->prev,0,1);  pos != (head); pos = pos->prev, __builtin_prefetch(pos->prev,0,1))

//֧��ѭ��������ɾ��pos������Ԥȡ��
#define rlist_for_each_prev_safe(pos, p, head) \
	for(pos = (head)->prev, p = pos->prev; pos != (head); pos = p, p = pos->prev)


}


