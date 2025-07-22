//
//  CustomInputThread.h
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2017/10/23.
//  Copyright © 2017年 generalplus_sa1. All rights reserved.
//

#ifndef CustomInputThread_h
#define CustomInputThread_h

#include "CodecDefine.h"
#include "InputTreadBase.h"
//----------------------------------------------------------------------
class C_CustomInputThread : public C_InputThreadBase
{
public:
    C_CustomInputThread(C_PlayerInfo *pPlayerInfo);
    ~C_CustomInputThread();
    
    bool Init(const char* VideoCode, const char* AudioCode , const char* pOptions)
    {
        if(pOptions)
            strcpy(m_UserOptions,pOptions);
        
        strncpy(m_VideoCodecName,VideoCode,16);
        strncpy(m_AudioCodecName,AudioCode,16);
        
        return true;
    }
    
    int PushPacket(const unsigned char* pbyData,int i32Size,int i32Type,int64_t lTimeStamp);
    
    AVCodecContext  *GetCodecContext(AVMediaType StreamType);
    void Close();
    
protected:
    
    virtual int ThreadFunction();
    
private:
    void SetupTrackInfo(E_Track track,AVMediaType type);
    
    int _Init();
    AVRational GetFrameRate();
    AVCodecContext      *m_pCodecContext;
    AVCodecContext      *m_pAudioCodecContext;

    char                m_VideoCodecName[16];
    char                m_AudioCodecName[16];
    
    char                m_UserOptions[1024];
    pthread_mutex_t     m_Contextlock;
};


#endif /* CustomInputThread_h */
