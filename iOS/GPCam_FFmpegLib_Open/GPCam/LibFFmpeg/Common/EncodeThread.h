//
//  EncodeThread.hpp
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2017/10/3.
//  Copyright © 2017年 generalplus_sa1. All rights reserved.
//

#ifndef EncodeThread_hpp
#define EncodeThread_hpp

#include "VideoEncoder.h"
#include "CodecDefine.h"
#include "ThreadBase.h"
//----------------------------------------------------------------------
class C_EncodeThread : public C_ThreadBase
{
public:
    C_EncodeThread(C_PlayerInfo *pPlayerInfo);
    ~C_EncodeThread();
    
    void SetStop()
    {
        m_bSaveVideo = false;
    }
    
    void Init(AVCodecContext *pVideoCodecCtx,
              AVCodecContext *pAudioCodecCtx)
    {
        m_VideoEncoder.init(pVideoCodecCtx, pAudioCodecCtx);
    }
    
    void SetPath(const char* ptszSavePath,char* ptszStreamPath)
    {
        strcpy(m_VideoPath, ptszSavePath);
        strcpy(m_MediaPath, ptszStreamPath);
    }
    
    void SetEncodeUsingLocalTime(bool bEnable) { m_bEncodeUsingLocalTime = bEnable; }
    void SetForceToTranscode(bool bForce)      { m_bForceToTransCode = bForce; }
    
    E_EncodeContainer GetContainerType() { return m_VideoEncoder.GetContainerType();}
 
    void onStop(C_Event &event)
    {

    }
    
    void onClearQueue(C_Event &event)
    {
        m_VideoEncoder.ClearQueue();
    }
    
protected:
    
    void onInputPacket(C_Event &event);
    void onVideoFrame(C_Event &event);
    
    virtual int ThreadFunction();
    
private:
    
    bool                m_bSaveVideo;
    bool                m_bEncodeUsingLocalTime;
    bool                m_bForceToTransCode;
    
    char                m_MediaPath[1024];
    char                m_VideoPath[1024];
    char                m_TransCodeOptions[1024];
    AVCodecID           m_EncodeID;
    C_VideoEncoder      m_VideoEncoder;
};

#endif /* EncodeThread_hpp */
