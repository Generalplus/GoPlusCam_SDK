#include "StreamCommandBase.h"
#include <stdio.h>
#include <string.h>

static unsigned short g_u16CmdIndex=0;

//--------------------------------------------------------------
C_StreamCommandBase::C_StreamCommandBase(I_CommandAgent* pCommandAgnet,PFN_CmdStatusCallBack StatusCallBack,E_CommandType Type):
m_pCommnadAgent(pCommandAgnet),
m_pbyStream(NULL),
m_CmdStatuCallBack(StatusCallBack),
m_CommandType(Type),
m_i32Retry(DEFAULT_RETRY)
{
    m_u16CmdIndex = g_u16CmdIndex;
    g_u16CmdIndex++;
}
//--------------------------------------------------------------
C_StreamCommandBase::~C_StreamCommandBase()
{
    SAFE_DELETE_ARRAY(m_pbyStream)
}
//--------------------------------------------------------------
E_HandleAck_Retcode C_StreamCommandBase::HandleAck(I_PacketParser *packet)
{
    if(packet->GetType() == GP_SOCK_TYPE_NAK)
    {
        short ErrorCode = packet->GetErrorCode();
        if(m_CmdStatuCallBack)
            m_CmdStatuCallBack(GetCMDIdex(),packet->GetType(),GetMode(),GetCMD(),sizeof(ErrorCode),(BYTE*)&ErrorCode);
        
        return E_HandleAck_Retcode_Failed;
    }
    else if(packet->GetMode() != GetMode() || packet->GetCMD() != GetCMD())
    {
        
        DEBUG_PRINT("Ack mismatch! Mode: %d =! %d CMD: %d != %d\n",packet->GetMode(),GetMode(),packet->GetCMD(),GetCMD());
        
        return E_HandleAck_Retcode_Mismatch;
    }
    else if(m_CommandType == E_CommandType_Set)
    {
        if(m_CmdStatuCallBack)
            m_CmdStatuCallBack(GetCMDIdex(),packet->GetType(),GetMode(),GetCMD(),packet->GetPayloadSize(),packet->GetPayload());
    }
    
    return E_HandleAck_Retcode_NoError;
}
//--------------------------------------------------------------
void C_StreamCommandBase::SetPacketload(UINT16 u16Type,BYTE byModeID,BYTE byCMDID, BYTE *pbyPayload,int i32PayloadSize)
{
    DEBUG_PRINT("C_StreamCommandBase:::SetPacketload not implemented!!!\n");
}
