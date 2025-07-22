//
//  InputTreadBase.h
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2017/10/23.
//  Copyright © 2017年 generalplus_sa1. All rights reserved.
//

#ifndef InputTreadBase_h
#define InputTreadBase_h

#include "CodecDefine.h"
#include "ThreadBase.h"

//----------------------------------------------------------------------
class C_InputThreadBase : public C_ThreadBase
{
public:
    C_InputThreadBase(C_PlayerInfo *pPlayerInfo):C_ThreadBase(pPlayerInfo){}
    ~C_InputThreadBase(){}
    
    
    virtual void Seek(int64_t position){}
    virtual int Pause(){return FFMPEGPLAYER_NOERROR;}
    virtual int Resume(){return FFMPEGPLAYER_NOERROR;}
    
    virtual AVCodecContext  *GetCodecContext(AVMediaType StreamType){ return NULL; }
    
    virtual char* GetPath() { return ""; }
    virtual void Close() {}
    
};


#endif /* InputTreadBase_h */
