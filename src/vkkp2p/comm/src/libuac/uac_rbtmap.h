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
		//前缀++it
		iterator& operator++(){
			ptr = rbtmap::next(ptr);
			return *this;
		}
		//后缀it++
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

	rb_node_t<_Kty,_Ty>* _remove_scnode(rb_node_t<_Kty,_Ty>* node); //删除最多只有一个子的节点,不释放内存,返回新的代替点
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
	//情况1:根结点,只将根结点染黑
	//情况2:父结点为黑色的不用处理.
	while((parent=node->parent) && parent->color==RB_RED)
	{
		assert(RB_RED==node->color);
		//父节点为红色的情况一定有祖父
		gparent = parent->parent;
		if(parent==gparent->left)
		{
			//父为左
			uncle = gparent->right;
			if(uncle && uncle->color==RB_RED)
			{
				//情况3:叔父节点为红色,将父与叔父染黑,祖父染红
				uncle->color = RB_BLACK;
				parent->color = RB_BLACK;
				gparent->color = RB_RED;
				node = gparent;
			}
			else
			{
				//叔父为黑
				if(parent->right==node)
				{
					//情况4: 父为左,我为右的情况,父左旋转(旋转前后的树黑路径不变
					_rotate_left(parent);
					node = parent;
					parent = node->parent;
				}
				//情况5:父为左,我为左,祖父右旋一次
				parent->color = RB_BLACK;
				gparent->color = RB_RED;
				_rotate_right(gparent);
				//至此结束
				break;
			}
		}
		else
		{
			//父为右的情况以上面对称
			uncle = gparent->left;
			if(uncle && uncle->color==RB_RED)
			{
				//情况3:叔父节点为红色,将父与叔父染黑,祖父染红
				uncle->color = RB_BLACK;
				parent->color = RB_BLACK;
				gparent->color = RB_RED;
				node = gparent;
			}
			else
			{
				//叔父为黑
				if(parent->left==node)
				{
					//情况4: 父为左,我为右的情况,父左旋转(旋转前后的树黑路径不变
					_rotate_right(parent);
					node = parent;
					parent = node->parent;
				}
				//情况5:父为左,我为左,祖父右旋一次
				parent->color = RB_BLACK;
				gparent->color = RB_RED;
				_rotate_left(gparent);
				//至此结束
				break;
			}
		}
	}
	m_root->color = RB_BLACK;
}

template<typename _Kty,typename _Ty>
int rbtmap<_Kty,_Ty>::insert(const _Kty& key,const _Ty& data)
{
	//如果节点已经存在，只更新值。
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
rb_node_t<_Kty,_Ty>* rbtmap<_Kty,_Ty>::_remove_scnode(rb_node_t<_Kty,_Ty>* node) //删除最多只有一个子的节点,不释放内存
{
	//返回新的代替点
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
		//一定是root
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

	//node: N, 父亲:P, 兄弟: S, 兄弟的左右孩子:SL,SR
	//情况:是根结点,或者是红色,染黑即可
	//情况1: node的颜色为黑色
	while(node!=m_root && (!node || node->color == RB_BLACK))
	{
		//node为左儿子
		//注意:在这里node->color为黑色,而原有树是平衡的,所以证明它一定存在非叶子兄弟
		if(node == parent->left)
		{
			other = parent->right;
			//情况2: 兄弟是红色,父新左旋一次,父亲与兄弟色交换,这样不改变之前的平衡状态,仅改变了兄弟的颜色
			if(other->color == RB_RED)
			{
				other->color = RB_BLACK;
				parent->color = RB_RED;
				_rotate_left(parent);
				other = parent->right;
			}
			//兄弟已经必然是黑色
	
			//情况3:兄弟的儿子都是黑色的情况下
			if ((!other->left || other->left->color == RB_BLACK) &&
                (!other->right || other->right->color == RB_BLACK))
            {
				//将兄弟染红,使N 与 兄弟平衡.回到 父亲点再做情况1处理,如果父亲为黑色将继续循环处理,如果父亲是红色,退出循环后被染黑
                other->color = RB_RED;
                node = parent;
				parent = node->parent;
				continue;
            }
			else
			{
				//兄弟的孩子不全为黑色
				//情况4：兄弟的左孩子为红色，做一次兄弟右旋转，将兄弟与兄弟的左孩子交换色，使得新兄弟有一个红色的右孩子，新兄弟左孩子必为黑色
				if (!other->right || other->right->color == RB_BLACK)
                {
					assert(other->left->color==RB_RED); //一定有一个非叶左孩子
                    if((o_left = other->left))
                    {
                        o_left->color = RB_BLACK;
                    }
                    other->color = RB_RED;
                    _rotate_right(other);
                    other = parent->right;
                }

				//情况5：兄弟有一个红色的右孩子，将父亲做右旋转，交换原父亲与兄弟的颜色,并将原兄弟右孩子改成黑色，达到平衡
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
			//node为右儿子
			other = parent->left;
			//情况2: 兄弟是红色,父新右旋一次,父亲与兄弟色交换,这样不改变之前的平衡状态,仅改变了兄弟的颜色
			if(other->color == RB_RED)
			{
				other->color = RB_BLACK;
				parent->color = RB_RED;
				_rotate_right(parent);
				other = parent->left;
			}
			//兄弟已经必然是黑色
	
			//情况3:兄弟的儿子都是黑色的情况下
			if ((!other->left || other->left->color == RB_BLACK) &&
                (!other->right || other->right->color == RB_BLACK))
            {
				//将兄弟染红,使N 与 兄弟平衡.回到 父亲点再做情况1处理,如果父亲为黑色将继续循环处理,如果父亲是红色,退出循环后被染黑
                other->color = RB_RED;
                node = parent;
				parent = node->parent;
				continue;
            }
			else
			{
				//兄弟的孩子不全为黑色
				//情况4：兄弟的右孩子为红色，做一次兄弟左旋转，将兄弟与兄弟的左孩子交换色，使得新兄弟有一个红色的左孩子，新兄弟右孩子必为黑色
				if (!other->left || other->left->color == RB_BLACK)
                {
					assert(other->right->color==RB_RED); //一定有一个非叶右孩子
                    if((o_right = other->right))
                    {
                        o_right->color = RB_BLACK;
                    }
                    other->color = RB_RED;
                    _rotate_left(other);
                    other = parent->left;
                }

				//情况5：兄弟有一个红色的左孩子，将父亲做左旋转，交换原父亲与兄弟的颜色,并将原兄弟左孩子改成黑色，达到平衡
                other->color = parent->color;
                parent->color = RB_BLACK; //因为原兄弟一定为黑色
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
		//有双孩子的情况
		rb_node_t<_Kty,_Ty> *tmp = node->right;
		while(tmp->left)
			tmp = tmp->left;

		color = tmp->color;
		parent = tmp->parent;
		child = _remove_scnode(tmp);

		////将tmp放到node的位置上
		//node->key = tmp->key;
		//node->data = tmp->data;
		//node = tmp;
		///////////////////////////////////
		//将tmp放到node位置中，考虑到erase(it++)的使用，执行后it找到了tmp，所以不能用上面的方法
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
		//单孩子的情况
		color = node->color;
		parent = node->parent;
		child = _remove_scnode(node);
	}
	delete node;
	if(color==RB_BLACK)
		_remove_rebalance(child,parent); //child 可能是NULL.所以里面不能直接使用child->parent
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

	//有右子树,找右子树中最左的
	if(node->right)
	{
		node = node->right;
		while(node->left) node = node->left;
		return node;
	}
	//无右子树情况
	//如果作为根或者是父亲的左孩子.返回父亲
	if(!node->parent || node == node->parent->left)
		return node->parent;
	
	//如果作为父亲的右孩子,寻找第一个左父亲
	while(node->parent)
	{
		if(node == node->parent->right)
			node = node->parent;
		else
			return node->parent;
	}
	//一直找到了root,说明没有更大的.
	return NULL;
}


///////////////////////////////////////////////////////////////test/////////////////////////////////////////////////
//以下的测试假定为 rbtmap<int,int>
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
		//如果根的左子树为空
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
		//右子树回朔到根测退出.
		if(b && tmp == node)
			break;
		//是根,没有右子树,退出
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
		//从根的右树回到根部,或者左子树回到根部,没有右子树.测退出
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
		//注意：如果此处内存分配失败，拷贝部分树将不平衡,所以后面要清理
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

