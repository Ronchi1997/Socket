#ifndef USERLIST_H
#define USERLIST_H
#include <string>
#include <iostream>
#include "user.h"

using namespace std;

struct Node
{
    User user;
    Node* next;
};

class UserList
{
public:
    UserList();
    ~UserList();

    void push(User user);
    void pop(int csockfd);
    int ifhave(User user);
    int length();
    string selectIP(const char *id);
    void show();//ceshi
    Node* & get_head_node();

private:
    Node* head;
    int m_length;


};

#endif // USERLIST_H
