#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>
#include <string>
#include "connectserver.h"
#include "interface.h"
#include "register.h"
#include "findpwd.h"
#define PORT 8888

namespace Ui {
class Login;
}

struct loginInfo
{
  char id[20]={0};
  char pwd[20]={0};
  char cpwd[20]={0};
  int flag;//1
};

class Login : public QDialog
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = 0);
    ~Login();

private:
    Ui::Login *ui;
    Interface* main_face;
    Register* regist;
    Findpwd* findpwd;
    int sockfd;
    connectServer cs;
    string IP;
private slots:
    void on_btn_login_clicked();
    void on_btn_register_clicked();
    void on_btn_findpwd_clicked();
};

#endif // LOGIN_H
