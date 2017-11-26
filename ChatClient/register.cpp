#include "register.h"
#include "ui_register.h"

Register::Register(int sockfd, QWidget *parent) :
   sockfd(sockfd),QDialog(parent),
    ui(new Ui::Register)
{
    ui->setupUi(this);
}

Register::~Register()
{
    delete ui;
}

void Register::on_pushButton_clicked()
{
    registerInfo rf;
    memset(&rf,0,sizeof(rf));
    rf.flag = 2;
    if(ui->lineEdit_name->text()==""||ui->lineEdit_pwd->text()==""||ui->lineEdit_cpwd->text()=="")
    {
        QMessageBox::about(NULL,"提示","信息不能为空");
        return;
    }
    strcpy(rf.name,ui->lineEdit_name->text().toStdString().c_str());
    strcpy(rf.pwd,ui->lineEdit_pwd->text().toStdString().c_str());
    strcpy(rf.cpwd,ui->lineEdit_cpwd->text().toStdString().c_str());
    send(sockfd,&rf,sizeof(rf),0);
    char account[10] = {0};
    recv(sockfd,account,sizeof(account),0);
    if(!strcmp(account,"0"))
    {
        QMessageBox::about(NULL,"注册信息","注册失败！该昵称已存在！");
        ui->lineEdit_name->clear();
        ui->lineEdit_pwd->clear();
        ui->lineEdit_cpwd->clear();
        return;
    }
    QMessageBox::about(NULL,"注册信息","注册成功！您的账号是："+ QString(account));
    this->close();
}
