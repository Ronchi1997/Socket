#ifndef SERVER_H
#define SERVER_H
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "dbmysql.h"
#include "thread.h"
#include <iostream>
#include <cstring>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>

using namespace std;

class Server
{
public:
    Server();
    ~Server();

    int getip(char *ip);
    bool acceptClient();
    void closeServer();
    void myencrypt(char *pwd,char *cpwd); //使用异或加密对密码和口令进行加密
private:
    struct sockaddr_in addr;
    int sockfd;
    UserList* list;
    DBMysql* db;

    void init();//fail:sockfd is -1 success: sockfd
};
#endif // SERVER_H
