#include "interface.h"
#include "ui_interface.h"

Interface::Interface(int sockfd, int my_id, QWidget *parent) :
    sockfd(sockfd),my_id(my_id),QWidget(parent),
    ui(new Ui::Interface)
{
    ui->setupUi(this);
    socket = new QTcpSocket(this);
    server = new QTcpServer(this);
    initSocket();

    chatType = 0;
    to_user_id = 0;
    msg.to_user_id = 0;
    msg.from_user_id = this->my_id;
    condition = false;

    ui->tableWidget_msgPage->setColumnCount(1);
    ui->tableWidget_msgPage->setShowGrid(false);
    ui->tableWidget_msgPage->setColumnWidth(0,500);
    ui->tableWidget_msgPage->verticalHeader()->setVisible(false);
    ui->tableWidget_msgPage->horizontalHeader()->setVisible(false);
    ui->tableWidget_msgPage->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->btn_send->setEnabled(false);
    ui->btn_fsend->setEnabled(false);
    ui->btn_over->setEnabled(false);
    recvoffmsg();
    initp2pServer();
//    thread = new recvMsgThread(sockfd);
//    thread->start();

    //connect(thread,SIGNAL(sendMsg_signal(Msg*)),this,SLOT(recv_msg(Msg*)));
}

Interface::~Interface()
{
    delete ui;
    socket->close();
    server->close();
}

void Interface::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton button;
    button = QMessageBox::question(this, tr("退出程序"),
                                   QString(tr("程序正在运行中 QAQ 你真的要退出吗?")),
                                   QMessageBox::Yes | QMessageBox::No);

    if (button == QMessageBox::No)
    {
        event->ignore();  //忽略退出信号，程序继续运行
    }
    else if (button == QMessageBox::Yes)
    {
        connectServer::closeSockfd(sockfd);
        event->accept();  //接受退出信号，程序退出
    }
}

//void Interface::recv_msg(Msg* msg)
//{
//    if(chatType)//P2P
//    {
//        to_user_id = msg->from_user_id;
//    }

//    if(msg->from_user_id == this->msg.from_user_id)
//        return;
//    int row_count = ui->tableWidget_msgPage->rowCount();
//    ui->tableWidget_msgPage->insertRow(row_count);
//    ui->tableWidget_msgPage->setItem(row_count,0,new QTableWidgetItem(QString(msg->from_user_name) +": " + QString(msg->msg)));
//}

void Interface::delete_Msg()
{
    int row_count = ui->tableWidget_msgPage->rowCount();
    while(row_count)
    {
        row_count--;
        ui->tableWidget_msgPage->removeRow(row_count);
    }
}

void Interface::on_btn_search_clicked()
{
    if(!strcmp(ui->lineEdit_Searchuser->text().toStdString().c_str(),""))
    {
        QMessageBox::about(NULL,"提示","输入不能为空");
        return;
    }
    char name[20];
    OnlineInfo of;
    strcpy(name,ui->lineEdit_Searchuser->text().toStdString().c_str());
    send(sockfd,name,sizeof(name),0);
    recv(sockfd,&of,sizeof(of),0);
    if(my_id == atoi(of.id))
    {
        QMessageBox::about(NULL,"提示","想和自己说话？不存在的");
        return;
    }
    if(of.online != 2)
    {
        chatType = of.online;
        to_user_id = atoi(of.id);
        if(of.online)
        {
            char ip[16],name[20];
            ui->chatuser->setText(ui->lineEdit_Searchuser->text()+"(在线)");
            recv(sockfd,&ip,sizeof(ip),0);
            recv(sockfd,&name,sizeof(name),0);
            to_user_ip = QString(ip);
            fromname = QString(name);
            ui->btn_fsend->setEnabled(true);
            p2pstart();
        }
        else
            ui->chatuser->setText(ui->lineEdit_Searchuser->text()+"(离线)");
        ui->btn_send->setEnabled(true);
        ui->btn_over->setEnabled(true);
        ui->btn_search->setEnabled(false);
        return;
    }
    else
    {
        QMessageBox::about(NULL,"提示","该用户不存在");
        return;
    }
}

void Interface::on_btn_over_clicked()
{
    ui->btn_send->setEnabled(false);
    ui->btn_fsend->setEnabled(false);
    ui->btn_over->setEnabled(false);
    ui->btn_search->setEnabled(true);
    ui->chatuser->setText("");
    ui->lineEdit_msg->clear();
    ui->lineEdit_Searchuser->clear();
    msg.to_user_id = 0;
    send(sockfd,&msg,sizeof(msg),0);

    p2pover();
}

void Interface::on_btn_send_clicked()
{
    QString msg_str = ui->lineEdit_msg->text();
    if(msg_str == "")
        return;
    msg.to_user_id = to_user_id;
    msg.from_user_id = this->my_id;
    strcpy(msg.msg,msg_str.toStdString().c_str());

    int row_count = ui->tableWidget_msgPage->rowCount();
    ui->tableWidget_msgPage->insertRow(row_count);
    ui->tableWidget_msgPage->setItem(row_count,0,new QTableWidgetItem(msg_str + " :我"));
    QTableWidgetItem* item = ui->tableWidget_msgPage->item(row_count,0);
    item->setTextAlignment(Qt::AlignRight);

    if(chatType == 0)
        send(sockfd,&msg,sizeof(msg),0);
    else
        socket->write((fromname + ": " + msg_str).toStdString().c_str());
    ui->lineEdit_msg->clear();
}

void Interface::recvoffmsg()
{
    int row_count = ui->tableWidget_msgPage->rowCount();
    ui->tableWidget_msgPage->insertRow(row_count);
    ui->tableWidget_msgPage->setItem(row_count,0,new QTableWidgetItem("— — — — —我收到的离线消息— — — — —"));
    QTableWidgetItem* item = ui->tableWidget_msgPage->item(row_count,0);
    item->setTextAlignment(Qt::AlignCenter);
    recv(sockfd,&msg,sizeof(msg),0);
    if(msg.to_user_id == 0)
    {
        row_count = ui->tableWidget_msgPage->rowCount();
        ui->tableWidget_msgPage->insertRow(row_count);
        ui->tableWidget_msgPage->setItem(row_count,0,new QTableWidgetItem("没有收到离线消息"));
    }
    else
    {
        row_count = ui->tableWidget_msgPage->rowCount();
        ui->tableWidget_msgPage->insertRow(row_count);
        ui->tableWidget_msgPage->setItem(row_count,0,new QTableWidgetItem(QString(msg.from_user_name) +": " + QString(msg.msg)));
    }
    row_count = ui->tableWidget_msgPage->rowCount();
    ui->tableWidget_msgPage->insertRow(row_count);
    ui->tableWidget_msgPage->setItem(row_count,0,new QTableWidgetItem("— — — — — — — — — — — — — — — —"));
    item = ui->tableWidget_msgPage->item(row_count,0);
    item->setTextAlignment(Qt::AlignCenter);
}

void Interface::p2pstart()
{
    if(chatType == 0)
        return;
    QHostAddress address(to_user_ip);
    socket->connectToHost(address, PORT_P2P);
    connected();
}

void Interface::p2pover()
{
    if(chatType == 0)
        return;
    socket->disconnectFromHost();
    disconnected();
}

void Interface::on_btn_fsend_clicked()
{
    fileName = QFileDialog::getOpenFileName(this,
                                           tr("Open Spreadsheet"), ".",
                                           tr("Images (*.MP3 *.MP4 *.RMVB *.JPG)"));
}

void Interface::initSocket() {

    connect(socket, SIGNAL(connected()), this, SLOT(connected()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(readData()));

}

void Interface::connected() {

    int row_count = ui->tableWidget_msgPage->rowCount();
    ui->tableWidget_msgPage->insertRow(row_count);
    ui->tableWidget_msgPage->setItem(row_count,0,new QTableWidgetItem("Connected!"));
    ui->btn_send->setEnabled(true);
    ui->btn_fsend->setEnabled(true);

    condition = true;

}

void Interface::disconnected() {

    int row_count = ui->tableWidget_msgPage->rowCount();
    ui->tableWidget_msgPage->insertRow(row_count);
    ui->tableWidget_msgPage->setItem(row_count,0,new QTableWidgetItem("Disconnected:) 连接已断开"));
    ui->btn_send->setEnabled(false);
    ui->btn_fsend->setEnabled(false);

    condition = false;

}

void Interface::readData() {

    QString data(socket->readAll());
    int row_count = ui->tableWidget_msgPage->rowCount();
    ui->tableWidget_msgPage->insertRow(row_count);
    ui->tableWidget_msgPage->setItem(row_count,0,new QTableWidgetItem(data));

}

void Interface::initp2pServer()
{
    server->listen(QHostAddress::Any, PORT_P2P);

    connect(server, SIGNAL(newConnection()), this, SLOT(newConnection()));
}

void Interface::newConnection() {

    socket = server->nextPendingConnection();
    int row_count = ui->tableWidget_msgPage->rowCount();
    ui->tableWidget_msgPage->insertRow(row_count);
    ui->tableWidget_msgPage->setItem(row_count,0,new QTableWidgetItem("Connected!"));
    ui->btn_send->setEnabled(true);
    ui->btn_fsend->setEnabled(true);
    connect(socket, SIGNAL(readyRead()), this, SLOT(readData()));

}
