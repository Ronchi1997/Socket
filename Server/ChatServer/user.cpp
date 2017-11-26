#include "user.h"

User::User(string id, string name, int csockfd,string ip):
id(id),name(name),csockfd(csockfd),ip(ip)
{
    
}

User::~User()
{

}
