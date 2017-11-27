#include "login.h"
#include "ui_login.h"

Login::Login(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Login)
{
    sockfd = -1;
    ui->setupUi(this);
}

Login::~Login()
{
    delete ui;
}

void Login::on_btn_login_clicked()
{
    IP = ui->lineEdit->text().toStdString().c_str();
    if(sockfd != -1)
        cs.closeSockfd(sockfd);
    sockfd = cs.connectTo(IP,PORT);
    if(-1 == sockfd)
    {
        QMessageBox::about(NULL,"提示","连接不到服务器");
        return;
    }

    loginInfo lf;
    lf.flag = 1;
    strcpy(lf.id ,ui->lineEdit_account->text().toStdString().c_str());
    strcpy(lf.pwd,ui->lineEdit_pwd->text().toStdString().c_str());
    if(!strcmp(lf.id,"") || !strcmp(lf.pwd,""))
    {
        QMessageBox::about(NULL,"提示","账号或密码不能为空");
        return;
    }
    ssize_t size = send(sockfd,(void*)&lf,sizeof(lf),0);
    if(-1 == size)
        return;

    int flag = 0;
    size = recv(sockfd,&flag,sizeof(int),0);
    if(1 == flag)
    {
        qDebug() << "login successful" << endl;
        main_face = new Interface(sockfd,atoi(lf.id));
        main_face->show();
        main_face->setWindowTitle("IM");
        this->close();
        //delete this;
    }
    else if(2 == flag)
    {
        QMessageBox::about(NULL,"提示","用户已经在线");
        return;
    }
    else
    {
        QMessageBox::about(NULL,"提示","账号或密码错误");
        return;
    }
}

void Login::on_btn_register_clicked()
{
    IP = ui->lineEdit->text().toStdString().c_str();
    if(-1 == sockfd)
    {
        connectServer cs;
        sockfd = cs.connectTo(IP,PORT);
        if(-1 == sockfd)
        {
            QMessageBox::about(NULL,"提示","连接不到服务器");
            return;
        }
    }
    regist = new Register(sockfd);
    regist->show();
    regist->setWindowTitle("Register");
}

void Login::on_btn_findpwd_clicked()
{
    IP = ui->lineEdit->text().toStdString().c_str();
    if(-1 == sockfd)
    {
        connectServer cs;
        sockfd = cs.connectTo(IP,PORT);
        if(-1 == sockfd)
        {
            QMessageBox::about(NULL,"提示","连接不到服务器");
            return;
        }
    }
    findpwd = new Findpwd(sockfd);
    findpwd->show();
    findpwd->setWindowTitle("FindPassword");
}
