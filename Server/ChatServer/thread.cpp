#include "thread.h"

Thread::Thread(User user, UserList *list):
    m_user(user),m_list(list)
{
    db = new DBMysql("127.0.0.1","root","0825","IM");
    db->db_connect();
}

Thread::~Thread()
{
    delete db;
}

void Thread::start()
{
    cout << "starting thread..." <<endl;

    int ret = pthread_create(&thread,NULL,run,this);
    if(0!= ret)
        cout << "create thread is fail" << endl;
}

void* Thread::run(void* arg)
{
    cout << "thread is started" << endl;
    Thread* th = (Thread*)(arg);
    //向客户端发送离线消息
    string sqlstr("select * from msg where toid = '");
    sqlstr.append(th->m_user.id).append("'");
    char** row = th->db->db_select(sqlstr.c_str());
    Msg off_msg;
    if(row == NULL)
        off_msg.to_user_id = 0;
    else
    {
        off_msg.to_user_id = atoi(row[0]);
        off_msg.from_user_id = atoi(row[1]);
        strcpy(off_msg.from_user_name,row[2]);
        strcpy(off_msg.msg,row[3]);
    }
    send(th->m_user.csockfd,&off_msg,sizeof(off_msg),0);
    
    userInfo uf;
    OnlineInfo of;
    
    while(1)
    {
        ssize_t re = recv(th->m_user.csockfd, &uf.id_name, sizeof(uf.id_name), 0);
        if(-1 == re || 0 == re)
        {
            th->m_list->pop(th->m_user.csockfd);
            cout << "user " << th->m_user.id << ":" << th->m_user.name << " offline" << endl;
            close(th->m_user.csockfd);
            break;
        }
        string sqlstr2("select * from user where name = '");
        sqlstr2.append(uf.id_name).append("'");
        char** row = th->db->db_select(sqlstr2.c_str());
        if (row == NULL)
        {
            of.online = 2; //错误信息
            send(th->m_user.csockfd, &of, sizeof(of), 0);
            continue;
        }
        else
        {
            strcpy(of.id ,row[0]);
            User user(of.id,uf.id_name,th->m_user.csockfd,th->m_user.ip);
            if(th->m_list->ifhave(user))
                of.online = 1;
            else
                of.online = 0;
            send(th->m_user.csockfd, &of, sizeof(of), 0);
            if(0 == of.online)  //离线消息由服务器托管
            {
                Msg msg;
                while(1)
                {
                    ssize_t ret = recv(th->m_user.csockfd,&msg,sizeof(msg),0);
                    if(-1 == ret || 0 == ret)
                    {
                        th->m_list->pop(th->m_user.csockfd);
                        cout << "user " << th->m_user.id << ":" << th->m_user.name << " offline" << endl;
                        close(th->m_user.csockfd);
                        break;
                    }
                    if(msg.to_user_id == 0)
                        break;
                    strcpy(msg.from_user_name,th->m_user.name.c_str());
                    string str1,str2;
                    stringstream ss;
                    ss << msg.to_user_id;
                    ss >> str1;
                    ss << msg.from_user_id;
                    ss >> str2;
                    string sqlstr3("insert into msg (toid,fromid,fromname,msg) values('");
                    sqlstr3.append(str1.c_str()).append("','").append(str2.c_str()).append("','").append(msg.from_user_name).append("','").append(msg.msg).append("')");
                    th->db->db_insert(sqlstr3.c_str());
                }
            }
            else //在线点对点聊天将IP和昵称发给相应客户端
            {
                char ip[16],name[20];
                strcpy(ip,th->m_list->selectIP(of.id).c_str());
                strcpy(name,th->m_user.name.c_str());
                send(th->m_user.csockfd, &(ip), sizeof(ip), 0);
                send(th->m_user.csockfd, &(name),sizeof(name),0);
            }
        }
    }
    cout << "thread is die" << endl;
    return NULL;
}
