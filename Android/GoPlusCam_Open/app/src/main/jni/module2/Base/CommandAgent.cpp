//
//  CommandAgent.cpp
//  GPCam
//
//  Created by generalplus_sa1 on 8/11/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//
#include <sys/time.h>
#include "CommandAgent.h"
#include <signal.h>
#include <string.h>
#include <stdio.h>

extern C_CommandAgent *g_pCommandAgent;

static pthread_mutex_t Cmd_Queuelock;
static pthread_mutex_t start_Connectlock;

//----------------------------------------------------
/* Catch Signal Handler functio */
void signal_callback_handler(int signum)
{
    DEBUG_PRINT("Caught signal SIGPIPE %d\n",signum);
    g_pCommandAgent->SetStatus(E_ConnectionStatus_DisConnected);
    g_pCommandAgent->Disconnect();
    g_pCommandAgent->FireNak();
}
//----------------------------------------------------
C_CommandAgent::C_CommandAgent():
m_status(E_ConnectionStatus_Idle),
m_CmdStatuCallBack(NULL),
m_bIsCmdRunning(FALSE),
m_ConnectThreadID(0),
m_RevThreadID(0),
m_bForceToStop(false)
{
    pthread_mutex_init(&start_Connectlock, NULL);
}
C_CommandAgent::~C_CommandAgent()
{
    ClearCmdQueue();
}
//----------------------------------------------------
int C_CommandAgent::ConnectToDevice(
                    LPCTSTR pszIPAddress,
                    int i32PortNum)
{
    int i32Ret = 0;
    strcpy(m_TargerIP,pszIPAddress);
    m_TargetPort = i32PortNum;
    
    if(m_status == E_ConnectionStatus_DisConnected)
        m_status = E_ConnectionStatus_Idle;
    
    pthread_create(&m_ConnectThreadID, NULL,ConnectThreadFun,NULL);
    return i32Ret;
}
//----------------------------------------------------
void C_CommandAgent::Disconnect()
{
    m_TcpSocket.CloseSocket();
    m_status = E_ConnectionStatus_DisConnected;
}
//----------------------------------------------------
void C_CommandAgent::SetCmdStatusCallBack(PFN_CmdStatusCallBack CallBack)
{
    m_CmdStatuCallBack = CallBack;
}
//----------------------------------------------------
int  C_CommandAgent::Abort(int i32Index)
{
    int i32Ret = 0;
    
    pthread_mutex_lock(&Cmd_Queuelock);
    StreamCmdVector_iterator iterator;
    for(iterator=m_CmdQueue.begin(); iterator!=m_CmdQueue.end(); ++iterator)
    {
        if((*iterator)->GetCMDIdex() == i32Index)
        {
            DEBUG_PRINT("Cmd Abort  ---- %d\n",i32Index);
            m_bIsCmdRunning = false;
            delete (*iterator);
            m_CmdQueue.erase(iterator);
            break;
        }
    }
    pthread_mutex_unlock(&Cmd_Queuelock);

    
    return i32Ret;
}
//----------------------------------------------------
void C_CommandAgent::QueueCmd(I_StreamCommand* pCommand)
{
    C_StreamCommandBase *pCmd = static_cast< C_StreamCommandBase*>(pCommand);
    m_CmdQueue.push_back(pCmd);
    
    RunCmdQueue();
}
//----------------------------------------------------
void C_CommandAgent::RunCommand()
{
    if(!m_CmdQueue.empty())
    {
        C_StreamCommandBase *pCmd = m_CmdQueue.at(0);
        gettimeofday(pCmd->GetSendTime(), NULL);
        m_bIsCmdRunning = true;
        
        if(pCmd->GetCommandType() == E_CommandType_Action)
        {
            DEBUG_PRINT("Cmd Action ++++ %d\n",pCmd->GetCMDIdex());
            pCmd->DoAction();
            DEBUG_PRINT("Cmd Delete ---- %d\n",pCmd->GetCMDIdex());
            
            delete pCmd;
            m_CmdQueue.erase(m_CmdQueue.begin());
            m_bIsCmdRunning = false;
            RunCommand();
        }
        else
        {
            DEBUG_PRINT("Cmd Run    ++++ %d\n",pCmd->GetCMDIdex());
            if(m_TcpSocket.sendStream(pCmd->GetPacketStream(), pCmd->GetStreamSize()) == Error_Socket_Broken)
            {
                DEBUG_PRINT("Failed to send command!");
                Disconnect();
            }
        }
    }
}
//----------------------------------------------------
void C_CommandAgent::RunCmdQueue()
{
    if(!m_bIsCmdRunning && m_status == E_ConnectionStatus_Connected)
    {
        if(pthread_mutex_trylock(&Cmd_Queuelock)==0)
        {
            RunCommand();
            
            pthread_mutex_unlock(&Cmd_Queuelock);
        }
    }
}
//----------------------------------------------------
void C_CommandAgent::FireNak()
{
    short ErrorCode = Error_LostConnection;
    
    if(m_CmdStatuCallBack)
        m_CmdStatuCallBack(-1,GP_SOCK_TYPE_NAK,0,0,sizeof(ErrorCode),(BYTE*)&ErrorCode);
}
//----------------------------------------------------
int C_CommandAgent::SendCommand(C_StreamCommandBase* pCommand)
{
    if(m_status!=E_ConnectionStatus_Connected)
    {
        delete pCommand;
        FireNak();
        return -1;
    }
    
    int i32Ret = 0;
    
    QueueCmd(pCommand);
    i32Ret = pCommand->GetCMDIdex();
    
    return i32Ret;
}
//----------------------------------------------------
E_ConnectionStatus C_CommandAgent::GetStatus()
{
    //DEBUG_PRINT("Status %d\n",m_status);
    
    return m_status;
}
//----------------------------------------------------
void  C_CommandAgent::SetStatus(E_ConnectionStatus eStatus)
{
    m_status = eStatus;
}
//----------------------------------------------------
void  C_CommandAgent::ClearCmdQueue()
{
    pthread_mutex_lock(&Cmd_Queuelock);
    
    StreamCmdVector_iterator iterator;
    for(iterator=m_CmdQueue.begin(); iterator!=m_CmdQueue.end(); ++iterator)
        delete (*iterator);

    m_CmdQueue.clear();
    
    m_bIsCmdRunning = false;
    
    pthread_mutex_unlock(&Cmd_Queuelock);
}
//----------------------------------------------------
void* C_CommandAgent::ConnectThreadFun(void* lpParam)
{
    g_pCommandAgent->HandleConnect();
    return 0;
}
//----------------------------------------------------
void C_CommandAgent::HandleConnect()
{
    
    if(pthread_mutex_trylock(&start_Connectlock)==0)
    {
        DEBUG_PRINT("Running connect\n");
    
        m_status = E_ConnectionStatus_Connecting;
        
        m_bForceToStop = true;
        m_TcpSocket.CloseSocket();
        if(m_RevThreadID!=0)
        {
            void* ret = NULL;
            pthread_join(m_RevThreadID,&ret);
        }
        m_bForceToStop = false;
        
        /* Catch Signal Handler SIGPIPE */
        signal(SIGPIPE, signal_callback_handler);
        
        pthread_mutex_init(&Cmd_Queuelock, NULL);
        ClearCmdQueue();
        CommandBeforeConnect();
        
        int i32Ret = m_TcpSocket.connectToServer(m_TargerIP, m_TargetPort);
        if(i32Ret!=0)
            m_status = E_ConnectionStatus_DisConnected;
        else
        {
            m_status = E_ConnectionStatus_Connected;
            pthread_create(&m_RevThreadID, NULL,RevThreadFun,NULL);
            
            CommandAfterConnected();
        }
        
        pthread_mutex_unlock(&start_Connectlock);
    }
}
//----------------------------------------------------
void* C_CommandAgent::RevThreadFun(void* lpParam)
{
    g_pCommandAgent->HandleAck();
    
    return 0;
}
//----------------------------------------------------
void C_CommandAgent::HandleAck()
{
    DEBUG_PRINT("Running Rev thread\n");
    
    I_PacketParser *pParcket = GetPacketParser();
    if(!pParcket)
    {
        DEBUG_PRINT("I_PacketParser is NULL!!!");
        Disconnect();
        return;
    }
    
    bool bIsStop = false;
    while(!bIsStop && !m_bForceToStop)
    {
        int i32Ret = pParcket->Parse();
        if(i32Ret==PACKET_DISCONNECTED)
        {
            FireNak();
            break;
        }
        else if(i32Ret==PACKET_GPTAGMISMATCH)
            continue;
        
        pthread_mutex_lock(&Cmd_Queuelock);
        
        if(m_CmdQueue.empty())
        {
            pthread_mutex_unlock(&Cmd_Queuelock);
            continue;
        }
        
        C_StreamCommandBase *pCmd = m_CmdQueue.at(0);
        
        if(i32Ret == PACKET_TIMEOUT)
        {
            struct timeval now;
            gettimeofday(&now, NULL);
            
            DEBUG_PRINT(" Time Out: now %ld send %ld\n",now.tv_sec,pCmd->GetSendTime()->tv_sec);
            if(now.tv_sec - pCmd->GetSendTime()->tv_sec <  pCmd->GetTimeout())
            {
                pthread_mutex_unlock(&Cmd_Queuelock);
                continue;
            }
        }
        
        E_HandleAck_Retcode eRet = pCmd->HandleAck(pParcket);
        switch(eRet)
        {
            case E_HandleAck_Retcode_Failed:
            case E_HandleAck_Retcode_NoError:
            {
                DEBUG_PRINT("Cmd Delete ---- %d\n",pCmd->GetCMDIdex());
                delete pCmd;
                m_CmdQueue.erase(m_CmdQueue.begin());
                m_bIsCmdRunning = false;
            }
                break;
            /*case E_HandleAck_Retcode_Failed:
            {
                DEBUG_PRINT(" C_CommandAgent::HandleAck   ---- %d Retry\n",pCmd->GetCMDIdex());
                
                if(!pCmd->CanRetry())
                {
                    delete pCmd;
                    m_CmdQueue.erase(m_CmdQueue.begin());
                }
                
                m_bIsCmdRunning = false;
            }*/
                break;
            case E_HandleAck_Retcode_RequestMore:
            case E_HandleAck_Retcode_Mismatch:
            {
                
            }
                break;
                
            case E_HandleAck_Retcode_StopConnection:
            {
                bIsStop = true;
                FireNak();
            }
                break;
        }
        pthread_mutex_unlock(&Cmd_Queuelock);
        
        RunCmdQueue();
    };
    
    if(!m_bForceToStop)
        m_status = E_ConnectionStatus_DisConnected;
    
    DEBUG_PRINT("HandleAck() exit\n");

    m_RevThreadID = 0;
}
