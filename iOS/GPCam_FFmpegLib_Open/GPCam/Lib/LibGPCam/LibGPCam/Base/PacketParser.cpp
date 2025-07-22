//
//  PacketParser.cpp
//  GPCam
//
//  Created by generalplus_sa1 on 8/12/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#include "PacketParser.h"
#include <string.h>
#include <stdio.h>
#include "CommandAgentDef.h"

//------------------------------------------------------------
C_PacketParser::C_PacketParser(C_TcpSocket* pSocket):
m_pSocket(pSocket),
m_i32PayloadSize(0),
m_pbyPayload(NULL),
m_ErrorCode(0),
m_byMode(0),
m_byCMD(0)
{
    
}

C_PacketParser::~C_PacketParser()
{
    SAFE_DELETE_ARRAY(m_pbyPayload)
}
//------------------------------------------------------------
int C_PacketParser::ReadStream(BYTE * message , int i32Size)
{
    int i32Read;
    
    i32Read = m_pSocket->readStream(message, i32Size);
    if(i32Read<=0)
    {
        if(i32Read == Error_Socket_Timeout)
        {
            m_eType = E_PacketType_NAK;
            m_ErrorCode = Error_RequestTimeOut;
            return PACKET_TIMEOUT;
            
        }
        else
            return PACKET_DISCONNECTED;
    }
    
    return PACKET_NOERROR;
}
//------------------------------------------------------------
int C_PacketParser::Parse()
{
    DEBUG_PRINT("C_PacketParser::Parse not implemented!!!\n");
    return PACKET_NOERROR;
}
