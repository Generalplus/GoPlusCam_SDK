//
//  CustomInputThread.cpp
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2017/10/23.
//  Copyright © 2017年 generalplus_sa1. All rights reserved.
//

#include "CustomInputThread.h"
#include "libavutil/intreadwrite.h"

//----------------------------------------------------------------------
C_CustomInputThread::C_CustomInputThread(C_PlayerInfo *pPlayerInfo):
C_InputThreadBase(pPlayerInfo),m_pCodecContext(NULL)
{
     pthread_mutex_init(&m_Contextlock, NULL);
}
//----------------------------------------------------------------------
C_CustomInputThread::~C_CustomInputThread()
{
    
}
//----------------------------------------------------------------------
int C_CustomInputThread::PushPacket(const unsigned char* pbyData,int i32Size,int i32Type,int64_t lTimeStamp)
{
    AVPacket packet;
    int i32Ret = av_new_packet(&packet,i32Size);
    if(i32Ret!=0)
        return FFMPEGPLAYER_PUSHPACKETFAILED;
    
    memcpy(packet.data,pbyData,i32Size);
    
    packet.pts = lTimeStamp;
    packet.flags |= AV_PKT_FLAG_KEY;
    
    C_Event event(E_EventType_Input_Packet,(void*)&packet);
    C_EventManager::GetEvnetManager()->ProcessEvent(event);
    
    av_free_packet(&packet);
    
    return FFMPEGPLAYER_NOERROR;
}
//----------------------------------------------------------------------
AVCodecContext  *C_CustomInputThread::GetCodecContext(AVMediaType StreamType)
{
    AVCodecContext  *pContext = NULL;
    
    pthread_mutex_lock(&m_Contextlock);
    
    if(StreamType == AVMEDIA_TYPE_VIDEO)
        pContext =  m_pCodecContext;
    else
        pContext =  m_pAudioCodecContext;
    
    pthread_mutex_unlock(&m_Contextlock);
    
    return pContext;
}
//----------------------------------------------------------------------
int C_CustomInputThread::ThreadFunction()
{
    m_pPlayerInfo->Reset();
    
    int i32Ret = _Init();
    if(i32Ret!=FFMPEGPLAYER_NOERROR)
    {
        return i32Ret;
    }
    
    while(m_pPlayerInfo->GetStatus()== E_PlayerStatus_Playing)
    {
        sleep(1);
    }
    
    DEBUG_PRINT("Custom input break\n");
    
    return FFMPEGPLAYER_NOERROR;
}
//----------------------------------------------------------------------
void C_CustomInputThread::Close()
{
    pthread_mutex_lock(&m_Contextlock);
    if(m_pCodecContext)
    {
        avcodec_free_context(&m_pCodecContext);
        m_pCodecContext = NULL;
    }
    pthread_mutex_unlock(&m_Contextlock);
}
//----------------------------------------------------------------------
int  C_CustomInputThread::_Init()
{
    C_Event event(E_EventType_Queue_Clear);
    C_EventManager::GetEvnetManager()->ProcessEvent(event);
    
    avcodec_register_all();
    av_register_all();
    avformat_network_init();
    
    DEVICE_PRINT("\n*** GP FFmpeg Video Player Library %s ***\n",VERSION_STR);
    DEVICE_PRINT("-------------------------------------------\n");
    DEVICE_PRINT("FFmpeg version:\n");
    print_all_libs_info(SHOW_VERSION, AV_LOG_INFO);
    DEVICE_PRINT("-------------------------------------------\n");
    DEVICE_PRINT("Start Custom input...\n");
    m_pPlayerInfo->SetIsStreaming(true);
    
    AVCodec *pCodec  = avcodec_find_decoder_by_name(m_VideoCodecName);
    AVCodec *pAudiopCodec  = avcodec_find_decoder_by_name(m_AudioCodecName);
    if(pCodec==NULL && pAudiopCodec==NULL)
        return FFMPEGPLAYER_INITMEDIAFAILED;
    
    if(!pCodec)
    {
        m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->bValid = false;
        m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->i32StreamID = -1;
    }
    else
    {
        SetupTrackInfo(E_TrackVideo,AVMEDIA_TYPE_VIDEO);
        m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->CodecID = pCodec->id;
    }

    AVRational frame_rate = GetFrameRate();
    m_pPlayerInfo->SetVideofps((float)frame_rate.num / (float)frame_rate.den);
    m_pPlayerInfo->SetDuration(0);
    
    if(!pAudiopCodec)
    {
        m_pPlayerInfo->GetTrackInfo(E_TrackAudio)->bValid = false;
        m_pPlayerInfo->GetTrackInfo(E_TrackAudio)->i32StreamID = -1;
    }
    else
    {
        SetupTrackInfo(E_TrackAudio,AVMEDIA_TYPE_AUDIO);
        m_pPlayerInfo->GetTrackInfo(E_TrackAudio)->CodecID = pAudiopCodec->id;
    }
    
    pthread_mutex_lock(&m_Contextlock);
    
    if(pCodec)
        m_pCodecContext = avcodec_alloc_context3(pCodec);
    if(pAudiopCodec)
        m_pAudioCodecContext = avcodec_alloc_context3(pAudiopCodec);
    
    pthread_mutex_unlock(&m_Contextlock);
    
    if (pCodec->capabilities & AV_CODEC_CAP_TRUNCATED)
        m_pCodecContext->flags |= AV_CODEC_FLAG_TRUNCATED;
    
    C_Event eventComplete(E_EventType_Player_Init_Complete);
    C_EventManager::GetEvnetManager()->ProcessEvent(eventComplete);
    
    return FFMPEGPLAYER_NOERROR;
}
//----------------------------------------------------------------------
void C_CustomInputThread::SetupTrackInfo(E_Track track,AVMediaType type)
{
    m_pPlayerInfo->GetTrackInfo(track)->duration = 0;
    m_pPlayerInfo->GetTrackInfo(track)->start_time = 0;
    m_pPlayerInfo->GetTrackInfo(track)->time_base = av_make_q(90000,1);
    m_pPlayerInfo->GetTrackInfo(track)->r_frame_rate = av_make_q(1,DEFAULT_FRAME_RATE);
    m_pPlayerInfo->GetTrackInfo(track)->bValid = true;
    
    if(track == E_TrackVideo)
    {
        m_pPlayerInfo->GetTrackInfo(track)->i32StreamID = 0;
    }
    else
    {
        m_pPlayerInfo->GetTrackInfo(track)->i32StreamID = 1;
    }
}
//----------------------------------------------------------------------
AVRational C_CustomInputThread::GetFrameRate()
{
    return av_make_q(DEFAULT_FRAME_RATE,1);
}
