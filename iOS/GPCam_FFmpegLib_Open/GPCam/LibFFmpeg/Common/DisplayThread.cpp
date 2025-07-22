//
//  DisplayThread.cpp
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2017/10/3.
//  Copyright © 2017年 generalplus_sa1. All rights reserved.
//

#include "DisplayThread.h"
//#define FRAME_TIME_TRACE

#define MAX_DELAY_FACTOR        1.5
#define MIN_DELAY_FACTOR        0.8
//----------------------------------------------------------------------
C_DisplayThread::C_DisplayThread(C_PlayerInfo *pPlayerInfo):
        C_ThreadBase(pPlayerInfo)
{
    SET_EVENT_HANDLER(E_EventType_Decode_VideoFrame,C_DisplayThread,onVideoFrame)
    SET_EVENT_HANDLER(E_EventType_Queue_Clear,C_DisplayThread,onClearQueue)
    SET_EVENT_HANDLER(E_EventType_Player_Stop,C_DisplayThread,onStop)
}
//----------------------------------------------------------------------
C_DisplayThread::~C_DisplayThread()
{

}

//----------------------------------------------------------------------
void C_DisplayThread::Init(AVCodecContext* pVideoCtx)
{
    if(m_sws_ctx)
    {
        sws_freeContext(m_sws_ctx);
        m_sws_ctx = NULL;
    }

    if(pVideoCtx->pix_fmt != AV_PIX_FMT_NONE)
    {
        // Setup scaler
        if(!(pVideoCtx->pix_fmt == AV_PIX_FMT_YUV420P ||
             pVideoCtx->pix_fmt == AV_PIX_FMT_YUVJ420P ||
             pVideoCtx->pix_fmt == AV_PIX_FMT_YUV422P ||
             pVideoCtx->pix_fmt == AV_PIX_FMT_YUVJ422P||
             pVideoCtx->pix_fmt == AV_PIX_FMT_YUV444P ||
             pVideoCtx->pix_fmt == AV_PIX_FMT_YUVJ444P ))
        {

            if(m_covertFrame)
            {
                av_freep(&m_covertFrame->data[0]);
                av_frame_unref(m_covertFrame);
                av_frame_free(&m_covertFrame);
            }


            static int sws_flags =  SWS_FAST_BILINEAR;
            m_sws_ctx = sws_getContext(
                    pVideoCtx->width,
                    pVideoCtx->height,
                    pVideoCtx->pix_fmt,
                    pVideoCtx->width,
                    pVideoCtx->height,
                    AV_PIX_FMT_YUV420P,
                    sws_flags, NULL, NULL, NULL);

            m_covertFrame = av_frame_alloc();
            m_covertFrame->format = AV_PIX_FMT_YUV420P;
            m_covertFrame->width  = pVideoCtx->width;
            m_covertFrame->height = pVideoCtx->height;
            av_image_alloc(m_covertFrame->data, m_covertFrame->linesize, m_covertFrame->width, m_covertFrame->height,AV_PIX_FMT_YUV420P, 1);

        }

    }
}
//----------------------------------------------------------------------
int C_DisplayThread::ThreadFunction()
{
    while(m_pPlayerInfo->GetStatus()  == E_PlayerStatus_Playing)
    {
        CheckAndDoPause();
        if(m_pPlayerInfo->GetStatus() != E_PlayerStatus_Playing)
            break;

        AVFrame *pFrame = PopDisplay();
        if(m_pPlayerInfo->GetStatus()!=E_PlayerStatus_Playing)
            break;

        if(pFrame == NULL)
            continue;

        int64_t timeBefore = GetClock();

        m_pPlayerInfo->SetFrameCnt(m_pPlayerInfo->GetFrameCnt()+1);
        displayFrame(pFrame);
        syncFrame(pFrame);

        av_freep(&pFrame->data[0]);
        av_frame_unref(pFrame);
        av_frame_free(&pFrame);

        m_pPlayerInfo->GetThreadTimeInfo()->DisplayTime = GetClock() - timeBefore;
        UpdateAvgThreadTime(m_pPlayerInfo->GetThreadTimeInfo()->DisplayTime,&m_pPlayerInfo->GetThreadTimeInfo()->AvgDisplayTime);
        m_pPlayerInfo->SetDisplaySize(m_DisplayFrameQueue.GetSize());
    }

    DEBUG_PRINT("display end\n");


    return FFMPEGPLAYER_NOERROR;
}
//----------------------------------------------------------------------
int C_DisplayThread::displayFrame(AVFrame* pFrame)
{
    AVFrame* pDisplayFrame = pFrame;

    if(m_sws_ctx!=NULL)
    {
        int ret = sws_scale(m_sws_ctx,
                            pFrame->data,
                            pFrame->linesize,
                            0,
                            m_covertFrame->height,
                            m_covertFrame->data,
                            m_covertFrame->linesize);

        pDisplayFrame = m_covertFrame;

    }

    C_Event event(E_EventType_Decode_DisplayFrame,(void*)pDisplayFrame);
    C_EventManager::GetEvnetManager()->ProcessEvent(event);

    return FFMPEGPLAYER_NOERROR;
}
//----------------------------------------------------------------------
int C_DisplayThread::syncFrame(AVFrame* pFrame)
{
    int64_t pts,ptsOrg,during;
    ptsOrg = av_frame_get_best_effort_timestamp(pFrame);

    pts = ptsOrg;
    if(m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->start_time != AV_NOPTS_VALUE)
        pts-=m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->start_time;

    pts = av_rescale_q (pts,
                        m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->time_base,
                        AV_TIME_BASE_Q );

    if(m_pPlayerInfo->GetBufferingTime() == 0)
    {
        if(m_pPlayerInfo->GetbLowLatency() &&
           !m_pPlayerInfo->GetTrackInfo(E_TrackAudio)->bValid) {

            m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->ClockInfo.frame_pts = pts;
            m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->ClockInfo.Sync_pts = pts;

            return FFMPEGPLAYER_NOERROR;
        }
    }

    during = av_rescale_q (pFrame->pkt_duration,
                           m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->time_base,
                           AV_TIME_BASE_Q );

    int64_t delay = 0;

    if(m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->ClockInfo.frame_pts==0)
        m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->ClockInfo.frame_pts = pts;

    if(ptsOrg != AV_NOPTS_VALUE && m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->ClockInfo.frame_pts!=AV_NOPTS_VALUE)
        delay = pts - m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->ClockInfo.frame_pts;

    int64_t now = GetClock();

    if(m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->ClockInfo.update_time==0)
        m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->ClockInfo.update_time = now;

    if(delay<0)
    {
        DEBUG_PRINT("delay < 0 , New frame time stamp is too old! Reset the time stamp!\n");
        delay = 0;
        m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->ClockInfo.update_time = now;
    }

    int64_t escTime = now -  m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->ClockInfo.update_time;
    int64_t timeDelay =  delay - escTime;

    int64_t FrameDiff = 0;
    int64_t AudioPts = m_pPlayerInfo->GetTrackInfo(E_TrackAudio)->ClockInfo.frame_pts;

    if(m_pPlayerInfo->GetTrackInfo(E_TrackAudio)->bValid)
    {
        FrameDiff += m_pPlayerInfo->GetAudioStartAlign();

        // while(m_pPlayerInfo->GetTrackInfo(E_TrackAudio)->ClockInfo.update_time==0)
        //     DoDelay(AV_TIME_BASE * 0.05);
    }

    if(m_pPlayerInfo->GetTrackInfo(E_TrackAudio)->ClockInfo.update_time!=0 &&
       ptsOrg != AV_NOPTS_VALUE &&
       m_pPlayerInfo->GetTrackInfo(E_TrackAudio)->ClockInfo.Sync_pts != m_pPlayerInfo->GetTrackInfo(E_TrackAudio)->ClockInfo.frame_pts)
    {
        FrameDiff += pts - AudioPts;
        timeDelay = FrameDiff;
    }

    if(m_pPlayerInfo->GetIsStreaming() && m_pPlayerInfo->GetBufferingTime() >0)
    {
        if(timeDelay<0)
        {
            int i32QueueSize = m_pPlayerInfo->GetVideofps() * ((float)m_pPlayerInfo->GetBufferingTime()  / 1000.0);
            if(m_pPlayerInfo->GetDisplaySize() == 0)
            {
                int64_t frameTime = AV_TIME_BASE * (1 / m_pPlayerInfo->GetVideofps());
                timeDelay = ((i32QueueSize - m_pPlayerInfo->GetDisplaySize()) * frameTime)*2;
                C_Event Startevent(E_EventType_Buffer_Start);
                C_EventManager::GetEvnetManager()->ProcessEvent(Startevent);

                int64_t StartCheck = GetClock();

                while(m_pPlayerInfo->GetStatus() == E_PlayerStatus_Playing &&
                      i32QueueSize > m_pPlayerInfo->GetDisplaySize() &&
                      timeDelay > GetClock() - StartCheck
                        )
                {
                    DEBUG_PRINT("Buffering...\n");
                    DoDelay(frameTime);
                }
                m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->ClockInfo.update_time = 0;
                C_Event Completeevent(E_EventType_Buffer_Complete);
                C_EventManager::GetEvnetManager()->ProcessEvent(Completeevent);
            }

        }
        else
        {
            DoDelay(timeDelay);
            m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->ClockInfo.update_time = GetClock();
        }
    }
    else
    {
        if(timeDelay > MAX_DELAY_FACTOR * during)
            timeDelay = MAX_DELAY_FACTOR * during;
        //else if(timeDelay < 0)
        //    timeDelay = 0;

        int i32PlayRate = m_pPlayerInfo->GetPlayRate();
        if(i32PlayRate!=0 && timeDelay>0)
            timeDelay = timeDelay / i32PlayRate;

        if(timeDelay>0)
            DoDelay(timeDelay);

        m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->ClockInfo.update_time = GetClock();

        if(timeDelay<0)
            m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->ClockInfo.update_time+=timeDelay;

    }

#if defined(DEBUG) && defined(FRAME_TIME_TRACE)

    DEBUG_PRINT("now: %.10lld timeDelay: %.10lld FrameDiff: %.10lld pts: %lld prev: %lld audio: %lld delay %lld escTime %lld ptsOrg %lld Packet %d Display %d\n",
                now,
                timeDelay,
                FrameDiff,
                pts,
                m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->ClockInfo.frame_pts,
                m_pPlayerInfo->GetTrackInfo(E_TrackAudio)->ClockInfo.frame_pts,
                delay,
                escTime,
                ptsOrg,
                m_pPlayerInfo->GetDecodeSize(),
                m_pPlayerInfo->GetDisplaySize()
                );
#endif

    m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->ClockInfo.frame_pts = pts;
    m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->ClockInfo.Sync_pts = pts;
    m_pPlayerInfo->GetTrackInfo(E_TrackAudio)->ClockInfo.Sync_pts = AudioPts;

    return FFMPEGPLAYER_NOERROR;
}
//----------------------------------------------------------------------
void C_DisplayThread::onVideoFrame(C_Event &event)
{
    AVFrame *pInputFrame = (AVFrame *)event.GetData();
    AVFrame *pFrame = av_frame_alloc();
    FrameCopy(pFrame,pInputFrame);

    m_DisplayFrameQueue.PushOject(pFrame);
    m_pPlayerInfo->SetDisplaySize(m_DisplayFrameQueue.GetSize());
}
