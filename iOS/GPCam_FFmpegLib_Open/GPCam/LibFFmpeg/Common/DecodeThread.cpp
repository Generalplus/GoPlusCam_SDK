//
//  DecodeThread.cpp
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2017/10/3.
//  Copyright © 2017年 generalplus_sa1. All rights reserved.
//

#include "DecodeThread.h"

#define MAX_QUEUED_DEALY        50000 // 50ms
//----------------------------------------------------------------------
C_DecodeThread::C_DecodeThread(C_PlayerInfo *pPlayerInfo):
C_ThreadBase(pPlayerInfo),m_bSaveSnapshot(false),
m_VideoDecoder(pPlayerInfo),m_SnapshotEncoder(pPlayerInfo)
{
    
    SET_EVENT_HANDLER(E_EventType_Input_Packet,C_DecodeThread,onInputPacket)
    SET_EVENT_HANDLER(E_EventType_Player_Seek,C_DecodeThread,onSeek)
    SET_EVENT_HANDLER(E_EventType_Queue_Clear,C_DecodeThread,onClearQueue)
    SET_EVENT_HANDLER(E_EventType_Player_Stop,C_DecodeThread,onStop)
}
//----------------------------------------------------------------------
C_DecodeThread::~C_DecodeThread()
{
    
}
//----------------------------------------------------------------------
int C_DecodeThread::OpenCodec(AVCodecContext *pCodeContext,AVCodecID CodeID,char* ptszOption)
{
    int i32Ret = m_VideoDecoder.OpenCodec(pCodeContext, CodeID, ptszOption);
    if(i32Ret == FFMPEGPLAYER_NOERROR)
    {
        m_pPlayerInfo->SetWitdh(pCodeContext->width);
        m_pPlayerInfo->SetHeight(pCodeContext->height);
    }
    
    return i32Ret;
}
//----------------------------------------------------------------------
void C_DecodeThread::onInputPacket(C_Event &event)
{
    if(!IsRunning())
        return ;
    
    AVPacket *pPacket = (AVPacket *)event.GetData();
    if(pPacket->stream_index == m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->i32StreamID)
    {
        AVPacket *tempkt = new AVPacket;
        av_init_packet(tempkt);
        av_copy_packet(tempkt, pPacket);
        m_VideoDecoder.pushDecodePacket(tempkt);
        m_pPlayerInfo->SetDecodeSize(m_VideoDecoder.GetDecodeSize());
    }
}
//----------------------------------------------------------------------
int C_DecodeThread::ThreadFunction()
{
    while(m_pPlayerInfo->GetStatus() ==E_PlayerStatus_Playing)
    {
        CheckAndDoPause();
        if(m_pPlayerInfo->GetStatus()!= E_PlayerStatus_Playing)
            break;
        
        AVPacket *pPacket = m_VideoDecoder.PopDecodePacket();
        if(pPacket == NULL)
            continue;
        
        int64_t timeBefore = GetClock();
        
        int64_t pts = av_rescale_q (pPacket->pts,
                                   m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->time_base,
                                   AV_TIME_BASE_Q);
        m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->ClockInfo.packet_pts = pts;
        if(m_VideoDecoder.Decode(pPacket))
        {
            if(m_bSaveSnapshot)
                EncodeSnapshot();
        }
        
        m_pPlayerInfo->GetThreadTimeInfo()->DecodeTime = GetClock() - timeBefore;
        UpdateAvgThreadTime(m_pPlayerInfo->GetThreadTimeInfo()->DecodeTime,&m_pPlayerInfo->GetThreadTimeInfo()->AvgDecodeTime);
        
        if(!m_pPlayerInfo->GetIsStreaming() && !m_pPlayerInfo->GetSeekInfo()->bSeeking)
        {
            int64_t delayTime = (m_pPlayerInfo->GetThreadTimeInfo()->AvgDisplayTime - m_pPlayerInfo->GetThreadTimeInfo()->AvgDecodeTime) * m_pPlayerInfo->GetDisplaySize();
            //DEVICE_PRINT("Decode delayTime %lld GetDisplaySize: %d \n",delayTime,m_pPlayerInfo->GetDisplaySize());
            ThreadDelay(delayTime);
        }
        
        av_free_packet(pPacket);
        delete pPacket;
        
        int i32QueueTime= (int)(m_pPlayerInfo->GetDecodeSize() * m_pPlayerInfo->GetThreadTimeInfo()->AvgDecodeTime);
        if(i32QueueTime > MAX_QUEUED_DEALY &&
           m_pPlayerInfo->GetIsStreaming() &&
           m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->CodecID == AV_CODEC_ID_MJPEG &&
           m_pPlayerInfo->GetBufferingTime() == 0)
        {
           DEVICE_PRINT("Over max queue delay, Drop packet!!! Decode: %d Display: %d Decode Time: %lld Avg %lld Display Time: %lld Avg %lld\n",
                         m_pPlayerInfo->GetDecodeSize(),
                         m_pPlayerInfo->GetDisplaySize(),
                         m_pPlayerInfo->GetThreadTimeInfo()->DecodeTime,
                         m_pPlayerInfo->GetThreadTimeInfo()->AvgDecodeTime,
                         m_pPlayerInfo->GetThreadTimeInfo()->DisplayTime,
                         m_pPlayerInfo->GetThreadTimeInfo()->AvgDisplayTime);
            
            m_VideoDecoder.ClearQueue();
        }
        m_pPlayerInfo->SetDecodeSize(m_VideoDecoder.GetDecodeSize());
    }

    m_VideoDecoder.ClearQueue();
    m_VideoDecoder.UnLockQueue();
    DEBUG_PRINT("decode End\n");
    
    return FFMPEGPLAYER_NOERROR;
}
//----------------------------------------------------------------------
int C_DecodeThread::EncodeSnapshot()
{
    m_bSaveSnapshot = false;
    
    m_SnapshotEncoder.Encode(m_VideoDecoder.GetDecodeFrame(),
                             m_VideoDecoder.GetCodecContext(),
                             m_snapShotPath);
    
    C_Event eventComplete(E_EventType_Save_SnapshotComplete);
    C_EventManager::GetEvnetManager()->ProcessEvent(eventComplete);
    
    return FFMPEGPLAYER_NOERROR;
}

