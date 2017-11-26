#ifndef REGISTER_H
#define REGISTER_H

#include <QDialog>
#include <QDebug>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <QMessageBox>

namespace Ui {
class Register;
}

struct registerInfo
{
  char name[20]={0};
  char pwd[20]={0};
  char cpwd[20]={0};
  int flag;//2
};

class Register : public QDialog
{
    Q_OBJECT

public:
    explicit Register(int sockfd,QWidget *parent = 0);
    ~Register();

private:
    Ui::Register *ui;
    int sockfd;

private slots:
    void on_pushButton_clicked();
};

#endif // REGISTER_H
