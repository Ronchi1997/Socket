#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QTcpSocket>
#include <QTcpServer>
#include <QFileDialog>
#include <QUdpSocket>
#include <QMessageBox>

#include <QHostAddress>
#include <QTextCodec>
#include <QFile>
#include <string>
#include <QTimer>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QString m_NAME,QString m_IP,QWidget *parent = 0);
    ~Dialog();

public slots:

    void readMessage();

    void sendMessage(QString string);

    void newConnect();

    void sendData(QString fNAME);

    void readPendingDatagrams();

private slots:

    void on_pushButton_3_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

    void reconnect();

public:
    QString NAME;
    QString IP;
    QString PORT;
    Ui::Dialog *ui;

    QString server_IP = "202.114.212.220";
    Dialog *d;
    QString message;
    QString fIP;
    QString fPORT = "8910";
    QString name;
    QString fileName;
    QTcpSocket *socket;
    QTcpServer *server;
    QUdpSocket *udpSocket;
    QFile file;
    int fpos=0;
    QString fflag="0";
};

#endif // DIALOG_H
