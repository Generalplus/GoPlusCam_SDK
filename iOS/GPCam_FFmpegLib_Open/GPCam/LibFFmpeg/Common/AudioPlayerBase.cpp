//
//  AudioPlayerBase.cpp
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2017/5/16.
//  Copyright © 2017年 generalplus_sa1. All rights reserved.
//

#include "AudioPlayerBase.h"
#define RAMP_TIME               0.200

//--------------------------------------------------------------------------
C_AudioPlayerBase::C_AudioPlayerBase():
m_i32SampleBit(16),
m_i32CHCnt(2),
m_fSampleRate(44100.00),
m_IsInited(false),
m_pCurrentFrame(NULL),
m_i32CurrentIndex(0),
m_i32PlayRate(1)
{
    SET_EVENT_HANDLER(E_EventType_Player_Pause,C_AudioPlayerBase,onPause)
    SET_EVENT_HANDLER(E_EventType_Player_Play,C_AudioPlayerBase,onPlay)
    SET_EVENT_HANDLER(E_EventType_Player_Seek,C_AudioPlayerBase,onSeek)
    SET_EVENT_HANDLER(E_EventType_Player_Stop,C_AudioPlayerBase,onStop)
    SET_EVENT_HANDLER(E_EventType_Decode_AudioData,C_AudioPlayerBase,onAudioData)
    SET_EVENT_HANDLER(E_EventType_Audio_RampUp,C_AudioPlayerBase,onAudioRampUp)
    SET_EVENT_HANDLER(E_EventType_Audio_RampDown,C_AudioPlayerBase,onAudioRampDown)
}
C_AudioPlayerBase::~C_AudioPlayerBase()
{
    Uninit();
}//--------------------------------------------------------------------------
int C_AudioPlayerBase::Init(Float64 fSampleRate,int i32CHCnt)
{
    m_fSampleRate = fSampleRate;
    m_i32CHCnt = i32CHCnt;
    
    m_RampInfo.eType = E_RampType_None;
    m_RampInfo.fStep = 1.0 / (float)(fSampleRate * RAMP_TIME);
    m_RampInfo.fCurrent = 0;
    
    return 0;
}
//--------------------------------------------------------------------------
void C_AudioPlayerBase::FillInFrame(short *pBuffer, UInt32 inNumberFrames)
{
    S_CoreAudioFrame *pFrame = NULL;
    
    UInt32 i32Index = 0;
    UInt32 i32total = inNumberFrames;
    
    int64_t i64SampleIndex = -1;
    
    if(m_pCurrentFrame)
    {
        i64SampleIndex = CopyFrameData(pBuffer,&inNumberFrames,&i32Index,&m_pCurrentFrame);
    }
    
    while(inNumberFrames>0)
    {
        pFrame = NULL;
        for(int i = 0; i < m_i32PlayRate && m_AudioQueue.GetSize() >0 ; i++)
        {
            
            FreeCoreAudioFrame(pFrame);
            pFrame = m_AudioQueue.PopObject();
            if(pFrame)
            {
                if(pFrame->pData ==NULL) //workaround iOS crash during long-term playing
                    pFrame = NULL;
            }
        }
        
        if(pFrame)
        {
            m_i32CurrentIndex = 0;
            i64SampleIndex = CopyFrameData(pBuffer,&inNumberFrames,&i32Index,&pFrame);
            m_pCurrentFrame = pFrame;
        }
        else
            break;
    }
    
    DoRamp(pBuffer,i32total);
    
    if(i64SampleIndex!=-1 && m_RampInfo.eType!=E_RampType_Down)
    {
        C_Event event(E_EventType_Audio_PlayedSample,(void*)&i64SampleIndex);
        C_EventManager::GetEvnetManager()->ProcessEvent(event);
    }
}
//--------------------------------------------------------------------------
int64_t C_AudioPlayerBase::CopyFrameData(short *pBuffer ,UInt32 *pinNumberFrames, UInt32 *pBuffIndex,S_CoreAudioFrame **ppCurrentFrame)
{
    int64_t i64SampleIndex = (*ppCurrentFrame)->i64SampleIndex;
    
    int i32PlayCount = ((*ppCurrentFrame)->i32SampleCount > *pinNumberFrames)? *pinNumberFrames: (*ppCurrentFrame)->i32SampleCount;
    int i32BytePerFrame =  (m_i32SampleBit/8) * m_i32CHCnt;
    
    short* pbuffer= (pBuffer + *pBuffIndex);
    memcpy(pbuffer, &(*ppCurrentFrame)->pData[0][m_i32CurrentIndex], i32PlayCount * i32BytePerFrame);
    
    *pinNumberFrames-=i32PlayCount;
    *pBuffIndex+=(i32PlayCount * m_i32CHCnt);
    
    m_i32CurrentIndex+=(i32PlayCount * i32BytePerFrame);
    (*ppCurrentFrame)->i32SampleCount-=i32PlayCount;
    (*ppCurrentFrame)->i64SampleIndex+=i32PlayCount;
    if((*ppCurrentFrame)->i32SampleCount==0)
    {
        FreeCoreAudioFrame((*ppCurrentFrame));
        (*ppCurrentFrame) = NULL;
        
        m_i32CurrentIndex = 0;
    }
    
    return i64SampleIndex + i32PlayCount;
}
//--------------------------------------------------------------------------
void C_AudioPlayerBase::AddFrame(
                                 uint8_t **pData,
                                 int      i32SampleCount,
                                 int64_t  i64SampleIndex
                                 )
{
    S_CoreAudioFrame *pNew = new S_CoreAudioFrame;
  
    pNew->pData = pData;
    pNew->i32SampleCount    = i32SampleCount;
    pNew->i32Total          = i32SampleCount;
    pNew->i64SampleIndex    = i64SampleIndex;
    
    m_AudioQueue.PushOject(pNew);
    
}
//--------------------------------------------------------------------------
void C_AudioPlayerBase::FreeCoreAudioFrame(S_CoreAudioFrame *pFrame)
{
    if(pFrame)
    {
        if(pFrame->pData)
            av_freep(&pFrame->pData[0]);
        
        av_freep(&pFrame->pData);
        pFrame->pData = NULL;
        delete pFrame;
    }
}
//--------------------------------------------------------------------------
void C_AudioPlayerBase::DoRamp(short *pBuffer , UInt32 i32Sample)
{
    switch(m_RampInfo.eType)
    {
        case E_RampType_Up:
        {
            
            //DEBUG_PRINT("E_RampType_Up:%d \n",i32Sample);
            for(int i=0;i<i32Sample && m_RampInfo.fCurrent<=1;i++,m_RampInfo.fCurrent+=m_RampInfo.fStep)
            {
                for(int j=0;j<m_i32CHCnt;j++)
                    pBuffer[m_i32CHCnt*i+j] = pBuffer[m_i32CHCnt*i+j] * m_RampInfo.fCurrent;
            }
            
            if(m_RampInfo.fCurrent >= 1)
                m_RampInfo.eType = E_RampType_None;
        }
            break;
        case E_RampType_Down:
        {
            //DEBUG_PRINT("E_RampType_Down:%d \n",i32Sample);
            for(int i=0;i<i32Sample;i++)
            {
                for(int j=0;j<m_i32CHCnt;j++)
                    pBuffer[m_i32CHCnt*i+j] = pBuffer[m_i32CHCnt*i+j] * m_RampInfo.fCurrent;

                if(m_RampInfo.fCurrent>0)
                {
                    m_RampInfo.fCurrent-=m_RampInfo.fStep;
                    if(m_RampInfo.fCurrent<0)
                        m_RampInfo.fCurrent = 0;
                }
            }


        }
            break;
        default:
            break;
    }
}
//--------------------------------------------------------------------------
void C_AudioPlayerBase::onSeek(C_Event &event)
{
    
}
//--------------------------------------------------------------------------
void C_AudioPlayerBase::onStop(C_Event &event)
{
     m_AudioQueue.ClearQueue();
    Uninit();
}
//--------------------------------------------------------------------------
void C_AudioPlayerBase::onAudioData(C_Event &event)
{
    S_Decode_AudioData *pData = (S_Decode_AudioData *)event.GetData();
    if(!m_IsInited)
    {
        Init(pData->i32SampleRate,pData->i32Channel);
        Play();
    }
    
    AddFrame(pData->pData,pData->i32SampleCnt,pData->i64SampleIndex);
}
//--------------------------------------------------------------------------
void C_AudioPlayerBase::onAudioRampUp(C_Event &event)
{
    m_AudioQueue.ClearQueue();
    DoRampUp();
}
//--------------------------------------------------------------------------
void C_AudioPlayerBase::onAudioRampDown(C_Event &event)
{
    DoRampDown();
}
//--------------------------------------------------------------------------
void C_AudioPlayerBase::onPause(C_Event &event)
{
    SetAudioRate(0);
}
//--------------------------------------------------------------------------
void C_AudioPlayerBase::onPlay(C_Event &event)
{
    SetAudioRate(1);
}
//--------------------------------------------------------------------------

