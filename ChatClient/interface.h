#ifndef INTERFACE_H
#define INTERFACE_H

#include <QWidget>
#include <QObject>
#include <sys/types.h>
#include <sys/socket.h>
#include <QDebug>
#include <QMessageBox>
#include <iostream>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QProgressDialog>
#include <QFont>
#include <string>
#include <QCloseEvent>
#include <QTableWidgetItem>
#include "connectserver.h"
#include "msg.h"
#include <QFileDialog>
#include <QUdpSocket>
#include <QTextCodec>
#include <QFile>
#include <QTimer>

#define PORT_P2P 6666
#define PORT_F 7777
#define PACKSIZE 8000
using namespace std;

namespace Ui {
class Interface;
}

struct OnlineInfo
{
    char id[10];
    int online;// 1 for online ,0 for offline
};

class Interface : public QWidget
{
    Q_OBJECT

public:
    Interface(int sockfd,int my_id,QWidget *parent = 0);
    ~Interface();
protected:
    void closeEvent(QCloseEvent * event);
private:
    Ui::Interface *ui;
    int sockfd;
    Msg msg;

    int chatType;//0 is offline ,1 is online

    int my_id;
    int to_user_id;
    int isend;
    int irecv;

    QTcpSocket *socket;
    QTcpServer *server;
    QUdpSocket* udpSocket;
    bool condition;

    QString to_user_name;
    QString to_user_ip;
    QString fromname;
    QString fileName;
    QFile file;
    QProgressDialog *progressDlg;
    int sendflag;

    void p2pstart();
    void p2pover();
    void initp2pServer();
    void recvoffmsg();
    void delete_Msg();
    void initSocket();

private slots:
    void on_btn_search_clicked();
    void on_btn_over_clicked();
    void on_btn_send_clicked();
    void on_btn_fsend_clicked();

    void connected();
    void disconnected();
    void readData();

    void newConnection();

    void sendFile();
    void readPendingDatagrams();
};

#endif // INTERFACE_H
