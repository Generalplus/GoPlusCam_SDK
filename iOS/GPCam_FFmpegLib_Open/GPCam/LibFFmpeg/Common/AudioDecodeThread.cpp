//
//  AudioDecodeThread.cpp
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2017/10/3.
//  Copyright © 2017年 generalplus_sa1. All rights reserved.
//

#include "AudioDecodeThread.h"
//----------------------------------------------------------------------
C_AudioDecodeThread::C_AudioDecodeThread(C_PlayerInfo *pPlayerInfo):
C_ThreadBase(pPlayerInfo),m_AudioDecoder(pPlayerInfo)
{
    SET_EVENT_HANDLER(E_EventType_Input_Packet,C_AudioDecodeThread,onInputPacket)
    SET_EVENT_HANDLER(E_EventType_Player_Seek,C_AudioDecodeThread,onSeek)
    SET_EVENT_HANDLER(E_EventType_Queue_Clear,C_AudioDecodeThread,onClearQueue)
    SET_EVENT_HANDLER(E_EventType_Player_Stop,C_AudioDecodeThread,onStop)
}
//----------------------------------------------------------------------
C_AudioDecodeThread::~C_AudioDecodeThread()
{
    
}
//----------------------------------------------------------------------
int C_AudioDecodeThread::OpenCodec(AVCodecContext *pCodeContext,AVCodecID CodeID,char* ptszOption)
{
    int i32Ret = m_AudioDecoder.OpenCodec(pCodeContext, CodeID, ptszOption);
    if(i32Ret == FFMPEGPLAYER_NOERROR)
    {
        m_AudioDecoder.initAudioResample();
    }
    
    return i32Ret;
}
//----------------------------------------------------------------------
void C_AudioDecodeThread::onInputPacket(C_Event &event)
{
    if(!IsRunning())
        return ;
    
    AVPacket *pPacket = (AVPacket *)event.GetData();
    if(pPacket->stream_index == m_pPlayerInfo->GetTrackInfo(E_TrackAudio)->i32StreamID)
    {
        AVPacket *tempkt = new AVPacket;
        av_init_packet(tempkt);
        av_copy_packet(tempkt, pPacket);
        m_AudioDecoder.pushDecodePacket(tempkt);
    }
}
//----------------------------------------------------------------------
int C_AudioDecodeThread::ThreadFunction()
{
    
    while(m_pPlayerInfo->GetStatus() ==E_PlayerStatus_Playing)
    {
        CheckAndDoPause();
        if(m_pPlayerInfo->GetStatus()!= E_PlayerStatus_Playing)
            break;
        
        AVPacket *pPacket = m_AudioDecoder.PopDecodePacket();
        if(pPacket == NULL)
            continue;
        
        int64_t timeBefore = GetClock();
        
        int64_t pts = av_rescale_q (pPacket->pts,
                                   m_pPlayerInfo->GetTrackInfo(E_TrackAudio)->time_base,
                                   AV_TIME_BASE_Q);
        m_pPlayerInfo->GetTrackInfo(E_TrackAudio)->ClockInfo.packet_pts = pts;
        m_AudioDecoder.Decode(pPacket);
        
        av_free_packet(pPacket);
        delete pPacket;
    }

    m_AudioDecoder.ClearQueue();
    m_AudioDecoder.UnLockQueue();
    DEBUG_PRINT("Audeio decode End\n");
    
    return FFMPEGPLAYER_NOERROR;
}
