//
//  DecodeThread.hpp
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2017/10/3.
//  Copyright © 2017年 generalplus_sa1. All rights reserved.
//

#ifndef DecodeThread_hpp
#define DecodeThread_hpp

#include "CodecDefine.h"
#include "ThreadBase.h"

#include "PictureEncoder.h"
#if defined(__APPLE__)
#include "VideoDecoder_iOS.h"
#else
#include "VideoDecoder_Android.h"
#endif

//----------------------------------------------------------------------
class C_DecodeThread : public C_ThreadBase
{
public:
    C_DecodeThread(C_PlayerInfo *pPlayerInfo);
    ~C_DecodeThread();
    
    int OpenCodec(AVCodecContext *pCodeContext,AVCodecID CodeID,char* ptszOption);
    void onSeek(C_Event &event) {}
    void onClearQueue(C_Event &event)
    {
        m_VideoDecoder.ClearQueue();
        m_pPlayerInfo->SetDecodeSize(m_VideoDecoder.GetDecodeSize());
    }
    
    void onStop(C_Event &event)
    {
        m_VideoDecoder.Close();
    }
    
    void onInputPacket(C_Event &event);
    
    int SaveSnapshot(const char *path)
    {
        strcpy(m_snapShotPath, path);
        m_bSaveSnapshot = true;
        
        return FFMPEGPLAYER_NOERROR;
    }
    
    
    AVFrame* DupDecodedFrame()              { return m_VideoDecoder.DupDecodedFrame();}
    void EnableDebugMessage(bool bEnable)   { m_VideoDecoder.EnableDebugMessage(bEnable); }
    
protected:
    
    virtual int ThreadFunction();
    
private:
    
    int EncodeSnapshot();
    
    C_PictureEncoder    m_SnapshotEncoder;
    C_VideoDecoder      m_VideoDecoder;
    bool                m_bSaveSnapshot;
    char                m_snapShotPath[1024];
};

#endif /* DecodeThread_hpp */
