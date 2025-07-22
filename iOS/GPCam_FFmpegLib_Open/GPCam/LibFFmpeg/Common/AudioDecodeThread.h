//
//  AudioDecodeThread.hpp
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2017/10/3.
//  Copyright © 2017年 generalplus_sa1. All rights reserved.
//

#ifndef AudioDecodeThread_hpp
#define AudioDecodeThread_hpp

#include "CodecDefine.h"
#include "ThreadBase.h"

#include "AudioDecoder.h"
//----------------------------------------------------------------------
class C_AudioDecodeThread : public C_ThreadBase
{
public:
    C_AudioDecodeThread(C_PlayerInfo *pPlayerInfo);
    ~C_AudioDecodeThread();
  
    int OpenCodec(AVCodecContext *pCodeContext,AVCodecID CodeID,char* ptszOption);
    void onSeek(C_Event &event) {}
    void onClearQueue(C_Event &event)
    {
        m_AudioDecoder.ClearQueue();
    }
    
    void onStop(C_Event &event)
    {
        m_AudioDecoder.Close();
    }
    
    void onInputPacket(C_Event &event);
    
protected:
    
    virtual int ThreadFunction();
    
private:
    
    C_AudioDecoder  m_AudioDecoder;
};


#endif /* AudioDecodeThread_hpp */
