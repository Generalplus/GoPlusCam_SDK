#ifndef _STREAMBASE_
#define _STREAMBASE_

#include <sys/time.h>
#include "GPCommondef.h"
#include "CommandAgentDef.h"

#define DEFAULT_RETRY   2

typedef enum 
{
	E_HandleAck_Retcode_NoError 		,
	E_HandleAck_Retcode_RequestMore 	,
	E_HandleAck_Retcode_Failed		 	,
	E_HandleAck_Retcode_Mismatch		,
    E_HandleAck_Retcode_StopConnection	,
}E_HandleAck_Retcode;


typedef enum
{
    E_CommandType_Set ,
    E_CommandType_Get ,
    E_CommandType_Action
    
}E_CommandType;

//--------------------------------------------------------------
class C_StreamCommandBase : public I_StreamCommand
{

public:

	C_StreamCommandBase( I_CommandAgent* pCommandAgnet,PFN_CmdStatusCallBack StatusCallBack,E_CommandType Type);
	virtual ~C_StreamCommandBase();

    E_CommandType GetCommandType() { return m_CommandType; }
	BYTE*   GetPacketStream() { if(m_pbyStream) return m_pbyStream; else return NULL; }
    int     GetStreamSize()   { return m_i32StreamSize;}

    BYTE    GetMode() { if(m_pbyStream) return m_pbyStream[10]; else return 0x00; }
    BYTE    GetCMD()  { if(m_pbyStream) return m_pbyStream[11]; else return 0x00; }
    unsigned short GetCMDIdex() { return m_u16CmdIndex; }
    struct timeval* GetSendTime(){ return &m_SendTime; }
    bool    CanRetry(){
                m_i32Retry--;
                if(m_i32Retry==0)
                    return false;
                else
                    return true;
            }
    
    
    //virtual functions
    virtual E_HandleAck_Retcode HandleAck(I_PacketParser *packet);
    virtual void  SetPacketload(UINT16 u16Type,BYTE byModeID,BYTE byCMDID, BYTE *pbyPayload,int i32PayloadSize);
    virtual unsigned int GetTimeout() { return DEFAULT_READ_TIMEOUT; }
    virtual void  DoAction() { }
    
protected:

    PFN_CmdStatusCallBack   m_CmdStatuCallBack;
    I_CommandAgent*         m_pCommnadAgent;
    
	BYTE*                   m_pbyStream;
    int                     m_i32StreamSize;
    int                     m_i32Retry;
    E_CommandType           m_CommandType;
    unsigned short          m_u16CmdIndex;
    struct timeval          m_SendTime;
};
//--------------------------------------------------------------
#endif