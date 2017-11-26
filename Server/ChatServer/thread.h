#ifndef THREAD_H
#define THREAD_H
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <dbmysql.h>
#include <sstream>
#include <string>
#include "userlist.h"
#include "msg.h"

struct OnlineInfo
{
    char id[10];
    int online;// 1 for online ,0 for offline
};

class Thread
{
public:
    Thread(User user,UserList* list);
    ~Thread();

    void start();
private:
    pthread_t thread;
    User m_user;
    UserList* m_list;
    DBMysql* db;
    
    static void* run(void* arg);
};

#endif // THREAD_H
