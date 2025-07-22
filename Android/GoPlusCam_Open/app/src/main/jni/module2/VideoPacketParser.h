//
//  PacketParser.h
//  GPCam
//
//  Created by generalplus_sa1 on 8/12/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#ifndef __GPCam__VideoPacketParser__
#define __GPCam__VideoPacketParser__

#include "PacketParser.h"
//---------------------------------------------------------------
class C_VideoPacketParser : public C_PacketParser
{
    
public:
    C_VideoPacketParser(C_TcpSocket* pSocket);
    virtual ~C_VideoPacketParser();
    
    int Parse();
};


#endif /* defined(__GPCam__PacketParser__) */
