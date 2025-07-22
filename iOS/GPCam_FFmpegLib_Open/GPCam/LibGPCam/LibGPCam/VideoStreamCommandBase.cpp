#include "VideoStreamCommandBase.h"
#include "VideoCommandAgentDef.h"
#include <stdio.h>
#include <string.h>


//--------------------------------------------------------------
C_VideoSteamCommmadBase::C_VideoSteamCommmadBase(I_CommandAgent* pCommandAgnet,I_ViedoFileAgent *pFileAgent,PFN_CmdStatusCallBack StatusCallBack,E_CommandType Type):
C_StreamCommandBase(pCommandAgnet,StatusCallBack,Type),
m_pFileAgent(pFileAgent)
{

}
//--------------------------------------------------------------
C_VideoSteamCommmadBase::~C_VideoSteamCommmadBase()
{
}
//--------------------------------------------------------------
void C_VideoSteamCommmadBase::SetPacketload(UINT16 u16Type,BYTE byModeID,BYTE byCMDID, BYTE *pbyPayload,int i32PayloadSize)
{
    SAFE_DELETE_ARRAY(m_pbyStream)
    
    m_i32StreamSize = i32PayloadSize + GPSOCK_HEADER_SIZE;
    m_pbyStream	= new BYTE[m_i32StreamSize];

    memcpy(m_pbyStream,GP_SOCK_Tag,8);
    memcpy(&m_pbyStream[8],&u16Type,sizeof(u16Type));
    
    m_pbyStream[10] = byModeID;
    m_pbyStream[11] = byCMDID;

    if(i32PayloadSize>0)
        memcpy(&m_pbyStream[GPSOCK_HEADER_SIZE],pbyPayload,i32PayloadSize);
    
    m_i32StreamSize = i32PayloadSize + GPSOCK_HEADER_SIZE;
    
}
