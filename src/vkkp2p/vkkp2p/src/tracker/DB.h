//#pragma once
//#include "commons.h"
//#include "mysql.h"
//
//char* DB_escape_string(char *to,const char *from,int len);
//string DB_escape_string(const string& str); 
//
//class SqlQuery
//{
//public:
//	SqlQuery(void);
//	~SqlQuery(void);
//public:
//	int query(const char* sql,int* affected_rows=NULL);
//	int next_row(char*** row,int& row_len);
//private:
//	void free_result();
//private: 
//	MYSQL_RES *m_pres;
//};
//
//class DBConnector
//{
//public:
//	DBConnector(void);
//	~DBConnector(void);
//
//public:
//	int connect();
//	int disconnect();
//	int ping();
//	MYSQL *get_mysql() const {return m_pmysql;}
//
//private:
//	MYSQL *m_pmysql;
//};
//
//typedef Singleton<DBConnector> DBConnectorSngl;
//
