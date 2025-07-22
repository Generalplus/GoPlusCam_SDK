//
//  EncodeThread.cpp
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2017/10/3.
//  Copyright © 2017年 generalplus_sa1. All rights reserved.
//

#include "EncodeThread.h"
//----------------------------------------------------------------------
C_EncodeThread::C_EncodeThread(C_PlayerInfo *pPlayerInfo):
C_ThreadBase(pPlayerInfo),m_VideoEncoder(pPlayerInfo),m_EncodeID(AV_CODEC_ID_H264),m_bForceToTransCode(false)
{
    SET_EVENT_HANDLER(E_EventType_Queue_Clear,C_EncodeThread,onClearQueue)
    SET_EVENT_HANDLER(E_EventType_Player_Stop,C_EncodeThread,onStop)
    SET_EVENT_HANDLER(E_EventType_Input_Packet,C_EncodeThread,onInputPacket)
    SET_EVENT_HANDLER(E_EventType_Decode_VideoFrame,C_EncodeThread,onVideoFrame)
}
//----------------------------------------------------------------------
C_EncodeThread::~C_EncodeThread()
{
    
}

//----------------------------------------------------------------------
void C_EncodeThread::onVideoFrame(C_Event &event)
{
    if(!IsRunning())
        return ;
    
    if(m_VideoEncoder.GetEncodeType() == E_EncodeType_RawStream)
        return ;

    AVFrame *pInputFrame = (AVFrame *)event.GetData();
    
    m_VideoEncoder.pushTranscodeFrame(pInputFrame); // pushTranscodeFrame() will dup avframe
}
//----------------------------------------------------------------------
void C_EncodeThread::onInputPacket(C_Event &event)
{
    if(!IsRunning())
        return ;
    
    if(m_VideoEncoder.GetEncodeType() == E_EncodeType_Transcode)
        return ;
    
    AVPacket *pPacket = (AVPacket *)event.GetData();
    AVPacket *tempkt = new AVPacket;
    av_init_packet(tempkt);
    av_copy_packet(tempkt, pPacket);
    m_VideoEncoder.pushRawPacket(tempkt);
}
//----------------------------------------------------------------------
int C_EncodeThread::ThreadFunction()
{
    
    if(m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->CodecID == AV_CODEC_ID_MJPEG && m_bForceToTransCode)
    {
        m_VideoEncoder.SetEncodeType(E_EncodeType_Transcode);
        m_bSaveVideo = m_VideoEncoder.CreateEncodeStream(m_EncodeID,m_VideoPath,m_pPlayerInfo->GetVideofps(),m_TransCodeOptions);
    }
    else
    {
        m_VideoEncoder.SetEncodeType(E_EncodeType_RawStream);
        
        bool bIsRTSP = true;
        bool bUsingLocalTime = m_bEncodeUsingLocalTime;
        char *pType = strrchr(m_MediaPath, '.');
        
        if ( pType == NULL ||
            (strstr(m_MediaPath, "rtsp")==NULL && ( pType && strcmp(pType,".sdp") != 0))
           )
        {
            bIsRTSP = false;
            bUsingLocalTime = true;
        }
        
        m_bSaveVideo = m_VideoEncoder.CreateEncodeRawStream(m_VideoPath,m_pPlayerInfo->GetVideofps(),bIsRTSP,bUsingLocalTime);
    }
    
    while(m_bSaveVideo && m_pPlayerInfo->GetStatus() == E_PlayerStatus_Playing)
    {
        if(m_VideoEncoder.GetEncodeType() == E_EncodeType_Transcode)
        {
            if(m_VideoEncoder.writeTranscodeFrame()!=FFMPEGPLAYER_NOERROR)
                break;
        }
        else
        {
            if(!m_VideoEncoder.writeRawStream())
                break;
        }
    } 
    
    m_VideoEncoder.CloseVideo();
    
    C_Event event(E_EventType_Save_VideoComplete);
    C_EventManager::GetEvnetManager()->ProcessEvent(event);
    
    return FFMPEGPLAYER_NOERROR;
}
