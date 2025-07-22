//
//  DisplayThread.hpp
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2017/10/3.
//  Copyright © 2017年 generalplus_sa1. All rights reserved.
//

#ifndef DisplayThread_hpp
#define DisplayThread_hpp

#include "CodecDefine.h"
#include "ThreadBase.h"
//----------------------------------------------------------------------
class C_DisplayThread : public C_ThreadBase
{
public:
    C_DisplayThread(C_PlayerInfo *pPlayerInfo);
    ~C_DisplayThread();
    
    void Init(AVCodecContext* pVideoCtx);
    
protected:
    
    int syncFrame(AVFrame* pFrame);
    int displayFrame(AVFrame* pFrame);
    
    AVFrame  *PopDisplay()      {  return m_DisplayFrameQueue.PopObject(true); }
    int GetDisplaySize()    { return m_DisplayFrameQueue.GetSize(); }
    
    virtual int ThreadFunction();
    
    void onClearQueue(C_Event &event)
    {
        m_DisplayFrameQueue.ClearQueue();
        m_pPlayerInfo->SetDisplaySize(m_DisplayFrameQueue.GetSize());
    }
    
    void onStop(C_Event &event)
    {
        m_DisplayFrameQueue.ClearQueue();
        m_DisplayFrameQueue.UnLockQueue();
    }
    
    void onVideoFrame(C_Event &event);
    
    void UnLockQueue()
    {
        m_DisplayFrameQueue.UnLockQueue();
    }
    
    void ClearQueue()
    {
        m_DisplayFrameQueue.ClearQueue();
    }
    
    
private:
    
    C_FrameQueue        m_DisplayFrameQueue;
    struct SwsContext   *m_sws_ctx;
    AVFrame             *m_covertFrame;
    
};

#endif /* DisplayThread_hpp */
