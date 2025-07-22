//
//  InputThread.hpp
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2017/10/3.
//  Copyright © 2017年 generalplus_sa1. All rights reserved.
//

#ifndef InputThread_hpp
#define InputThread_hpp

#include "CodecDefine.h"
#include "InputTreadBase.h"
//----------------------------------------------------------------------
class C_InputThread : public C_InputThreadBase
{
public:
    C_InputThread(C_PlayerInfo *pPlayerInfo);
    ~C_InputThread();
    
    bool Init(const char* path,const char* pOptions);
    
    void Seek(int64_t position);
    int Pause();
    int Resume();
    
    AVCodecContext  *GetCodecContext(AVMediaType StreamType);
    
    char* GetPath() { return m_MediaPath; }
    void Close();
    
protected:
    
    virtual int ThreadFunction();
    
private:
    void SetupTrackInfo(E_Track track,AVMediaType type);
    
    int _Init();
    AVRational GetFrameRate();
    void SeekInternal(int64_t position);
    void inputDelay(int64_t prevTime,int64_t lastVideoTime,int64_t lastAudioTime);
    bool seekingFrame(AVPacket *packet);
    
    AVFormatContext     *m_formatCtx;
    
    char                m_MediaPath[1024];
    char                m_UserOptions[1024];
    
    pthread_mutex_t     m_Contextlock;
};


#endif /* InputThread_hpp */
