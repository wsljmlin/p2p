#include "ICXml.h"

#include <assert.h>



#define XML_HEAD "<?xml version=\"1.0\" encoding=\"gbk\" standalone=\"yes\"?>"


void xml_test()
{
	//ICXml xml;
	//int ret = 0;
	//const char* str;
	//ret = xml.load_file("test.xml");
	//XMLNode *node = xml.find_first_node(NULL);
	//node = xml.find_first_node("VideoInfo");
	//node = xml.find_first_node("VideoInfo/");
	//node = xml.find_first_node("VideoInfo/Files/File/Files");
	//node = xml.add_node("VideoInfo2/Files/File1/File_1");
	//node = xml.add_node("VideoInfo2/Files/File1/File_1");
	//node = xml.add_node("VideoInfo2/Files/File1/File_2","123");
	//node = xml.find_first_node("VideoInfo2/Files/File1/File_1");
	//node = xml.add_child(node,"File_1_1","12345");
	//node = xml.find_first_node("VideoInfo2/Files/File1/File_1");
	//node = xml.add_child(node,"File_1_1","4567778");
	//node->attri.set_attri("a1","123");
	//node->attri.set_attri("a2","456");
	//node->attri.set_attri("a3","789");
	//node->attri.set_attri("a2","3456");
	//str = node->attri.get_attri("a2");
	//ret = xml.save_file("test2.xml");
}

//****************************************************
//XMLAttri

void XMLAttri::clear()
{
	//释放所有节点
	ANode_t *n = NULL;
	ANode_t *ptr = head.next;
	while(ptr)
	{
		n = ptr->next;
		SAFEDELETE_ARRAY(ptr->name);
		SAFEDELETE_ARRAY(ptr->val);
		delete ptr;
		ptr = n;
	}
	head.next = NULL;
}
int XMLAttri::load_string(const char* szAttri)
{
	clear();
	if(NULL==szAttri)
		return 0;

	ANode_t *ptr,*n;
	const char *p = szAttri;
	const char *p2;
	int len;
	ptr = &head; 
	while(p[0]!='\0')
	{
		while(p[0]==' ') p++;
		if(p[0]=='\0') return 0;

		//name
		p2 = strchr(p+1,'=');
		if(NULL==p2) return 0;
		//p2为=
		n = new ANode_t();
		len = (int)(p2-p);
		n->name = new char[len+2];
		memcpy(n->name,p,len);
		n->name[len] = '\0';

		//val
		p = p2+1;
		if('\''==p[0]) 
		{
			p++;
			p2 = strchr(p,'\'');
		}
		else if('\"'==p[0])
		{
			p++;
			p2 = strchr(p,'\"');
		}
		else
		{
			p2 = strchr(p,' ');
		}
		if(NULL==p2)
		{
			p2 = p;
			while(*p2=='\0') p2++;
		}

		len = (int)(p2-p);
		n->val = new char[len+2];
		if(len>0)
			memcpy(n->val,p,len);
		n->val[len] = '\0';
		ptr->next = n;
		ptr = ptr->next;

		p = p2;
		if(*p!='\0') p++;
	}
	return 0;
}
const char* XMLAttri::get_attri(const char* name)
{
	ANode_t *ptr;
	assert(name);
	ptr = head.next;
	while(ptr)
	{
		if(0 == strcmp(ptr->name,name))
		{
			return ptr->val;
		}
		ptr = ptr->next;
	}
	return NULL;
}
int XMLAttri::set_attri(const char* name,const char* val)
{
	ANode_t *pre,*ptr,*node;
	assert(name&&val);
	pre = &head;
	ptr = pre->next;
	while(ptr)
	{
		if(0 == strcmp(ptr->name,name))
		{
			SAFEDELETE_ARRAY(ptr->val);
			ptr->val = new char[strlen(val)+2];
			strcpy(ptr->val,val);
			return 0;
		}
		pre = ptr;
		ptr = ptr->next;
	}
	node = new ANode_t();
	node->name = new char[strlen(name)+2];
	node->val = new char[strlen(val)+2];
	strcpy(node->name,name);
	strcpy(node->val,val);
	node->next = pre->next;
	pre->next = node;
	return 0;
}
int XMLAttri::size()
{
	int s = 4;
	ANode_t *ptr = head.next;
	while(ptr)
	{
		s += (int)strlen(ptr->name);
		s += (int)strlen(ptr->val);
		s += 4;
		ptr = ptr->next;
	}
	return s;
}

int XMLAttri::to_string(char* buf,int buflen)
{
	if(buflen<size())
		return -1;
	ANode_t *ptr = head.next;
	char str[1024];
	buf[0] = '\0';
	while(ptr)
	{
		sprintf(str," %s=\"%s\"",ptr->name,ptr->val);
		strcat(buf,str);
		ptr = ptr->next;
	}
	return 0;
}

//****************************************************
//XMLNode
void XMLNode::clear()
{
		SAFEDELETE_ARRAY(szTag)
		SAFEDELETE_ARRAY(szData)
		attri.clear();
}


//****************************************************
//ICXml
ICXml::ICXml(void)
{
	strcpy(m_szXMLHead,XML_HEAD);
}

ICXml::~ICXml(void)
{
	clear();
}

bool ICXml::string_english_only(const char* str)
{
	int len = (int)strlen(str);
	for(int i=0;i<len;++i)
	{
		if(str[i] & 0x80)
			return false;
	}
	return true;
}
void ICXml::clear()
{
	delete_node_root(m_head.pnext,true);
	m_head.pnext = NULL;
}
int ICXml::load_file(const char* path)
{
	clear();
	char *szXML = NULL;
	int size = 0;
	int readsize = 0;
	int ret = 0;

	if(NULL==path)
		return -1;
	FILE *fp = fopen(path,"rb+");
	if(!fp)
		return -1;
	fseek(fp,0,SEEK_END);
	size = ftell(fp);
	fseek(fp,0,SEEK_SET);
	if(size>0)
	{
		szXML = new char[size+2];
		if(szXML)
		{
			while(readsize<size)
			{
				ret = (int)fread(szXML+readsize,1,size-readsize,fp);
				if(ret<=0)
					break;
				readsize+=ret;
			}
		}
	}
	fclose(fp);
	if(!szXML)
		return -1;
	if(readsize!=size)
	{
		assert(0);
		delete[] szXML;
		return -1;
	}
	szXML[readsize] = '\0';
	ret = load_string(szXML);
	delete[] szXML;
	return ret;
}

int ICXml::load_string(const char* szXML)
{
	size_t size = strlen(szXML);
	if(NULL==szXML ||0==size)
		return -1;
	clear();

	const char *ptr = strstr(szXML,"?>");
	if(!ptr)
		return -1;
	ptr+=2;
	memcpy(m_szXMLHead,szXML,ptr-szXML);
	m_szXMLHead[ptr-szXML]='\0';
	const char *ptr1 = strchr(ptr,'<');
	if(!ptr1)
		return -1;
	return load_next(ptr1,szXML+size,&m_head);
}
int ICXml::set_xml_head(const char* head)
{
	if(!head)
		return -1;
	strcpy(m_szXMLHead,head);
	return 0;
}
XMLNode *ICXml::find_first_node(const char* tagPath,XMLNode* start/*=NULL*/)
{
	if(!tagPath||0==strlen(tagPath))
		return m_head.pnext;
	
	if(NULL==start)
		start = m_head.pnext;
	const char *p1,*p2;
	p1 = tagPath;
	p2 = strchr(p1,'/');
	if(p2)
		return find_node(p1,(int)(p2-p1),p2+1,start);
	else
		return find_node(p1,(int)strlen(p1),NULL,start);
}
const char* ICXml::get_node_data(const char* tagPath,XMLNode* start/*=NULL*/)
{
	XMLNode* node = find_first_node(tagPath,start);
	if(node)
		return node->get_data();
	return NULL;
}
XMLNode *ICXml::find_node(const char* tag,int tagLen,const char* childTag,XMLNode *node)
{
	if(0==tagLen)
		return NULL;
	XMLNode *p = NULL;
	while(node)
	{
		if(0==memcmp(tag,node->get_tag(),tagLen))
		{
			if(NULL==childTag || 0==strlen(childTag))
				return node;

			const char *p1,*p2;
			p1 = childTag;
			p2 = strchr(p1,'/');
			if(p2)
			{
				p = find_node(p1,(int)(p2-p1),p2+1,node->pchild);
				if(p) return p;
			}
			else
			{
				p = find_node(p1,(int)strlen(p1),NULL,node->pchild);
				if(p) return p;
			}
		}
		node = node->pnext;
	}
	return NULL;
}
int ICXml::delete_node(XMLNode *node)
{
	if(NULL==node)
		return -1;
	if(node->pparent)
		node->pparent->pchild = node->pnext;
	delete_node_root(node,false);
	return 0;
}
//**************************
//注意：load_next()与load_child()互相产生递归调用，如果XML过大，递归调用过深，会导致大量堆栈开销。VC环境0默认1M
//正常如果处理过大XML，建议“堆栈保留大小为10M”。
//**************************
int ICXml::load_next(const char* szMin,const char* szMax,XMLNode* head)
{
	const char *p1=NULL,*p2=NULL,*p4=NULL,*p5=NULL;
	char *szAttri;
	assert(*szMin=='<');
	if(*szMin!='<')
		return -1;
	const char* szStart = szMin;
	p1 = strchr(szStart,'>');
	p2 = strchr(szStart,' ');
	if(szStart+4 < szMax && 0==memcmp(szStart,"<!--",4))
	{
		p4 = szStart;
		p5 = strstr(p4+4,"-->");
		if(p5 && (szMax && p5>szMax))
			p5 = NULL;
		if(NULL==p5)
			p4 = NULL;
	}
	if(!p1 || p1>szMax)
		return -1;

	int n = 0;
	if(p2>p1) p2=NULL;
	bool isEmpty = false;
	XMLNode *node = new XMLNode();
	node->pparent = head->pparent;
	head->pnext = node;
	if(p4)
	{
		//是注释
		p4 = p4+4;//data
		node->szTag = new char[2];
		strcpy(node->szTag,"!");
		if(p5>p4)
		{
			int n = (int)(p5-p4);
			node->szData = new char[n+1];
			memcpy(node->szData,p4,n);
			node->szData[n] = '\0';
		}
		p1 = p5+3;  //p1指向添加下一个的起点
	}
	else
	{
		if(*(p1-1)=='/')
		{
			p1 -= 1;
			isEmpty = true; //无数据格式为:<tag .../>
			
		}
		if(NULL==p2) p2 = p1;//如果搜不到" ",则"/"之前都是tag
		n = (int)(p2-szStart);
		node->szTag = new char[n];
		memcpy(node->szTag,szStart+1,n-1);
		node->szTag[n-1] = '\0';
		if(p2!=p1)
		{
			//有属性
			n = (int)(p1-p2+1);
			szAttri = new char[n];
			memcpy(szAttri,p2,n-1);
			szAttri[n-1] = '\0';
			node->attri.load_string(szAttri);
			delete[] szAttri;
		}
		
		if(!isEmpty)
		{
			p1++; //此标签结构为<tag>...</tag>形式,p1指向<tag>的后一个字节
			if(NULL==(p2=find_tag_end(node->szTag,p1,szMax)) || 0!=load_child(p1,p2,node))
			{
				delete node;
				return -1;
			}
			p1 = p2+1;  //p1指向添加下一个的起点
		}
	}

	//尝试添加下一个
	p2 = strchr(p1,'<');
	if(p2)
	{
		if(!szMax || p2<szMax)
		{
			return load_next(p2,szMax,node);
		}
	}
	return 0;
}
int ICXml::load_child(const char* szMin,const char* szMax,XMLNode* head)
{
	//可能是叶结点，叶的话就只加数据，否则忽略数据
	//如果同时有数据和子节点，数据在前并有<![CDATA[]]>,则忽略子节点，否则忽略数据
	const char *p1=NULL,*p2=NULL,*p4=NULL,*p5=NULL;
	char *szAttri;
	p1 = strchr(szMin,'<');
	if(p1&&p1>=szMax)
		p1 = NULL;
	if(p1)
	{
		if(p1+9<szMax && 0==memcmp(p1,"<![CDATA[",9))
			p2 = p1;//数据
		else if(p1+4<szMax && 0==memcmp(p1,"<!--",4))
		{
			p4 = p1; //注释
			p5 = strstr(p4+4,"-->");
			if(p5 && p5>szMax)
				p5 = NULL;
			if(NULL==p5)
				p4 = NULL;
		}
	}
	
	if(!p1)
	{
		//纯数据,szMin指向数据起始位，szMax指向结束标签的起始位(即数据的后一位)
		int n = (int)(szMax-szMin);
		head->szData = new char[n+1];
		memcpy(head->szData,szMin,n);
		head->szData[n] = '\0';
		return 0;
	}
	if(p2)
	{
		//格式数据
		p1 = p2+9;
		p2 = strstr(p1,"]]>");
		if(!p2)
			return -1;
		//无数据
		if(p1==p2)
			return 0;
		head->szData = new char[p2-p1+2];
		if(!head->szData)
			return -1;
		memcpy(head->szData,p1,p2-p1);
		head->szData[p2-p1] = '\0';
		return 0;
	}
	
	
	XMLNode *node = new XMLNode();
	node->pparent = head;
	head->pchild = node;
	if(p4)
	{
		//是注释
		p4 = p4+4;//data
		node->szTag = new char[2];
		strcpy(node->szTag,"!");
		if(p5>p4)
		{
			int n = (int)(p5-p4);
			node->szData = new char[n+1];
			memcpy(node->szData,p4,n);
			node->szData[n] = '\0';
		}
		p1 = p5+3;  //p1指向添加下一个的起点
	}
	else
	{
		//有子节点
		const char* szStart = p1;
		p1 = strchr(szStart,'>'); 
		p2 = strchr(szStart,' ');
		if(!p1 || p1>szMax)
		{
			delete node;
			return -1;
		}
		int n = 0;
		if(p2>p1) p2=NULL;
		bool isEmpty = false;
		if(*(p1-1)=='/')
		{
			p1 -= 1;
			isEmpty = true;
		}
		if(NULL==p2) p2 = p1;//如果搜不到" ",则"/"之前都是tag
		n = (int)(p2-szStart);
		node->szTag = new char[n];
		memcpy(node->szTag,szStart+1,n-1);
		node->szTag[n-1] = '\0';
		if(p2!=p1)
		{
			//有属性
			n = (int)(p1-p2+1);
			szAttri = new char[n];
			memcpy(szAttri,p2,n-1);
			szAttri[n-1] = '\0';
			node->attri.load_string(szAttri);
			delete[] szAttri;
		}

		if(!isEmpty)
		{
			p1++; //此标签结构为<tag>...</tag>形式,p1指向<tag>的后一个字节
			if(NULL==(p2=find_tag_end(node->szTag,p1,szMax)) || 0!=load_child(p1,p2,node))
			{
				delete node;
				return -1;
			}
			p1 = p2+1;  //p1指向添加下一个的起点
		}
	}
	//尝试添加下一个
	p2 = strchr(p1,'<');
	if(p2)
	{
		if(!szMax || p2<szMax)
		{
			return load_next(p2,szMax,node);
		}
	}
	return 0;
}
const char* ICXml::find_tag_end(const char* tag,const char* szMin,const char* szMax)
{
	const char *p1,*p2,*p3;
	char *szBegin = new char[strlen(tag)+5];
	char *szEnd = new char[strlen(tag)+5];
	int err = 0;
	szMin--; //<tag>...</tag>形式,szMin原指向<tag>的后一位，现将它前移一位即指向">". 以便后面的循环算法.
	do
	{
		if(!szEnd ||!szBegin)
		{
			err = 1;
			break;
		}
		int beginLen = (int)strlen(tag)+1;
		sprintf(szBegin,"<%s",tag);
		sprintf(szEnd,"</%s>",tag);
		p1 = szMin;
		int n,m;
		while(1)
		{
			//找出最近的一个结束，然后数开始与结束之间的配对是否匹配
			p1 = strstr(p1+1,szEnd); 
			if(!p1||(szMax&&p1>szMax))
			{
				err = 2;
				break;
			}
			n = m = 0;
			p2 = szMin;
			//数开始
			while(1)
			{
				p2 = strstr(p2+1,szBegin);
				if(!p2||p2>=p1)
					break;
				p3 = p2+beginLen;
				if(*p3!=' '&& *p3!='/' && *p3!='>')
					continue;
				p3 = strchr(p2,'>');
				if(p3&& *(p3-1)=='/')
					continue;
				n++;
			}
			//数结束
			p2 = szMin;
			while(1)
			{
				p2 = strstr(p2+1,szEnd);
				if(!p2||p2>=p1)
					break;
				m++;
			}
			if(m==n)
				break;

		}
		if(0!=err)
			break;
		
		if(!p1||(szMax&&p1>szMax))
		{
			err = 3;
			break;
		}
		delete[] szBegin;
		delete[] szEnd;
		return p1;
	}while(0);
	if(0!=err)
	{
		if(szBegin)
			delete[] szBegin;
		if(szEnd)
			delete[] szEnd;
		return NULL;
	}
	return p1;
}
int ICXml::save_file(const char* path)
{
	int isize = size();
	if(!isize)
		return -1;
	char *szXML = new char[isize];
	if(!szXML)
		return -1;
	if(0!=to_string(szXML,isize))
		return -1;
	int size = (int)strlen(szXML);
	int writesize = 0;
	int ret = 0;
	if(size>0)
	{
		FILE *fp = fopen(path,"wb+");
		if(fp)
		{
			while(writesize<size)
			{
				ret = (int)fwrite(szXML+writesize,1,size-writesize,fp);
				if(ret<=0)
					break;
				writesize += ret;
			}
			fclose(fp);

		}
	}
	delete[] szXML;
	if(writesize<size)
		return -1;
	return 0;
}
int ICXml::size()
{
	if(!m_head.pnext)
		return 0;
	int isize = 0;
	isize = (int)strlen(m_szXMLHead) + 5;
	isize += size_count(m_head.pnext,0);
	return isize;
}
int ICXml::size_count(XMLNode* head,int step)
{
	if(!head)
		return 0;
	int size = 0;
	size += 3 + step;
	size += (int)strlen(head->szTag);
	if(!head->attri.empty())
		size += head->attri.size();
	if(!head->szData && !head->pchild)
	{
		size += 2;
	}
	else
	{
		size += 1;
		//如果有子节点，则忽略数据
		if(head->pchild)
		{
			size += size_count(head->pchild,step+1);
			size += 2 + step;
		}
		else if(head->szData)
		{
			size += 12;
			size += (int)strlen(head->szData);
		}

		size += 3;
		size += (int)strlen(head->szTag);
	}
	return (size + size_count(head->pnext,step));
}
int ICXml::to_string(char* szXML,int/* maxSize*/)
{
	if(!m_head.pnext)
		return -1;
	szXML[0] = '\0';
	strcat(szXML,m_szXMLHead);
	return format_string(szXML,m_head.pnext,0);
}
int ICXml::format_string(char* szXML,XMLNode* head,int step)
{
	if(!head)
		return 0;
	strcat(szXML,"\r\n");
	for(int i=0;i<step;++i)
		strcat(szXML,"	");
	strcat(szXML,"<");

	strcat(szXML,head->szTag);
	if(!head->attri.empty())
	{
		int len = head->attri.size();
		char *szAttri = new char[len];
		szAttri[0] = '\0';
		head->attri.to_string(szAttri,len);
		strcat(szXML,szAttri);
		delete[] szAttri;
	}
	if('!'==head->szTag[0])
	{
		//注释
		strcat(szXML,"--");
		strcat(szXML,head->szData);
		strcat(szXML,"-->");
	}
	else
	{
		if(!head->szData && !head->pchild)
		{
			strcat(szXML,"/>");
		}
		else
		{
			strcat(szXML,">");
			//如果有子节点，则忽略数据
			if(head->pchild)
			{
				format_string(szXML,head->pchild,step+1);
				strcat(szXML,"\r\n");
				for(int i=0;i<step;++i)
					strcat(szXML,"	");
			}
			else if(head->szData)
			{
				bool b = string_english_only(head->szData);
				if(!b) strcat(szXML,"<![CDATA[");
				strcat(szXML,head->szData);
				if(!b) strcat(szXML,"]]>");
			}

			strcat(szXML,"</");
			strcat(szXML,head->szTag);
			strcat(szXML,">");
		}
	}
	format_string(szXML,head->pnext,step);
	return 0;
}

XMLNode *ICXml::add_node(const char* path,const char* data/*=NULL*/)
{
	if(!path||0==strlen(path))
		return NULL;
	char buf[256];
	const char* p1,*p2,*p3;
	p1 = strrchr(path,'/');
	if(!p1) 
		p1 = path-1;
	if(0==strlen(p1+1))
		return NULL;
	p2 = path;
	XMLNode *pre = &m_head;
	XMLNode *parent = NULL;
	XMLNode *node = pre->pnext;
	while(p2)
	{
		p3 = strchr(p2,'/');
		if(!p3 || p3>p1)
			break;
		memcpy(buf,p2,p3-p2);
		buf[p3-p2] = '\0';
		p2 = p3+1;
		while(node)
		{
			parent = NULL;
			if(0==strcmp(node->szTag,buf))
				break;
			pre = node;
			node = node->pnext;
		}
		if(!node)
		{
			node = new XMLNode();
			if(!node)
				return NULL;
			node->szTag = new char[strlen(buf)+1];
			if(node->szTag)
				strcpy(node->szTag,buf);
			else
			{
				delete node;
				return NULL;
			}
			if(pre)
				pre->pnext = node;
			else
			{
				assert(parent);
				parent->pchild = node;
				node->pparent = parent;
			}
		}
		pre = NULL;
		parent = node;
		node = node->pchild;
	}
	p1++;
	if(node)
	{
		parent = NULL;
		while(node)
		{
			pre = node;
			node = node->pnext;
		}
	}
	node = new XMLNode();
	if(!node)
		return NULL;
	node->szTag = new char[strlen(p1)+1];
	if(node->szTag)
		strcpy(node->szTag,p1);
	else
	{
		delete node;
		return NULL;
	}
	if(data)
	{
		node->szData = new char[strlen(data)+1];
		if(node->szData)
			strcpy(node->szData,data);
	}
	if(pre)
	{
		assert(!parent);
		pre->pnext = node;
	}
	else
	{
		assert(parent);
		parent->pchild = node;
		node->pparent = parent;
	}
	return node;
}

XMLNode *ICXml::add_child(XMLNode *node,const char* tag,const char* data/*=NULL*/)
{
	if(!node||!tag||0==strlen(tag))
		return NULL;
	XMLNode *child = new XMLNode();
	if(!child)
		return NULL;
	child->szTag = new char[strlen(tag)+1];
	if(!child->szTag)
	{
		delete child;
		return NULL;
	}
	strcpy(child->szTag,tag);
	if(data && strlen(data))
	{
		child->szData = new char[strlen(data)+1];
		if(child->szData)
			strcpy(child->szData,data);
	}

	if(!node->pchild)
	{
		node->pchild = child;
		child->pparent = node;
	}
	else
	{
		node = node->pchild;
		while(node->pnext) node = node->pnext;
		node->pnext = child;
	}
	return child;
}

void ICXml::delete_node_root(XMLNode* head,bool delnext)
{
	if(head)
	{
		delete_node_root(head->pchild,true);
		if(delnext)
			delete_node_root(head->pnext,true);
		delete head;
	}
}
