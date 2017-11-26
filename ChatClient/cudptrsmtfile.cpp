#include "cudptrsmtfile.h"
#include "ui_cudptrsmtfile.h"

CUDPTrsmtFile::CUDPTrsmtFile(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CUDPTrsmtFile)
{
    ui->setupUi(this);
}

CUDPTrsmtFile::~CUDPTrsmtFile()
{
    delete ui;
}

//发送方

bool CUDPTrsmtFile::ServerSend()
{
    if(!s_SendHoleMsg())
        return false;
    // 设置文件指针位置，指向上次已发送的大小
    SetFilePointer(m_hFile, m_dwSend, NULL, FILE_BEGIN);
    int  ret;
    int  nPacketCount = 0;
    DWORD dwRet;
    SendBuf sendbuf;
    DWORD dwRead;
    DWORD dwReadSize;
    SendBuf* pushbuf;
    SetEvent(m_hEvent);

    //若已发送大小小于文件大小并且发送窗口前沿等于后沿，则继续发送
    //否则退出循环
    m_sumPacket = (m_dwFileSize - m_dwSend)/(MAXBUF_SIZE);
    if(m_dwSend < m_dwFileSize)  // 文件没有传输完时才继续传输
    {
        while(1)
        {
            dwRet = WaitForSingleObject(m_hEvent, 1000);
            if(dwRet == WAIT_FAILED)
            {
                CString temp="等待发送信号灯出错!";
                SendMessage(the_hWnd,ECHO_MSG,0,LPARAM(&temp));
                SendMessage(the_hWnd,FINISHEDTRANS_MSG,0,0);
                return false;
            }
            else if(dwRet == WAIT_TIMEOUT)
            {
                //重发
                int ackindex=0;
                ::EnterCriticalSection(&m_csQueue);  // 进入m_bufqueue的排斥区
                ackindex=m_bufqueue.front()->index;
                ret = m_hsocket.hsendto((char*)m_bufqueue.front(), sizeof(sendbuf));
                ::LeaveCriticalSection(&m_csQueue);  // 退出m_bufqueue的排斥区
                if(ret == SOCKET_ERROR)
                {
                    CString temp="发送窗口中的数据包发送失败，重发数据包!";
                    SendMessage(the_hWnd,ECHO_MSG,0,LPARAM(&temp));
                    continue;
                }
                CString temp;
                temp.Format("等待ACK确认消息超时,重发发送窗口的第 %d 个数据包",ackindex);
                SendMessage(the_hWnd,ECHO_MSG,0,LPARAM(&temp));
                ResetEvent(m_hEvent);
                continue;
            }
            //若发送窗口大小 < 预定大小 && 已读文件次数(nReadIndex) < 需要读文件的次数(nReadCount)，则继续读取发送
            //否则，要发送的内容从m_bufqueue里提取
            if(m_dwSend < m_dwFileSize)
            {
                dwReadSize = m_dwFileSize - m_dwSend;
                dwReadSize = dwReadSize < MAXBUF_SIZE ? dwReadSize : MAXBUF_SIZE;
                memset(sendbuf.buf, 0, sizeof(sendbuf.buf));
                if(!ReadFile(m_hFile, sendbuf.buf, dwReadSize, &dwRead, NULL))
                {
                    CString temp="读取文件失败,请确认文件存在或有读取权限!";
                    SendMessage(the_hWnd,ECHO_MSG,0,LPARAM(&temp));
                    SendMessage(the_hWnd,FINISHEDTRANS_MSG,0,0);
                    return false;
                }
                m_dwSend += dwRead;
                nPacketCount++;
                sendbuf.index = m_nSendIndexHead;
                m_nSendIndexHead = (m_nSendIndexHead + 1) % Sliding_Window_Size; // 发送窗口前沿向前移一格
                sendbuf.dwLen = dwRead;
                //保存发送过的数据，以便重发
                ::EnterCriticalSection(&m_csQueue);   // 进入m_bufqueue的排斥区
                pushbuf = GetBufFromLookaside();
                memcpy(pushbuf, &sendbuf, sizeof(sendbuf));
                m_bufqueue.push(pushbuf);
                if(m_dwSend >= m_dwFileSize)    // 文件已读完，在队列中加一File_End标志，以便判断是否需要继续发送
                {
                    pushbuf = GetBufFromLookaside();
                    pushbuf->index = File_End;
                    pushbuf->dwLen = File_End;
                    memset(pushbuf->buf, 0, sizeof(pushbuf->buf));
                    m_bufqueue.push(pushbuf);
                }
                ::LeaveCriticalSection(&m_csQueue);   // 退出m_bufqueue的排斥区
            }
            ::EnterCriticalSection(&m_csQueue);    // 进入m_bufqueue的排斥区
            if(m_bufqueue.front()->index == File_End)  // 所有数据包已发送完毕,退出循环
            {
                ::LeaveCriticalSection(&m_csQueue);   // 退出m_bufqueue的排斥区
                break;
            }
            else if(m_bufqueue.size() <= Send_Window_Size) // 发送窗口小于指定值，继续发送
            {
                ret = m_hsocket.hsendto((char*)m_bufqueue.front(), sizeof(sendbuf));
                CString temp;
                temp.Format("发送:发送窗口第 %d 个数据包正被发送!",m_bufqueue.front()->index);
                SendMessage(the_hWnd,ECHO_MSG,0,LPARAM(&temp));
                if(ret == SOCKET_ERROR)
                {
                    ::LeaveCriticalSection(&m_csQueue);  // 退出m_bufqueue的排斥区
                    CString temp="发送数据包失败，重发!";
                    SendMessage(the_hWnd,ECHO_MSG,0,LPARAM(&temp));
                    continue;
                }
                //延时，防止丢包
                Sleep(50);
            }
            else           // 发送窗口大于指定值，等持接收线程接收确认消息
            {
                ResetEvent(m_hEvent);
            }
            ::LeaveCriticalSection(&m_csQueue);    // 退出m_bufqueue的排斥区
        }
    }
    CString temp;
    temp.Format("共分成 %d 个数据包传送文件!",nPacketCount);
    SendMessage(the_hWnd,ECHO_MSG,0,LPARAM(&temp));
    CloseHandle(m_hEvent);
    CloseHandle(m_hFile);
    m_hFile = NULL;
    m_hsocket.hclosesocket();

    while(!m_bufLookaside.empty()) // clear
    {
        delete m_bufLookaside.front();
        m_bufLookaside.pop();
    }

    temp="-------文件已经传输完毕-------!";
    SendMessage(the_hWnd,ECHO_MSG,0,LPARAM(&temp));
    SendMessage(the_hWnd,FINISHEDTRANS_MSG,0,0);
    return true;
}

bool CUDPTrsmtFile::ServerRecv()
{
    if(!s_RecvHoleSuccessMsg())
        return false;
    int ret;
    RecvBuf recvbuf;
    int recvSumPacket=0;
    while(m_hFile != NULL)
    {
        ret = m_hsocket.hrecvfrom((char*)&recvbuf, sizeof(recvbuf));
        if(ret == SOCKET_ERROR)
        {
            ::EnterCriticalSection(&m_csQueue);
            if(m_bufqueue.front()->index == File_End) // 文件传输完毕
            {
                ::LeaveCriticalSection(&m_csQueue);
                break;
            }
            ::LeaveCriticalSection(&m_csQueue);
            CString temp="接收文件传输ACK消息出错!";
            SendMessage(the_hWnd,ECHO_MSG,0,LPARAM(&temp));
            SendMessage(the_hWnd,FINISHEDTRANS_MSG,0,0);
            return false;
        }
        if(recvbuf.flag == Flag_Ack && recvbuf.index == m_nSendIndexTail)
        {
            CString temp;
            temp.Format("接收:已接收到发送窗口第 %d 个数据包的ACK消息!",recvbuf.index);
            SendMessage(the_hWnd,ECHO_MSG,0,LPARAM(&temp));
            recvSumPacket++;
            int progress=(int)((1.0*recvSumPacket/m_sumPacket)*100);
            SendMessage(the_hWnd,PROGRESS_MSG,0,LPARAM(&progress));
            m_nSendIndexTail = (m_nSendIndexTail + 1) % Sliding_Window_Size;

            //该结点已得到确认，将其加入旁视列表，以备再用
            ::EnterCriticalSection(&m_csQueue);
            m_bufLookaside.push(m_bufqueue.front());
            m_bufqueue.pop();
            ::LeaveCriticalSection(&m_csQueue);
            SetEvent(m_hEvent);
        }
    }
    return true;
}

bool CUDPTrsmtFile::s_SendHoleMsg(void)
{
    int  ret;
    int  count = 0;
    DWORD dwRet;
    char sendbuf[32];
    //发送连接消息,最多发10次，若10次都失败，则不能通信
    while(1)
    {
        if(count == 10)
        {
            CString temp="连接对方超时!";
            SendMessage(the_hWnd,ECHO_MSG,0,LPARAM(&temp));
            SendMessage(the_hWnd,FINISHEDTRANS_MSG,0,0);
            return false;
        }
        strcpy(sendbuf, "hole");
        ret = m_hsocket.hsendto(sendbuf, strlen(sendbuf));
        if(ret == SOCKET_ERROR)
        {
            CString temp="发送连接消息失败!";
            SendMessage(the_hWnd,ECHO_MSG,0,LPARAM(&temp));
            SendMessage(the_hWnd,FINISHEDTRANS_MSG,0,0);
            return false;
        }

        dwRet = WaitForSingleObject(m_hEvent, 2 * 1000); // 等待接收线程接收连接成功消息
        if(dwRet == WAIT_FAILED)
        {
            return false;
        }
        else if(dwRet == WAIT_TIMEOUT)
        {
            CString temp="重新给对方发送连接消息!";
            SendMessage(the_hWnd,ECHO_MSG,0,LPARAM(&temp));
            count++;
            continue; // 重发连接消息
        }
        break;
    }

    //发送文件总大小
    FileInfo fi;
    fi.dwLen=m_dwFileSize;
    strcpy(fi.filename,m_hFileName);
    ret = m_hsocket.hsendto((char*)&fi,sizeof(fi));
    if(ret == SOCKET_ERROR)
    {
        CString temp="发送传送文件信息失败!";
        SendMessage(the_hWnd,ECHO_MSG,0,LPARAM(&temp));
        SendMessage(the_hWnd,FINISHEDTRANS_MSG,0,0);
        return false;
    }

    ResetEvent(m_hEvent);       //设置m_hEvent为无信号状态，等待接收线程接收已发送文件大小

    dwRet = WaitForSingleObject(m_hEvent, 15 * 1000);
    if(dwRet == WAIT_FAILED)
    {
        CString temp="等待信号灯出错!";
        SendMessage(the_hWnd,ECHO_MSG,0,LPARAM(&temp));
        SendMessage(the_hWnd,FINISHEDTRANS_MSG,0,0);
        return false;
    }
    else if(dwRet == WAIT_TIMEOUT)
    {
        CString temp="对方没响应或拒收发送的文件，您可以再重发一次!";
        SendMessage(the_hWnd,ECHO_MSG,0,LPARAM(&temp));
        SendMessage(the_hWnd,FINISHEDTRANS_MSG,0,0);
        return false;
    }

    SetEvent(m_hEvent);  //设置m_hEvent为有信号状态，以便发送文件
    return true;
}

bool CUDPTrsmtFile::s_RecvHoleSuccessMsg(void)
{
    int ret;
    char szbuf[64];
    RecvBuf recvbuf;
    while(1)
    {
        ret = m_hsocket.hrecvfrom(szbuf, sizeof(szbuf));  //接收连接成功消息
        if(ret == SOCKET_ERROR)
        {
            CString temp="接收连接消息不成功!";
            SendMessage(the_hWnd,ECHO_MSG,0,LPARAM(&temp));
            Sleep(1000);
            continue;
        }
        else
        {
            szbuf[ret]='/0';
            if(strcmp(szbuf,"holesuccess")==0)
            {
                CString temp="连接成功，开始准备传送文件!";
                SendMessage(the_hWnd,ECHO_MSG,0,LPARAM(&temp));
            }
            break;
        }
    }

    SetEvent(m_hEvent);  //接收成功，设置m_hEvent为有信号状态，以便发送线程退出发送连接消息循环

    ret = m_hsocket.hrecvfrom((char*)&recvbuf, sizeof(recvbuf));  //接收已传输送文件大小
    if(ret == SOCKET_ERROR)
    {
        CString temp="接收已发送文件大小信息出错!";
        SendMessage(the_hWnd,ECHO_MSG,0,LPARAM(&temp));
        SendMessage(the_hWnd,FINISHEDTRANS_MSG,0,0);
        return false;
    }
    if(recvbuf.flag == Flag_Refuse)
    {
        CString temp="Sorry,对方拒收传送的文件!";
        SendMessage(the_hWnd,ECHO_MSG,0,LPARAM(&temp));
        CloseHandle(m_hEvent);

        CloseHandle(m_hFile);
        m_hFile = NULL;

        m_hsocket.hclosesocket();
        return false;
    }
    m_dwSend = recvbuf.dwFileSize;
    CString temp;
    temp.Format("上次断点已传送 %ld Byte!",m_dwSend);
    SendMessage(the_hWnd,ECHO_MSG,0,LPARAM(&temp));

    SetEvent(m_hEvent);  //接收成功，设置m_hEvent为有信号状态，发送线程开始发送文件
    return true;
}

//接收方

bool CUDPTrsmtFile::ClientRecv()
{
    if(!c_RecvHole_SendHoleSuccessMsg())
        return false;
    CString bstr="开始接收文件!";
    SendMessage(the_hWnd,ECHO_MSG,0,LPARAM(&bstr));
    int  ret;
    DWORD dwWritten;
    SendBuf recvbuf;
    RecvBuf sendbuf;
    int nerror = 0;
    // 设置文件指针位置，指向上次已发送的大小
    SetFilePointer(m_hFile, 0, NULL, FILE_END);
    m_sumPacket = (m_dwFileSize - m_dwSend)/(MAXBUF_SIZE)+1;
    int recvPacket = 0;
    //若已接收文件大小小于需要接收的大小，则继续
    while(m_dwSend < m_dwFileSize)
    {
        //接收
        memset(&recvbuf, 0, sizeof(recvbuf));
        ret = m_hsocket.hrecvfrom((char*)&recvbuf, sizeof(recvbuf));
        if(ret == SOCKET_ERROR)
        {
            CString temp="接收文件数据包出现错误，终止接收!";
            SendMessage(the_hWnd,ECHO_MSG,0,LPARAM(&temp));
            return false;
        }
        //不是希望接收的，丢弃，继续接收
        if(recvbuf.index != (m_nRecvIndex) % Sliding_Window_Size)
        {
            nerror++;
            CString temp="接收的数据报文非当前所需，丢弃!";
            SendMessage(the_hWnd,ECHO_MSG,0,LPARAM(&temp));
            continue;
        }
        if(!WriteFile(m_hFile, recvbuf.buf, recvbuf.dwLen, &dwWritten, NULL))
        {
            CString temp="写文件出现错误，文件接收终止!";
            SendMessage(the_hWnd,ECHO_MSG,0,LPARAM(&temp));
            return false;
        }
        //已接收文件大小
        m_dwSend += dwWritten;
        //发送确认消息
        sendbuf.flag = Flag_Ack;
        sendbuf.index = m_nRecvIndex;

        ret = m_hsocket.hsendto((char*)&sendbuf, sizeof(sendbuf));
        if(ret == SOCKET_ERROR)
        {
            CString temp="发送ACK确认报文出错，文件接收终止!";
            SendMessage(the_hWnd,ECHO_MSG,0,LPARAM(&temp));
            return false;
        }

        recvPacket++;
        int progress=(int)((1.0*recvPacket/m_sumPacket)*100);
        SendMessage(the_hWnd,PROGRESS_MSG,0,LPARAM(&progress));
        //接收窗口前移一格
        m_nRecvIndex = (m_nRecvIndex + 1) % Sliding_Window_Size;
    }
    CString temp;
    temp.Format("-------文件接收完毕!--------/n>总共接收数据包个数 %ld 个!/n>传输过程中出错的数据包个数: %d 个!",m_sumPacket,nerror);
    SendMessage(the_hWnd,ECHO_MSG,0,LPARAM(&temp));
    CloseHandle(m_hFile);
    m_hFile = NULL;
    m_hsocket.hclosesocket();
    SendMessage(the_hWnd,LISTEN_MSG,0,0);
    return true;
}

bool CUDPTrsmtFile::c_RecvHole_SendHoleSuccessMsg(void)
{
    int  ret;
    char recvbuf[64];
    while(1)
    {
        ret = m_hsocket.hrecvfrom(recvbuf, sizeof(recvbuf));
        if(ret == SOCKET_ERROR)
        {
        //    CString temp="接收连接消息失败!";
        //    SendMessage(the_hWnd,ECHO_MSG,0,LPARAM(&temp));
            return false;
        }
        recvbuf[ret] = '/0';
        //收到对方发来的连接消息，结束循环。同时新建套接字，向发对方发送连接成功消息，否则继续等待接收
        if(strcmp(recvbuf, "hole") == 0)
        {
            //发送连接成功消息
            char szsend[] = "holesuccess";
            m_hsocket.makesendsockaddr(m_hsocket.getsockaddr(), m_hsocket.getsockport());
            ret = m_hsocket.hsendto(szsend, sizeof(szsend));
            if(ret == SOCKET_ERROR)
            {
                CString temp="发送连接成功确认消息失败!";
                SendMessage(the_hWnd,ECHO_MSG,0,LPARAM(&temp));
                return false;
            }
            break;
        }
        else
        {
            CString temp="再次接收连接消息!";
            SendMessage(the_hWnd,ECHO_MSG,0,LPARAM(&temp));
            continue;
        }
    }
    Sleep(10); // 等待对方接收

    CString temp="连接成功!";
    SendMessage(the_hWnd,ECHO_MSG,0,LPARAM(&temp));
    FileInfo fi;
    ret = m_hsocket.hrecvfrom((char*)&fi,sizeof(fi));
    if(ret == SOCKET_ERROR)
    {
        CString temp="接收文件信息失败!";
        SendMessage(the_hWnd,ECHO_MSG,0,LPARAM(&temp));
        return false;
    }
    m_dwFileSize=fi.dwLen;
    strcpy(m_hFileName,fi.filename);
    CString fName=CString(m_hFileName);
    fName.Replace('//','/');
    int index=fName.ReverseFind('/');
    CString namestr=fName.Right(fName.GetLength()-index-1);
    //SendMessage(the_hWnd,ECHO_MSG,0,LPARAM(&namestr));
    if(IDNO == AfxMessageBox("有文件发送过来，是否接收?",MB_YESNO))
    {
        RecvBuf sendbuf;
        sendbuf.index = -1;
        sendbuf.flag = Flag_Refuse;

        ret = m_hsocket.hsendto((char*)&sendbuf, sizeof(sendbuf));
        if(ret == SOCKET_ERROR)
        {
            CString temp="发送拒收消息失败!";
            SendMessage(the_hWnd,ECHO_MSG,0,LPARAM(&temp));
            return false;
        }

        CString temp="发送拒收消息成功，本机不接收该文件!";
        SendMessage(the_hWnd,ECHO_MSG,0,LPARAM(&temp));

        m_hsocket.hclosesocket();
        SendMessage(the_hWnd,LISTEN_MSG,0,0);
        return false;
    }
    char szName[255];
    strcpy(szName,LPCTSTR(namestr));
    saveDlg.m_ofn.lpstrFile=new char[255];
    memset(saveDlg.m_ofn.lpstrFile,0,255);
    //strcpy(saveDlg.m_ofn.lpstrFile,(LPCTSTR)namestr);
    saveDlg.m_ofn.lpstrFile=LPSTR((LPCTSTR)namestr);
    if (saveDlg.DoModal() == IDOK)
    {
        CString fullname=saveDlg.GetPathName();
        strcpy(m_hFileName,LPCTSTR(fullname));
        CString temp;
        temp.Format("待接收的文件: ");
        temp+=fullname;
        temp+="!";
        SendMessage(the_hWnd,ECHO_MSG,0,LPARAM(&temp));
    }
    delete[] saveDlg.m_ofn.lpstrFile;
    SendMessage(the_hWnd,UPDATEINFO_MSG,0,0);
    SetFile(m_hFileName);
    temp.Format("待接收文件的总大小为 %ld Byte!",fi.dwLen);
    SendMessage(the_hWnd,ECHO_MSG,0,LPARAM(&temp));

    //发送已传输文件大小
    RecvBuf sendbuf;
    sendbuf.dwFileSize = m_dwSend;
    sendbuf.flag = Flag_ReSend;
    ret = m_hsocket.hsendto((char*)&sendbuf, sizeof(sendbuf));
    if(ret == SOCKET_ERROR)
    {
        CString temp="发送已传送文件大小失败!";
        SendMessage(the_hWnd,ECHO_MSG,0,LPARAM(&temp));
        return false;
    }
    temp.Format("发送断点信息:上次断点已经接收 %ld Byte!",m_dwSend);
    SendMessage(the_hWnd,ECHO_MSG,0,LPARAM(&temp));
    return true;
}
