#ifndef DBMYSQL_H
#define DBMYSQL_H
#include <mysql.h>
#include <iostream>
#include <cstring>
#include <stdio.h>
using namespace std;

class DBMysql
{
public:
    DBMysql(const char* host,const char* user,const char* passwd,const char* db_name);
    ~DBMysql();

    int db_connect();
    void db_close();
    void db_insert(const char* sqlstr);
    MYSQL_ROW db_select(const char* sqlstr);
    void db_delete(const char* sqlstr);
private:
    MYSQL mysql;
    const char* host;
    const char* user;
    const char* passwd;
    const char* db_name;
};

#endif // DBMYSQL_H
