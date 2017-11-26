#include "findpwd.h"
#include "ui_findpwd.h"

Findpwd::Findpwd(int sockfd,QWidget *parent) :
    sockfd(sockfd),QDialog(parent),
    ui(new Ui::Findpwd)
{
    sockfd = -1;
    ui->setupUi(this);
}

Findpwd::~Findpwd()
{
    delete ui;
}

void Findpwd::on_pushButton_clicked()
{
    if(!strcmp(ui->lineEdit_name->text().toStdString().c_str(),"")
            || !strcmp(ui->lineEdit_cpwd->text().toStdString().c_str(),""))
    {
        QMessageBox::about(NULL,"提示","信息不能为空");
        return;
    }
    findInfo ff;
    memset(&ff,0,sizeof(ff));
    ff.flag = 3;
    strcpy(ff.name,ui->lineEdit_name->text().toStdString().c_str());
    strcpy(ff.cpwd,ui->lineEdit_cpwd->text().toStdString().c_str());
    send(sockfd,&ff,sizeof(ff),0);

    recv(sockfd,&ff,sizeof(ff),0);
    if(ff.flag == 1)
    {
        qDebug() << "findpwd successful" << endl;
        QString str;
        QMessageBox::about(NULL,"找回密码",str.sprintf("找回成功！您的账号是：%s\n密码是：%s",ff.name,ff.pwd));
        this->close();
    }
    else
    {
        QMessageBox::about(NULL,"提示","找回失败！昵称或口令错误");
        return;
    }
}
