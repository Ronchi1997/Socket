#ifndef CUDPTRSMTFILE_H
#define CUDPTRSMTFILE_H

#include <QDialog>

namespace Ui {
class CUDPTrsmtFile;
}

class CUDPTrsmtFile : public QDialog
{
    Q_OBJECT

public:
    explicit CUDPTrsmtFile(QWidget *parent = 0);
    ~CUDPTrsmtFile();
    //发送
    bool ServerSend();
    bool ServerRecv();
    bool s_SendHoleMsg(void);
    bool s_RecvHoleSuccessMsg(void);
    //接收
    bool ClientRecv();
    bool c_RecvHole_SendHoleSuccessMsg(void);


private:
    Ui::CUDPTrsmtFile *ui;
};

#endif // CUDPTRSMTFILE_H
