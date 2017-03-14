//#include "DB.h"
//#include "Setting.h"
//
////******************************************
////没有连接数据库时，转换成gdk格式的字符串例子
////MYSQL g_gbk_mysql;
////g_gbk_mysql.charset = &my_charset_gbk_bin;
//
//
////=======================================================
//#define  DBIP          "localhost"
//#define  DBPORT        3306
//#define  DBNAME        "vkkstat_db"
//#define  DBUSER        "root"
//#define  DBPASS        "mysql"
//#define  UNIX_SOCKET   "/tmp/mysql.sock"
////#define  UNIT_SOCKET   NULL
////========================================================
//
//char* DB_escape_string(char *to,const char *from,int len)
//{
//	if(0 == to || 0 == from)
//		return NULL;
//	int reallen = (int)strlen(from); 
//	if(reallen == 0 || len < (2*reallen+1))
//		return NULL;
//
//	if(DBConnectorSngl::instance()->get_mysql())
//	{
//		mysql_real_escape_string(DBConnectorSngl::instance()->get_mysql(),to,from,reallen);
//	}
//	else
//	{
//		mysql_escape_string(to,from,reallen);
//	}
//	return to;
//}
//
//string DB_escape_string(const string& str)
//{
//	char *sz = new char[2*str.length()+1];
//	if(NULL==sz)
//		return "";
//	sz[0]='\0';
//	DB_escape_string(sz,str.c_str(),(int)(2*str.length()+1));
//	string strret=sz;
//	delete[] sz;
//	return strret;
//}
////******************************************
//
//
//
////**********************************************************************
//SqlQuery::SqlQuery(void)
//{
//	m_pres = NULL;
//}
//SqlQuery::~SqlQuery(void)
//{
//	free_result();
//}
//void SqlQuery::free_result()
//{
//	if(m_pres)
//	{
//		mysql_free_result(m_pres);
//		m_pres = NULL;
//	}
//}
//int SqlQuery::query(const char* sql,int* affected_rows/*=NULL*/)
//{
//	free_result();
//	MYSQL *pmysql = DBConnectorSngl::instance()->get_mysql();
//	if(NULL==pmysql)
//	{
//		printf("SqlQuery::query: mysql is not connecte! \n");
//		return -1;
//	}
//
//	if(0!=mysql_real_query(pmysql,sql,(unsigned long)strlen(sql)))
//	{
//		printf("mysql_real_query(%s) : [%d] [%s] \n",sql,mysql_errno(pmysql),mysql_error(pmysql));
//		return -2;
//	}
//	if(affected_rows)
//		*affected_rows = (int)mysql_affected_rows(pmysql);
//
//	//不论有没有结果返回，以下执行语句都不会错，也就是正常后面的mysql_errno(pmysql)==0;
//	m_pres = mysql_store_result(pmysql);
//
//	if(0!=mysql_errno(pmysql))
//	{
//		m_pres = NULL;
//		printf("mysql_store_result(%s) : [%d] [%s] \n",sql,mysql_errno(pmysql),mysql_error(pmysql));
//		return -3;
//	}
//	return 0;
//}
//int SqlQuery::next_row(char*** row,int& row_len)
//{
//	if(NULL==m_pres)
//		return -1;
//	row_len = mysql_num_fields(m_pres);
//	MYSQL_ROW  myrow;
//	myrow = mysql_fetch_row(m_pres);
//	if(myrow)
//	{
//		*row = myrow;
//		return 0;
//	}
//	return -1;
//}
////**********************************************************************
//DBConnector::DBConnector(void)
//: m_pmysql(NULL)
//{
//	//connect();
//}
//
//DBConnector::~DBConnector(void)
//{
//	disconnect();
//}
//int DBConnector::connect()
//{
//	if(SettingSngl::instance()->get_db_ip().empty())
//	{
//		printf("DBConnector::connect: db ip empty! \n");
//		return -1;
//	}
//	disconnect();
//	if(NULL==(m_pmysql=mysql_init(NULL)))
//	{
//		printf("mysql init() error\n");
//		return -1;
//	}
//	printf("---- connect to db: ip=%s,user=%s,password=%s,name=%s,port=%d,unix_socket=%s \n",
//		SettingSngl::instance()->get_db_ip().c_str(),
//		SettingSngl::instance()->get_db_user().c_str(),
//		SettingSngl::instance()->get_db_password().c_str(),
//		SettingSngl::instance()->get_db_name().c_str(),
//		SettingSngl::instance()->get_db_port(),
//		SettingSngl::instance()->get_db_unix_socket().empty()?NULL:SettingSngl::instance()->get_db_unix_socket().c_str());
//
//	if(NULL==mysql_real_connect(m_pmysql,
//		SettingSngl::instance()->get_db_ip().c_str(),
//		SettingSngl::instance()->get_db_user().c_str(),
//		SettingSngl::instance()->get_db_password().c_str(),
//		SettingSngl::instance()->get_db_name().c_str(),
//		SettingSngl::instance()->get_db_port(),
//		SettingSngl::instance()->get_db_unix_socket().empty()?NULL:SettingSngl::instance()->get_db_unix_socket().c_str(),
//		0))
//	{
//		printf("mysql_real_connect error: [%d] %s\n",mysql_errno(m_pmysql), mysql_error(m_pmysql));
//		disconnect();
//		return -2;
//	}
//	return 0;
//}
//int DBConnector::disconnect()
//{
//	if(m_pmysql)
//	{
//		mysql_close(m_pmysql);
//		m_pmysql = NULL;
//	}
//	return 0;
//}
//int DBConnector::ping()
//{
//	if(m_pmysql)
//	{
//		if(0!=mysql_ping(m_pmysql))
//			return connect();
//	}
//	else
//	{
//		connect();
//	}
//	return 0;
//}
