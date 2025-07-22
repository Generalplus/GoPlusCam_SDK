//
//  PacketParser.h
//  GPCam
//
//  Created by generalplus_sa1 on 8/12/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#ifndef __GPCam__PacketParser__
#define __GPCam__PacketParser__

#include <stdio.h>
#include "GPCommondef.h"
#include "TCPSocket.h"
#include "CommandAgentDef.h"

//---------------------------------------------------------------
class C_PacketParser : public I_PacketParser
{
    
public:
    C_PacketParser(C_TcpSocket* pSocket);
    virtual ~C_PacketParser();
    
    virtual int Parse();
    
    
    int          GetPayloadSize()   { return m_i32PayloadSize;}
    BYTE*        GetPayload()       { return m_pbyPayload;}
    short        GetErrorCode()     { return m_ErrorCode;}
    E_PacketType GetType()          { return m_eType;}
    BYTE         GetMode()          { return m_byMode;}
    BYTE         GetCMD()           { return m_byCMD;}
    
protected:
    
    int          ReadStream(BYTE * message , int i32Size);
    
    int           m_i32PayloadSize;
    BYTE*         m_pbyPayload;
    short         m_ErrorCode;
    BYTE          m_byMode;
    BYTE          m_byCMD;
    
    E_PacketType  m_eType;
    C_TcpSocket  *m_pSocket;
};





#endif /* defined(__GPCam__PacketParser__) */
