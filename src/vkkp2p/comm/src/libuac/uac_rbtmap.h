#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

namespace UAC
{
#define rbtmap map

enum{RB_RED = 0,RB_BLACK = 1};

template<typename _Kty,typename _Ty>
struct rb_node_t
{
	_Kty key;
	_Ty data;
	_Kty& first;
	_Ty& second;
	rb_node_t *left,*right,*parent;
	char color;
	
	rb_node_t(void) : first(key),second(data),left(NULL),right(NULL),parent(NULL),color(RB_RED) {}
};


template<typename _Kty,typename _Ty>
class rbtmap
{
public:
	rbtmap(void);
	~rbtmap(void);
public:
	rbtmap(const rbtmap<_Kty,_Ty>& _mp);
	bool _copy_tree(rb_node_t<_Kty,_Ty> *n,rb_node_t<_Kty,_Ty> *parent,bool bleft);
	rbtmap<_Kty,_Ty>& operator=(const rbtmap<_Kty,_Ty>& _mp);

public:
	class iterator
	{
	public:
		iterator():ptr(NULL){}
		~iterator(){}
	public:
		//ǰ׺++it
		iterator& operator++(){
			ptr = rbtmap::next(ptr);
			return *this;
		}
		//��׺it++
		iterator operator++(int){
			iterator it=*this;
			ptr = rbtmap::next(ptr);
			return it;
		}
		bool operator==(const iterator& it)
		{
			return (ptr==it.ptr);
		}
		bool operator!=(const iterator& it)
		{
			return (ptr!=it.ptr);
		}

		rb_node_t<_Kty,_Ty>* operator->()
		{
			return ptr;
		}

	public:
		rb_node_t<_Kty,_Ty> *ptr;
	};

private:
//public:
	rb_node_t<_Kty,_Ty> *m_root;
	unsigned int m_size;
	_Ty m_datatmp;

private:
	void _rotate_left(rb_node_t<_Kty,_Ty>* node);
	void _rotate_right(rb_node_t<_Kty,_Ty>* node);
	rb_node_t<_Kty,_Ty>* _search_auxiliary(const _Kty& key,rb_node_t<_Kty,_Ty>** pparent);

	rb_node_t<_Kty,_Ty>* _insert_node(const _Kty& key,const _Ty& data,rb_node_t<_Kty,_Ty>* parent);
	void _insert_rebalance(rb_node_t<_Kty,_Ty>* node);

	rb_node_t<_Kty,_Ty>* _remove_scnode(rb_node_t<_Kty,_Ty>* node); //ɾ�����ֻ��һ���ӵĽڵ�,���ͷ��ڴ�,�����µĴ����
	void _remove_rebalance(rb_node_t<_Kty,_Ty>* node,rb_node_t<_Kty,_Ty>* parent);
	int _remove_node(rb_node_t<_Kty,_Ty>* node); //

	void _clear(rb_node_t<_Kty,_Ty>* node);

public:
	unsigned int size() const {return m_size;}
	bool empty() const {return (0==m_size);}
	int insert(const _Kty& key,const _Ty& data);
	int remove(const _Kty& key);
	void clear();

	static rb_node_t<_Kty,_Ty>* next(rb_node_t<_Kty,_Ty>* node);
	iterator begin()
	{
		iterator it;
		rb_node_t<_Kty,_Ty>* node = m_root;
		while(node && node->left)
			node = node->left;
		it.ptr = node;
		return it;
	}
	iterator end()
	{
		iterator it;
		it.ptr = NULL;
		return it;
	}
	_Ty& operator[](const _Kty& key);

	iterator find(const _Kty& key)
	{
		iterator it;
		it.ptr = _search_auxiliary(key,NULL);
		return it;
	}

	void erase(const iterator& it)
	{
		_remove_node(it.ptr);
	}

public:
	static void rb_print(rb_node_t<_Kty,_Ty>* node);
	static void rb_traversal(rbtmap<_Kty,_Ty>& mp);
	static void rb_count(rb_node_t<_Kty,_Ty>* node);
};

template<typename _Kty,typename _Ty>
rbtmap<_Kty,_Ty>::rbtmap(void)
:m_root(NULL)
,m_size(0)
{
}
template<typename _Kty,typename _Ty>
rbtmap<_Kty,_Ty>::rbtmap(const rbtmap<_Kty,_Ty>& _mp)
:m_root(NULL)
,m_size(0)
{
	operator=(_mp);
}
template<typename _Kty,typename _Ty>
rbtmap<_Kty,_Ty>::~rbtmap(void)
{
	clear();
}

/*-----------------------------------------------------------
|   node           right
|   / \    ==>     / \
|   a  right     node  y
|       / \      / \
|       b  y    a   b
 -----------------------------------------------------------*/
template<typename _Kty,typename _Ty>
void rbtmap<_Kty,_Ty>::_rotate_left(rb_node_t<_Kty,_Ty>* node)
{
    rb_node_t<_Kty,_Ty>* right = node->right;

    if((node->right = right->left))
    {
        right->left->parent = node;
    }

    right->left = node;
    if((right->parent = node->parent))
    {
        if(node == node->parent->right)
        {
            node->parent->right = right;
        }
        else
        {
            node->parent->left = right;
        }
    }
    else
    {
        m_root = right;
    }
    node->parent = right;
}

/*-----------------------------------------------------------
|       node           left
|       / \            / \
|    left  y   ==>    a   node
|   / \                   / \
|  a   b                 b   y
-----------------------------------------------------------*/
template<typename _Kty,typename _Ty>
void rbtmap<_Kty,_Ty>::_rotate_right(rb_node_t<_Kty,_Ty>* node)
{
    rb_node_t<_Kty,_Ty>* left = node->left;

    if ((node->left = left->right))
    {
        left->right->parent = node;
    }

    left->right = node;
    if((left->parent = node->parent))
    {
        if(node == node->parent->right)
        {
            node->parent->right = left;
        }
        else
        {
            node->parent->left = left;
        }
    }
    else
    {
        m_root = left;
    }
    node->parent = left;
}

template<typename _Kty,typename _Ty>
rb_node_t<_Kty,_Ty>* rbtmap<_Kty,_Ty>::_search_auxiliary(const _Kty& key,rb_node_t<_Kty,_Ty>** pparent)
{
	rb_node_t<_Kty,_Ty> *node = m_root,*retnode = NULL,*parent = NULL;

	while(node)
	{
		if(key < node->key)
		{
			parent = node;
			node = node->left;
		}
		else if(node->key < key)
		{
			parent = node;
			node = node->right;
		}
		else
		{
			retnode = node;
			break;
		}
	}
	if(pparent)
		*pparent = parent;
	return retnode;
}

template<typename _Kty,typename _Ty>
rb_node_t<_Kty,_Ty>* rbtmap<_Kty,_Ty>::_insert_node(const _Kty& key,const _Ty& data,rb_node_t<_Kty,_Ty>* parent)
{
	rb_node_t<_Kty,_Ty> *node = NULL;
	node = new rb_node_t<_Kty,_Ty>();
	if(!node)
		return node;
	node->key = key;
	node->data = data;
	node->parent = parent;
	node->color = RB_RED;
	if(parent==NULL)
	{
		m_root = node;
	}else
	{
		if(parent->key<key)
		{
			assert(parent->right==NULL);
			parent->right = node;
		}
		else
		{
			assert(parent->left==NULL);
			parent->left = node;
		}
	}
	m_size++;
	return node;
}

template<typename _Kty,typename _Ty>
void rbtmap<_Kty,_Ty>::_insert_rebalance(rb_node_t<_Kty,_Ty>* node)
{
	rb_node_t<_Kty,_Ty> *parent,*gparent,*uncle;
	//���1:�����,ֻ�������Ⱦ��
	//���2:�����Ϊ��ɫ�Ĳ��ô���.
	while((parent=node->parent) && parent->color==RB_RED)
	{
		assert(RB_RED==node->color);
		//���ڵ�Ϊ��ɫ�����һ�����游
		gparent = parent->parent;
		if(parent==gparent->left)
		{
			//��Ϊ��
			uncle = gparent->right;
			if(uncle && uncle->color==RB_RED)
			{
				//���3:�常�ڵ�Ϊ��ɫ,�������常Ⱦ��,�游Ⱦ��
				uncle->color = RB_BLACK;
				parent->color = RB_BLACK;
				gparent->color = RB_RED;
				node = gparent;
			}
			else
			{
				//�常Ϊ��
				if(parent->right==node)
				{
					//���4: ��Ϊ��,��Ϊ�ҵ����,������ת(��תǰ�������·������
					_rotate_left(parent);
					node = parent;
					parent = node->parent;
				}
				//���5:��Ϊ��,��Ϊ��,�游����һ��
				parent->color = RB_BLACK;
				gparent->color = RB_RED;
				_rotate_right(gparent);
				//���˽���
				break;
			}
		}
		else
		{
			//��Ϊ�ҵ����������Գ�
			uncle = gparent->left;
			if(uncle && uncle->color==RB_RED)
			{
				//���3:�常�ڵ�Ϊ��ɫ,�������常Ⱦ��,�游Ⱦ��
				uncle->color = RB_BLACK;
				parent->color = RB_BLACK;
				gparent->color = RB_RED;
				node = gparent;
			}
			else
			{
				//�常Ϊ��
				if(parent->left==node)
				{
					//���4: ��Ϊ��,��Ϊ�ҵ����,������ת(��תǰ�������·������
					_rotate_right(parent);
					node = parent;
					parent = node->parent;
				}
				//���5:��Ϊ��,��Ϊ��,�游����һ��
				parent->color = RB_BLACK;
				gparent->color = RB_RED;
				_rotate_left(gparent);
				//���˽���
				break;
			}
		}
	}
	m_root->color = RB_BLACK;
}

template<typename _Kty,typename _Ty>
int rbtmap<_Kty,_Ty>::insert(const _Kty& key,const _Ty& data)
{
	//����ڵ��Ѿ����ڣ�ֻ����ֵ��
	rb_node_t<_Kty,_Ty> *parent = NULL;
	rb_node_t<_Kty,_Ty> *node = _search_auxiliary(key,&parent);
	if(node)
	{
		node->data = data;
		return 0;
	}
	node = _insert_node(key,data,parent);
	if(!node)
		return -1;
	_insert_rebalance(node);
	return 0;
}

template<typename _Kty,typename _Ty>
_Ty& rbtmap<_Kty,_Ty>::operator[](const _Kty& key)
{
	rb_node_t<_Kty,_Ty> *parent = NULL;
	rb_node_t<_Kty,_Ty> *node = _search_auxiliary(key,&parent);
	if(node)
	{
		return node->data;
	}
	node = _insert_node(key,m_datatmp,parent);
	if(!node)
		return m_datatmp;
	_insert_rebalance(node);
	return node->data;
}

template<typename _Kty,typename _Ty>
rb_node_t<_Kty,_Ty>* rbtmap<_Kty,_Ty>::_remove_scnode(rb_node_t<_Kty,_Ty>* node) //ɾ�����ֻ��һ���ӵĽڵ�,���ͷ��ڴ�
{
	//�����µĴ����
	assert(node);
	assert(!node->left || !node->right);
	if(!node)
		return NULL;

	rb_node_t<_Kty,_Ty> *child = NULL, *parent = node->parent;
	if(NULL==node->left)
		child = node->right;
	else
		child = node->left;
	if(child)
		child->parent = parent;
	if(parent)
	{
		if(node == parent->left)
			parent->left = child;
		else
			parent->right = child;
	}
	else
	{
		//һ����root
		assert(m_root==node);
		m_root = child;
	}
	m_size--;
	return child;
}

template<typename _Kty,typename _Ty>
void rbtmap<_Kty,_Ty>::_remove_rebalance(rb_node_t<_Kty,_Ty>* node,rb_node_t<_Kty,_Ty>* parent)
{
	rb_node_t<_Kty,_Ty> *other, *o_left, *o_right;

	//node: N, ����:P, �ֵ�: S, �ֵܵ����Һ���:SL,SR
	//���:�Ǹ����,�����Ǻ�ɫ,Ⱦ�ڼ���
	//���1: node����ɫΪ��ɫ
	while(node!=m_root && (!node || node->color == RB_BLACK))
	{
		//nodeΪ�����
		//ע��:������node->colorΪ��ɫ,��ԭ������ƽ���,����֤����һ�����ڷ�Ҷ���ֵ�
		if(node == parent->left)
		{
			other = parent->right;
			//���2: �ֵ��Ǻ�ɫ,��������һ��,�������ֵ�ɫ����,�������ı�֮ǰ��ƽ��״̬,���ı����ֵܵ���ɫ
			if(other->color == RB_RED)
			{
				other->color = RB_BLACK;
				parent->color = RB_RED;
				_rotate_left(parent);
				other = parent->right;
			}
			//�ֵ��Ѿ���Ȼ�Ǻ�ɫ
	
			//���3:�ֵܵĶ��Ӷ��Ǻ�ɫ�������
			if ((!other->left || other->left->color == RB_BLACK) &&
                (!other->right || other->right->color == RB_BLACK))
            {
				//���ֵ�Ⱦ��,ʹN �� �ֵ�ƽ��.�ص� ���׵��������1����,�������Ϊ��ɫ������ѭ������,��������Ǻ�ɫ,�˳�ѭ����Ⱦ��
                other->color = RB_RED;
                node = parent;
				parent = node->parent;
				continue;
            }
			else
			{
				//�ֵܵĺ��Ӳ�ȫΪ��ɫ
				//���4���ֵܵ�����Ϊ��ɫ����һ���ֵ�����ת�����ֵ����ֵܵ����ӽ���ɫ��ʹ�����ֵ���һ����ɫ���Һ��ӣ����ֵ����ӱ�Ϊ��ɫ
				if (!other->right || other->right->color == RB_BLACK)
                {
					assert(other->left->color==RB_RED); //һ����һ����Ҷ����
                    if((o_left = other->left))
                    {
                        o_left->color = RB_BLACK;
                    }
                    other->color = RB_RED;
                    _rotate_right(other);
                    other = parent->right;
                }

				//���5���ֵ���һ����ɫ���Һ��ӣ�������������ת������ԭ�������ֵܵ���ɫ,����ԭ�ֵ��Һ��Ӹĳɺ�ɫ���ﵽƽ��
                other->color = parent->color;
                parent->color = RB_BLACK;
				assert(other->right->color == RB_RED);
                if(other->right)
                {
                    other->right->color = RB_BLACK;
                }
                _rotate_left(parent);
                node = m_root;
                break;
			}
		}
		else
		{
			//nodeΪ�Ҷ���
			other = parent->left;
			//���2: �ֵ��Ǻ�ɫ,��������һ��,�������ֵ�ɫ����,�������ı�֮ǰ��ƽ��״̬,���ı����ֵܵ���ɫ
			if(other->color == RB_RED)
			{
				other->color = RB_BLACK;
				parent->color = RB_RED;
				_rotate_right(parent);
				other = parent->left;
			}
			//�ֵ��Ѿ���Ȼ�Ǻ�ɫ
	
			//���3:�ֵܵĶ��Ӷ��Ǻ�ɫ�������
			if ((!other->left || other->left->color == RB_BLACK) &&
                (!other->right || other->right->color == RB_BLACK))
            {
				//���ֵ�Ⱦ��,ʹN �� �ֵ�ƽ��.�ص� ���׵��������1����,�������Ϊ��ɫ������ѭ������,��������Ǻ�ɫ,�˳�ѭ����Ⱦ��
                other->color = RB_RED;
                node = parent;
				parent = node->parent;
				continue;
            }
			else
			{
				//�ֵܵĺ��Ӳ�ȫΪ��ɫ
				//���4���ֵܵ��Һ���Ϊ��ɫ����һ���ֵ�����ת�����ֵ����ֵܵ����ӽ���ɫ��ʹ�����ֵ���һ����ɫ�����ӣ����ֵ��Һ��ӱ�Ϊ��ɫ
				if (!other->left || other->left->color == RB_BLACK)
                {
					assert(other->right->color==RB_RED); //һ����һ����Ҷ�Һ���
                    if((o_right = other->right))
                    {
                        o_right->color = RB_BLACK;
                    }
                    other->color = RB_RED;
                    _rotate_left(other);
                    other = parent->left;
                }

				//���5���ֵ���һ����ɫ�����ӣ�������������ת������ԭ�������ֵܵ���ɫ,����ԭ�ֵ����Ӹĳɺ�ɫ���ﵽƽ��
                other->color = parent->color;
                parent->color = RB_BLACK; //��Ϊԭ�ֵ�һ��Ϊ��ɫ
				assert(other->left->color == RB_RED);
                if (other->left)
                {
                    other->left->color = RB_BLACK;
                }
                _rotate_right(parent);
                node = m_root;
                break;
			}
		}
	}
	if (node)
    {
        node->color = RB_BLACK;
	}
}
template<typename _Kty,typename _Ty>
int rbtmap<_Kty,_Ty>::_remove_node(rb_node_t<_Kty,_Ty>* node)
{
	if(!node)
		return 0;

	rb_node_t<_Kty,_Ty> *child = NULL,*parent;
	char color;
	if(node->left && node->right)
	{
		//��˫���ӵ����
		rb_node_t<_Kty,_Ty> *tmp = node->right;
		while(tmp->left)
			tmp = tmp->left;

		color = tmp->color;
		parent = tmp->parent;
		child = _remove_scnode(tmp);

		////��tmp�ŵ�node��λ����
		//node->key = tmp->key;
		//node->data = tmp->data;
		//node = tmp;
		///////////////////////////////////
		//��tmp�ŵ�nodeλ���У����ǵ�erase(it++)��ʹ�ã�ִ�к�it�ҵ���tmp�����Բ���������ķ���
		tmp->color = node->color;
		tmp->left = node->left;
		tmp->right = node->right;
		tmp->parent = node->parent;
		if(node->parent)
		{
			if(node==node->parent->left)
				node->parent->left = tmp;
			else
				node->parent->right = tmp;
		}
		if(tmp->left)
			tmp->left->parent = tmp;
		if(tmp->right)
			tmp->right->parent = tmp;
		if(node == m_root)
			m_root = tmp;
		if(node == parent)
			parent = tmp;
	}
	else
	{
		//�����ӵ����
		color = node->color;
		parent = node->parent;
		child = _remove_scnode(node);
	}
	delete node;
	if(color==RB_BLACK)
		_remove_rebalance(child,parent); //child ������NULL.�������治��ֱ��ʹ��child->parent
	return 0;
}
template<typename _Kty,typename _Ty>
int rbtmap<_Kty,_Ty>::remove(const _Kty& key)
{
	return _remove_node(_search_auxiliary(key,NULL));
}

template<typename _Kty,typename _Ty>
void rbtmap<_Kty,_Ty>::_clear(rb_node_t<_Kty,_Ty>* node)
{
	if(node)
	{
		_clear(node->left);
		_clear(node->right);
		delete node;
		m_size--;
	}
}
template<typename _Kty,typename _Ty>
void rbtmap<_Kty,_Ty>::clear()
{
	_clear(m_root);
	m_root = NULL;
	assert(m_size==0);
}

template<typename _Kty,typename _Ty>
rb_node_t<_Kty,_Ty>* rbtmap<_Kty,_Ty>::next(rb_node_t<_Kty,_Ty>* node)
{
	if(!node)
		return NULL;

	//��������,���������������
	if(node->right)
	{
		node = node->right;
		while(node->left) node = node->left;
		return node;
	}
	//�����������
	//�����Ϊ�������Ǹ��׵�����.���ظ���
	if(!node->parent || node == node->parent->left)
		return node->parent;
	
	//�����Ϊ���׵��Һ���,Ѱ�ҵ�һ������
	while(node->parent)
	{
		if(node == node->parent->right)
			node = node->parent;
		else
			return node->parent;
	}
	//һֱ�ҵ���root,˵��û�и����.
	return NULL;
}


///////////////////////////////////////////////////////////////test/////////////////////////////////////////////////
//���µĲ��Լٶ�Ϊ rbtmap<int,int>
//

template<typename _Kty,typename _Ty>
void rbtmap<_Kty,_Ty>::rb_print(rb_node_t<_Kty,_Ty>* node)
{
	if(NULL==node)
		return;
	rb_print(node->left);
	printf("%d  ",node->key);
	rb_print(node->right);
}
template<typename _Kty,typename _Ty>
void rbtmap<_Kty,_Ty>::rb_traversal(rbtmap<_Kty,_Ty>& mp)
{
	printf("Traversal: size=%d \n",mp.size());
	iterator it;
	int n = 0;
	for(it = mp.begin();it!=mp.end();++it)
	{
		printf("%d  ",it->first);
		n++;
	}
	printf("\n ---->%d \n",n);
}

template<typename _Kty,typename _Ty>
void rbtmap<_Kty,_Ty>::rb_count(rb_node_t<_Kty,_Ty>* node)
{
	if(NULL==node)
		return;
	int i=0,n=0;
	int arr[100];
	rb_node_t<_Kty,_Ty> *tmp = node ,*tmp2;
	n = 1;
	while(1)
	{
		while(tmp->left)
		{
			tmp = tmp->left;
			if(tmp->color==RB_BLACK) n++;
		}
		arr[i++] = n;
		//�������������Ϊ��
		if(tmp==node && tmp->left==NULL)
		{
			if(tmp->right==NULL)
				break;
			tmp = tmp->right;
			if(tmp->color==RB_BLACK) n++;
			continue;
		}
		bool b = false;
		while(tmp->parent && tmp == tmp->parent->right)
		{
			if(tmp->color==RB_BLACK) n--;
			tmp = tmp->parent;
			b = true;
		}
		//��������˷�������˳�.
		if(b && tmp == node)
			break;
		//�Ǹ�,û��������,�˳�
		if(tmp==node && !node->right)
			break;
		b = false;
		tmp2 = NULL;
		while(tmp->parent)
		{
			tmp2 = tmp;
			if(tmp->color == RB_BLACK) n--;
			tmp = tmp->parent;

			if(tmp->right && tmp->right!=tmp2)
			{
				tmp = tmp->right;
				if(tmp->color==RB_BLACK) n++;
				break;
			}
		}
		//�Ӹ��������ص�����,�����������ص�����,û��������.���˳�
		if(tmp==node && (tmp2==tmp->right || tmp->right==NULL))
			break;
	}
	printf("leaf_num=%d, depth:",i);
	for(int j=0;j<i;++j)
	{
		printf("%d ",arr[j]);
	}
	printf("\n");
}

template<typename _Kty,typename _Ty>
bool rbtmap<_Kty,_Ty>::_copy_tree(rb_node_t<_Kty,_Ty> *n,rb_node_t<_Kty,_Ty> *parent,bool bleft)
{
	if(!n)
		return true;
	
	rb_node_t<_Kty,_Ty> *node = NULL;
	node = new rb_node_t<_Kty,_Ty>();
	if(!node)
	{
		//ע�⣺����˴��ڴ����ʧ�ܣ���������������ƽ��,���Ժ���Ҫ����
		assert(0);
		return false;
	}
	node->key = n->key;
	node->data = n->data;
	node->color = n->color;
	node->parent = parent;
	if(parent==NULL)
	{
		m_root = node;
	}else
	{
		if(bleft)
		{
			assert(parent->left==NULL);
			parent->left = node;
		}
		else
		{
			assert(parent->right==NULL);
			parent->right = node;
		}
	}
	m_size++;
	if(false==_copy_tree(n->left,node,true))
		return false;
	if(false==_copy_tree(n->right,node,false))
		return false;
	return true;
}
template<typename _Kty,typename _Ty>
rbtmap<_Kty,_Ty>& rbtmap<_Kty,_Ty>::operator=(const rbtmap<_Kty,_Ty>& _mp)
{
	clear();
	if(false==_copy_tree(_mp.m_root,NULL,false))
		clear();
	else
		assert(m_size==_mp.m_size);
	return *this;
}

}

