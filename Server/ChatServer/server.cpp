#include "server.h"

Server::Server()
{
    db = new DBMysql("127.0.0.1","root","0825","IM");
    db->db_connect();
    init();
    list = new UserList();
}

Server::~Server()
{
    delete db;
}

//获取服务器IP
int Server::getip(char *ip)
{
    int sd;
    struct sockaddr_in sin;
    struct ifreq ifr;
    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == sd)
    {
        printf("socket error: %s\n", strerror(errno));
        return -1;
    }
    strncpy(ifr.ifr_name, "en0" , IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;
    
    // if error: No such device
    if (ioctl(sd, SIOCGIFADDR, &ifr) < 0)
    {
        printf("ioctl error: %s\n", strerror(errno));
        close(sd);
        return -1;
    }
    
    memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
    strcpy(ip, inet_ntoa(sin.sin_addr));
    close(sd);
    return 0;
}

//初始化服务器到监听状态
void Server::init()
{
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8888);
    addr.sin_addr.s_addr = INADDR_ANY;

    sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(-1 == sockfd)
    {
        return;
    }

    int ret = ::bind(sockfd,(struct sockaddr*)&addr,sizeof(addr));
    if(-1 == ret)
    {
        close(sockfd);
        sockfd = -1;
        return;
    }
    ret = listen(sockfd,1024);
    if(-1 == ret)
    {
        close(sockfd);
        sockfd = -1;
        return;
    }
    char ip[16];
    if(-1 == getip(ip))
    {
        close(sockfd);
        sockfd = -1;
        return;
    }
    cout <<"My IP is " << ip << endl;
    cout <<"开始监听……"<< endl;
}

//接受客户端的请求，如果一个用户的用户名和密码都正确才算是连接上服务器。否则就删除已连接套接字描述符
bool Server::acceptClient()
{
    if(-1 == sockfd)
        return false;
    struct sockaddr_in caddr;
    socklen_t len = sizeof(caddr);
    
    int csockfd = accept(sockfd,(sockaddr*)&caddr, &len);
    
    char* lpszStr = inet_ntoa(caddr.sin_addr);
    cout << "accept a connection from " << lpszStr << endl;
    
    if(-1 == csockfd)
    {
        return false;
    }
    userInfo lf;
	
    //接收客户端
    ssize_t size = recv(csockfd,(void*)&lf,sizeof(lf),0);
    if(-1 == size || 0 == size)
    {
        close(csockfd);
        return false;
    }
    if(1 == lf.flag)//login
    {
        //判断用户名和密码是否正确，实际应用从数据库查询
        string sqlstr("select * from user where id = '");
        sqlstr.append(lf.id_name).append("'");
        myencrypt(lf.pwd, lf.cpwd);
        char** row = db->db_select(sqlstr.c_str());
        int login_flag = 0;
        if(row == NULL)
        {
            login_flag = 0;
            size =send(csockfd,&login_flag,sizeof(int),0);
            close(csockfd);
            return false;
        }
        else
        {
            if(!strcmp(lf.id_name,row[0])&&!strcmp(lf.pwd,row[2]))
            {
                User user(lf.id_name,row[1],csockfd,lpszStr);
                if(list->ifhave(user))
                {
                    login_flag = 2;
                    size =send(csockfd,&login_flag,sizeof(int),0);
                    return true;
                }
                else
                {
                    //给客户端返回登录状态，正确登录返回1
                    cout << "confirmation successful" << endl;
                    login_flag = 1;
                    
                    //将已经登录的用户保存到online用户列表中
                    list->push(user);
                    list->show();
                    
                    //start a thread session for curr user
                    size =send(csockfd,&login_flag,sizeof(int),0);
                    Thread* thread = new Thread(user,list);
                    thread->start();
                    cout << "user " << row[0] << ":" << row[1] << "  online...." << endl;
                    return true;
                }
            }
            else
            {
                login_flag = 0;
                size =send(csockfd,&login_flag,sizeof(int),0);
                close(csockfd);
                return false;
            }
        }
    }
    else if(2 == lf.flag)//register
    {
        cout << "user name:" << lf.id_name << endl << "user pwd:" << lf.pwd << endl;
        char account[10] ={0};
        
        ifstream in("auto_account.dat");
        in.read(account,sizeof(account));
        in.close();

        int ac = atoi(account);
        ac++;
        string str;
        stringstream ss;
        ss << ac;
        ss >> str;
        ofstream out("auto_account.dat");
        out.write(str.c_str(),strlen(str.c_str()));
        if(!out)
        {
            cout << "write fail" << endl;
        }
        cout << account << endl;

        send(csockfd,account,sizeof(account),0);
        myencrypt(lf.pwd, lf.cpwd);
        string sqlstr("insert into user (id,name,pwd,cpwd) values('");
        sqlstr.append(account).append("','").append(lf.id_name).append("','").append(lf.pwd).append("','").append(lf.cpwd).append("')");
        db->db_insert(sqlstr.c_str());
        close(csockfd);
    }
    else //findpwd
    {
        //判断昵称和口令是否正确，实际应用从数据库查询
        string sqlstr("select * from user where name = '");
        sqlstr.append(lf.id_name).append("'");
        myencrypt(lf.pwd, lf.cpwd);
        char** row = db->db_select(sqlstr.c_str());

        if(row == NULL||strcmp(lf.id_name,row[1])||strcmp(lf.cpwd,row[3]))
        {
            lf.flag = 0;
            myencrypt(lf.pwd, lf.cpwd);
            send(csockfd,&lf,sizeof(lf),0);
            close(csockfd);
            return false;
        }
        else
        {
            lf.flag = 1;
            strcpy(lf.pwd,row[2]);
            strcpy(lf.id_name, row[0]);
            myencrypt(lf.pwd, lf.cpwd);
            send(csockfd,&lf,sizeof(lf),0);
            close(csockfd);
        }
    }
    return true;
}

void Server::closeServer()
{
    close(sockfd);
}

void Server::myencrypt(char *pwd, char *cpwd)
{
    int i;
    char key[20] = "0123456789876543210";
    for(i=0;pwd[i];i++)
        pwd[i]^=key[i];
    for(i=0;cpwd[i];i++)
        cpwd[i]^=key[i];
}
