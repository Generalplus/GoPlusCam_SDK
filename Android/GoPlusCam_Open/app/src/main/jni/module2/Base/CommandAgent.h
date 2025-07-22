//
//  CommandAgent.h
//  GPCam
//
//  Created by generalplus_sa1 on 8/11/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#ifndef __GPCam__CommandAgent__
#define __GPCam__CommandAgent__

#include <stdio.h>
#include <pthread.h>
#include "TCPSocket.h"
#include <vector>
#include "StreamCommandBase.h"
#include <string>
#include "CommandAgentDef.h"

//---------------------------------------------------
#ifndef Create_Agent_Helper
#define	Create_Agent_Helper(AgentClass)\
\
C_CommandAgent *g_pCommandAgent = NULL;\
class C_AgentAllocHelper\
{\
public:\
    C_AgentAllocHelper()\
    {\
        g_pCommandAgent = new AgentClass();\
    }\
    ~C_AgentAllocHelper()\
    {\
        if(g_pCommandAgent)\
            delete g_pCommandAgent;\
    }\
};\
static C_AgentAllocHelper g_AllocHelper;\
\
AgentClass* AgentClass::GetShareAgent()\
{\
    return (AgentClass*)g_pCommandAgent;\
}\

#endif

//---------------------------------------------------
typedef std::vector<C_StreamCommandBase*> StreamCmdVector;
typedef std::vector<C_StreamCommandBase*>::iterator StreamCmdVector_iterator;
//---------------------------------------------------

class C_CommandAgent : public I_CommandAgent
{
public:
    C_CommandAgent();
    virtual ~C_CommandAgent();
    
    int ConnectToDevice(
        LPCTSTR pszIPAddress,
        int i32PortNum);
    
    void Disconnect();
    
    void SetCmdStatusCallBack(PFN_CmdStatusCallBack CallBack);
    void ClearCmdQueue();
    
    int Abort(int i32Index);
    int SendCommand(C_StreamCommandBase* pCommand);
    
    E_ConnectionStatus  GetStatus();
    
    void SetStatus(E_ConnectionStatus eStatus);
    
    static void* RevThreadFun(void* lpParam);
    void HandleAck();
    
    static void* ConnectThreadFun(void* lpParam);
    void HandleConnect();
    
    void RunCmdQueue();
    void RunCommand();
    
    
    void FireNak();

    // C_CommandAgent virtual
    virtual bool CommandBeforeConnect() { return true; }
    virtual bool CommandAfterConnected() { return true; }
    virtual I_PacketParser* GetPacketParser() { return NULL; }
    
    //I_CommandAgent interface
    virtual void QueueCmd(I_StreamCommand* pCommand);
    
    //Debug
    void    SetDataCallBack(PFN_SocketDataCallBack CallBack) { m_TcpSocket.SetDataCallBack(CallBack); }
    
protected:
    
    C_TcpClient             m_TcpSocket;
    E_ConnectionStatus      m_status;
    pthread_t               m_RevThreadID;
    
    pthread_t               m_ConnectThreadID;
    char                    m_TargerIP[256];
    int                     m_TargetPort;
    PFN_CmdStatusCallBack   m_CmdStatuCallBack;
    StreamCmdVector         m_CmdQueue;
    
    bool                    m_bIsCmdRunning;
    bool                    m_bForceToStop;

};





#endif /* defined(__GPCam__CommandAgent__) */
