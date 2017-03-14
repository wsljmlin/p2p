#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#ifdef _WIN32
#pragma warning(disable:4996)
//windows 版本，如果XML过大，递归调用过深，会导致大量堆栈开销。VC环境0默认1M,正常如果处理过大XML，建议“堆栈保留大小为10M”。
#endif

#define SAFEDELETE_ARRAY(a) if(a) {delete[] a;a=NULL;}

class ICXml;


class XMLAttri
{
	friend class XMLNode;
private:
	typedef struct tagANode
	{
		char* name;
		char* val;
		tagANode *next;
		tagANode(void):name(NULL),val(NULL),next(NULL){}
	}ANode_t;

	XMLAttri(void)
	{
	}

	~XMLAttri(void)
	{
		clear();
	}
	void clear();
public:
	bool empty() const {return NULL==head.next;}
	int load_string(const char* szAttri);
	const char* get_attri(const char* name);
	int set_attri(const char* name,const char* val);
	int size();
	int to_string(char* buf,int buflen);
private:
	ANode_t head;
};

class XMLNode
{
	friend class ICXml;
private:
	XMLNode(void):szTag(NULL),szData(NULL),pparent(NULL),pchild(NULL),pnext(NULL) {}
	~XMLNode(void){
		//以下因为ecos执行delete时 将以下代码内联到其它函数中，可能导致代码栈溢出。所以放在独立函数中
		//SAFEDELETE_ARRAY(szTag)
		//SAFEDELETE_ARRAY(szData)
		clear();
	}
public:
	void clear();
	const char* get_tag(){return szTag;}
	const char* get_data(){return szData;};
	int set_data(const char* data)
	{
		char *ptr = NULL;
		if(data)
		{
			ptr = new char[strlen(data)+1];
			if(!ptr)
				return -1;
			strcpy(ptr,data);
		}
		SAFEDELETE_ARRAY(szData)
		szData = ptr;
		return 0;
	}

	XMLNode* next(){return pnext;}
	XMLNode* parent(){return pparent;}
	XMLNode* child() {return pchild;}

public:
	XMLAttri attri;
private:
	char *szTag;
	char *szData;
	XMLNode *pparent,*pchild,*pnext;
};

class ICXml
{
public:
	ICXml(void);
	~ICXml(void);
	static bool string_english_only(const char* str);
	void clear();
	int load_file(const char* path);
	int load_string(const char* szXML);
	int save_file(const char* path);
	int size();
	int to_string(char* szXML,int maxSize);
	int set_xml_head(const char* head);
	XMLNode *find_first_node(const char* tagPath,XMLNode* start=NULL);
	const char* get_node_data(const char* tagPath,XMLNode* start=NULL);
	int delete_node(XMLNode *node);
	XMLNode *add_node(const char* path,const char* data=NULL);
	XMLNode *add_child(XMLNode *node,const char* tag,const char* data=NULL);
private:
	void delete_node_root(XMLNode* head,bool delnext);
	int load_next(const char* szMin,const char* szMax,XMLNode* head);
	int load_child(const char* szMin,const char* szMax,XMLNode* head);
	const char* find_tag_end(const char* tag,const char* szMin,const char* szMax);
	int size_count(XMLNode* head,int step);
	int format_string(char* szXML,XMLNode* head,int step);
	XMLNode *find_node(const char* tag,int tagLen,const char* childTag,XMLNode *node);
private:
	XMLNode m_head;
	char m_szXMLHead[256];
};
void xml_test();
