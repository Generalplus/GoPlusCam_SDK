#ifndef _VIDEOSTREAMBASE_
#define _VIDEOSTREAMBASE_

#include "StreamCommandBase.h"
#include "VideoCommandAgentDef.h"

//--------------------------------------------------------------
class C_VideoSteamCommmadBase : public C_StreamCommandBase
{

public:

	C_VideoSteamCommmadBase( I_CommandAgent* pCommandAgnet,I_ViedoFileAgent *pFileAgent,PFN_CmdStatusCallBack StatusCallBack,E_CommandType Type);
	virtual ~C_VideoSteamCommmadBase();
    
    virtual void  SetPacketload(UINT16 u16Type,BYTE byModeID,BYTE byCMDID, BYTE *pbyPayload,int i32PayloadSize);
    
protected:
    
    I_ViedoFileAgent        *m_pFileAgent;
    
};
//--------------------------------------------------------------
#endif