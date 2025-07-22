//
//  AudioPlayerBase.h
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2017/5/16.
//  Copyright © 2017年 generalplus_sa1. All rights reserved.
//

#ifndef AudioPlayerBase_hpp
#define AudioPlayerBase_hpp

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <vector>
#include "Defines.h"
#include "EventManager.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <libavutil/mem.h>
#ifdef __cplusplus
}
#endif

//----------------------------------------------------------------------

#if defined(ANDROID) || defined(__ANDROID__)
    #define UInt32  unsigned int
    #define Float64 double
#else
    #include <MacTypes.h>
#endif
//----------------------------------------------------------------------
enum E_RampType
{
    E_RampType_None = 0,
    E_RampType_Up ,
    E_RampType_Down
};

typedef struct AudioRampInfo
{
    E_RampType  eType;
    float       fCurrent;
    float       fStep;
    
}S_AudioRampInfo;
//----------------------------------------------------------------------
typedef struct CoreAudioFrame
{
    uint8_t **pData;
    int      i32SampleCount;
    int      i32Total;
    int64_t  i64SampleIndex;
    
}S_CoreAudioFrame;

#include "QueueTemplate.h"
class C_CoreAudioQueue : public T_Queue<S_CoreAudioFrame>
{
public:
    C_CoreAudioQueue(){}
    ~C_CoreAudioQueue(){}
    
    
    virtual void FreeObjectContent(S_CoreAudioFrame* pObject)
    {
        if(pObject->pData)
            av_freep(&pObject->pData[0]);
        
         av_freep(&pObject->pData);
        pObject->pData = NULL;
    }
};

//----------------------------------------------------------------------
class C_AudioPlayerBase
{
    
public:
    
    C_AudioPlayerBase();
    ~C_AudioPlayerBase();

    virtual int     Init(Float64 fSampleRate,int i32CHCnt);
    virtual int     Uninit(){ return 0;}
    
    virtual int     Play(){ return 0;}
    virtual int     Stop(){ return 0;}
    virtual int     Pause(){ return 0;}
    virtual int     Resume(){ return 0;}
    
protected:
    
    void    DoRampDown()
    {
        m_RampInfo.eType = E_RampType_Down;
        m_RampInfo.fCurrent = 1.0;
    }
    
    void    DoRampUp()
    {
        m_RampInfo.eType = E_RampType_Up;
        m_RampInfo.fCurrent = 0.0;
    }
    
    void    SetAudioRate(int i32Rate)
    {
        m_i32PlayRate = i32Rate;
    }
    
    void onPause(C_Event &event);
    void onPlay(C_Event &event);
    void onSeek(C_Event &event);
    void onStop(C_Event &event);
    void onAudioData(C_Event &event);
    void onAudioRampUp(C_Event &event);
    void onAudioRampDown(C_Event &event);
    
    void    AddFrame(
                     uint8_t **pData,
                     int      i32SampleCount,
                     int64_t  i64SampleIndex
                     );
    
    void FillInFrame(short *pBuffer, UInt32 inNumberFrames);
    int64_t CopyFrameData(short *pBuffer,UInt32 *pinNumberFrames, UInt32 *pBuffIndex,S_CoreAudioFrame **ppCurrentFrame);
    void FreeCoreAudioFrame(S_CoreAudioFrame *pFrame);
    
    void DoRamp(short *pBuffer , UInt32 i32Sample);
    
    Float64                     m_fSampleRate;
    int                         m_i32SampleBit;
    int                         m_i32CHCnt;
    bool                        m_IsInited;
    
    S_AudioRampInfo             m_RampInfo;
    S_CoreAudioFrame            *m_pCurrentFrame;
    int                         m_i32CurrentIndex;
    int                         m_i32PlayRate;
    
    C_CoreAudioQueue            m_AudioQueue;
};


#endif /* AudioPlayerBase_hpp */
