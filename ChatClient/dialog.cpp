#include "dialog.h"
#include "ui_dialog.h"

int flag=0;

Dialog::Dialog(QString m_NAME,QString m_IP,QWidget *parent) :
    QDialog(parent),IP(m_IP),NAME(m_NAME),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    socket = new QTcpSocket(this);
    server = new QTcpServer(this);
    ui->label->setText("端口号: "+PORT);
    ui->label_2->setText("名称： "+NAME);

    udpSocket = new QUdpSocket(this);
    udpSocket->bind(QHostAddress(IP), PORT.toInt());

    connect(udpSocket, SIGNAL(readyRead()),
    this, SLOT(readPendingDatagrams()));
    fflag="0";
    fpos=0;
    socket->disconnectFromHost();
    socket->connectToHost(QHostAddress(server_IP),6666);

    connect(socket,SIGNAL(readyRead()),
                this,SLOT(readMessage()));
}

Dialog::~Dialog()
{
    server->close();
    delete ui;
    delete this->socket;
}

void Dialog::on_pushButton_2_clicked()//选择文件
{
    fileName = QFileDialog::getOpenFileName(this,
                                           tr("Open Spreadsheet"), ".",
                                           tr("Images (*.MP4 *.RMVB)"));

    QString fname=fileName.right(fileName.size()-fileName.lastIndexOf('/')-1);
    QString ffname="fname";
    fname=ffname+"#"+fPORT+"#"+fname;
    sendMessage(fname);
    QEventLoop eventloop;
    QTimer::singleShot(100, &eventloop, SLOT(quit()));
    eventloop.exec();
    sendData(fileName);
}

void Dialog::sendData(QString fNAME)
{
    QFile Sfile(fNAME);
    if (!Sfile.open(QIODevice::ReadOnly))
    QMessageBox::information(this,"警告","文件打开失败",QMessageBox::Ok);
    fflag="1";
    Sfile.seek(fpos);
    if (!Sfile.atEnd()) {
    QByteArray line=Sfile.read(30000);
    fpos+=30000;
    udpSocket->writeDatagram( line , QHostAddress(fIP),fPORT.toInt());
    qDebug() << "send over!"<< line.size();
    }
    else {
        fpos=0;
        fflag="0";
        Sfile.close();
        QString ffname="fend";
        QString fname=ffname+"#"+fPORT;
        sendMessage(fname);
        QMessageBox::information(this,"恭喜","文件传送成功",QMessageBox::Ok);
    }
}

void Dialog::readPendingDatagrams()
{
    int i = 0;//receive
    while (udpSocket->hasPendingDatagrams()) {
    QByteArray datagram;
    datagram.resize(udpSocket->pendingDatagramSize());
    QHostAddress sender;
    quint16 senderPort;

    udpSocket->readDatagram(datagram.data(), datagram.size(),
    &sender, &senderPort);

    if(fflag=="1"){//发送
    sendData(fileName);
    }
    else if(fflag=="0"){//接收
    file.write(datagram.data(),datagram.size());
    i++;
    qDebug() << i << datagram.size();
    udpSocket->writeDatagram("1",1, sender ,senderPort);
    }}
}

void Dialog::newConnect()
{
    socket = server->nextPendingConnection();
    connect(socket,SIGNAL(readyRead()),
                this,SLOT(readMessage()));
    connect(socket,SIGNAL(disconnected()),
                this,SLOT(reconnect()));
    flag=1;
    server->close();
    d=new Dialog(NAME,IP);
    d->show();
}

void Dialog::reconnect()
{
    socket->disconnectFromHost();
    socket->connectToHost(QHostAddress(server_IP),6666);
    flag=2;
}

void Dialog::readMessage()
{
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_4);

    QString greeting;
    in >> greeting;
    QStringList info = greeting.split("#");
    if((info[0]=="f")&&(PORT == info[2]))
    {
        socket->disconnectFromHost();
        ui->lineEdit_4->setText(info[1]);
        flag=1;
        name=info[4];
        fIP=info[6];
        fPORT=info[1];
        socket->connectToHost(QHostAddress(info[6]),ui->lineEdit_4->text().toInt());
        ui->textEdit_2->append(tr("%1:  %2").arg(name).arg(info[3]));
        QString lev="m";
        QString m_message=lev+"#"+PORT+"#"+fPORT+"#"+IP;
        sendMessage(m_message);
        d=new Dialog(NAME,IP);
        d->show();
    }
    else if((info[0]=="m")&&(PORT == info[2]))
        {fIP=info[3];
        fPORT=info[1];}
    else if((info[0]=="fname")&&(PORT == info[1]))
    {
        QString sAppPath = QCoreApplication::applicationDirPath();
        QString filname=sAppPath+"/"+info[2];
        file.setFileName(filname);
        file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Unbuffered);
    }
    else if((info[0]=="fend")&&(PORT == info[1]))
    {
        QMessageBox::information(this,"恭喜","文件接收完成",QMessageBox::Ok);
        file.close();
    }
    else if((info[0]!="f")&&(PORT == info[1]))
      ui->textEdit_2->append(tr("%1:  %2").arg(info[3]).arg(info[2]));
    info.clear();
}

void Dialog::sendMessage(QString string)
{
     if((flag==2)&&(ui->lineEdit_4->text()!=""))
     {
         QString l="l";
         string=l+"#"+string;
     }
     if(flag==3)
     {
         QString leave="lea";
         string=leave+"#"+PORT;
     }
     QByteArray message;
     QDataStream out(&message,QIODevice::WriteOnly);
     out.setVersion(QDataStream::Qt_5_4);
     out<<string;
     qDebug() << "cli string" << string;
     socket->write(message);
}

void Dialog::on_pushButton_3_clicked()//退出
{
    server->close();
    socket->disconnectFromHost();
    socket->connectToHost(QHostAddress(server_IP),6666);
    flag=3;
    QString string;
    sendMessage(string);
    accept();
}

void Dialog::on_pushButton_clicked()//发送
{
    if(ui->lineEdit_2->text()=="")
    {
        QMessageBox::information(this,"警告","输入框不能为空",QMessageBox::Ok);
    }
    else
    {
    QString string = PORT + "#" + ui->lineEdit_4->text() + "#" + ui->lineEdit_2->text()+"#"+NAME+"#"+name;
    sendMessage(string);
    ui->lineEdit_2->clear();
    if(flag==0)
    {
        socket->disconnectFromHost();
        server->listen(QHostAddress::Any,PORT.toInt());
        connect(server,SIGNAL(newConnection()),
                    this,SLOT(newConnect()));
    }
    }
}
