#ifndef FINDPWD_H
#define FINDPWD_H

#include <QDialog>
#include <QDebug>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <QMessageBox>

namespace Ui {
class Findpwd;
}

struct findInfo
{
  char name[20]={0};
  char pwd[20]={0};
  char cpwd[20]={0};
  int flag;//3
};

class Findpwd : public QDialog
{
    Q_OBJECT

public:
    explicit Findpwd(int sockfd,QWidget *parent = 0);
    ~Findpwd();

private slots:
    void on_pushButton_clicked();

private:
    Ui::Findpwd *ui;
    int sockfd;
};

#endif // FINDPWD_H
