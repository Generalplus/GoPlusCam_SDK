//
//  PacketParser.cpp
//  GPCam
//
//  Created by generalplus_sa1 on 8/12/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#include "videoPacketParser.h"
#include <string.h>
#include <stdio.h>
#include "CommandAgentDef.h"

//------------------------------------------------------------
C_VideoPacketParser::C_VideoPacketParser(C_TcpSocket* pSocket):
C_PacketParser(pSocket)
{
    
}

C_VideoPacketParser::~C_VideoPacketParser()
{
    
}
//------------------------------------------------------------
int C_VideoPacketParser::Parse()
{
    const char* HeaderTag = GP_SOCK_Tag;
    int i32Ret = PACKET_NOERROR;
    BYTE abyHeader[8];
    
    m_i32PayloadSize = 0;
    m_ErrorCode = 0;
    
    i32Ret = ReadStream(abyHeader, sizeof(abyHeader));
    if(i32Ret!=PACKET_NOERROR)
        return i32Ret;
    
    SAFE_DELETE_ARRAY(m_pbyPayload)
    
    //check gp tag
    for(int i=0;i<8;i++)
    {
        if(abyHeader[i] != HeaderTag[i])
            return PACKET_GPTAGMISMATCH;
    }
    
    //type
    i32Ret = ReadStream(abyHeader, 2);
    if(i32Ret!=PACKET_NOERROR)
        return i32Ret;
    
    UINT16 ui16Type = abyHeader[0] + (abyHeader[1]<<8);
    m_eType = (E_PacketType)ui16Type;

    i32Ret = ReadStream(abyHeader, 2);
    if(i32Ret!=PACKET_NOERROR)
        return i32Ret;
    
    m_byMode = abyHeader[0];
    m_byCMD  = abyHeader[1];
    
    switch(m_eType)
    {
        case E_PacketType_ACK:
        {
            i32Ret = ReadStream(abyHeader, 2);
            if(i32Ret!=PACKET_NOERROR)
                return i32Ret;
            
            m_i32PayloadSize = abyHeader[0] + (abyHeader[1]<<8);
            
            if(m_i32PayloadSize>0)
            {
                m_pbyPayload = new BYTE[m_i32PayloadSize];
                
                i32Ret = ReadStream(m_pbyPayload, m_i32PayloadSize);
                if(i32Ret!=PACKET_NOERROR)
                    return i32Ret;
            }
        }
            break;
        case E_PacketType_NAK:
        {
            i32Ret = ReadStream(abyHeader, 2);
            if(i32Ret!=PACKET_NOERROR)
                return i32Ret;
            
            m_ErrorCode = abyHeader[0] + (abyHeader[1]<<8);
        }
            break;
        default:
            break;
    }
    
    
    return i32Ret;
}
