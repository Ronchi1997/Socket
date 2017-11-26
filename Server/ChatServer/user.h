#ifndef USER
#define USER
#include <arpa/inet.h>
#include <string>
using namespace std;

struct userInfo  //loginInfo or registerInfo
{
  char id_name[20]={0};
  char pwd[20]={0};
  char cpwd[20]={0};
  int flag;//1 for login or 2 for register
};

class User
{
    public:
        User(){}
        User(string id,string name,int csockfd,string ip);
        ~User();

        string id;
        string name;
        int csockfd;
        string ip;
};

#endif // USER
