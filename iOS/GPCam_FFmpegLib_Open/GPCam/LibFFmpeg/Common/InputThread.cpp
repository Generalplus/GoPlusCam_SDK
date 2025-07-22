//
//  InputThread.cpp
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2017/10/3.
//  Copyright © 2017年 generalplus_sa1. All rights reserved.
//

#include "InputThread.h"
#include "CodecDefine.h"

#define SYNC_FACTOR       0.3
#define SYNC_DELAY_DIV    10


//----------------------------------------------------------------------
C_InputThread::C_InputThread(C_PlayerInfo *pPlayerInfo):
C_InputThreadBase(pPlayerInfo)
{
    pthread_mutex_init(&m_Contextlock, NULL);
}
//----------------------------------------------------------------------
C_InputThread::~C_InputThread()
{
    
}
//----------------------------------------------------------------------
AVCodecContext  *C_InputThread::GetCodecContext(AVMediaType StreamType)
{
    AVCodec *pCodec = NULL;
    int i32StreamID = -1;
    
    if(StreamType == AVMEDIA_TYPE_AUDIO)
        i32StreamID = m_pPlayerInfo->GetTrackInfo(E_TrackAudio)->i32StreamID;
    else
        i32StreamID = m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->i32StreamID;
    
    if(i32StreamID == -1)
        return NULL;

    
    return m_formatCtx->streams[i32StreamID]->codec;
}
//----------------------------------------------------------------------
int C_InputThread::Pause()
{
    m_pPlayerInfo->SetPlayRate(PAUSE_PLAYRATE);
    
    C_Event event(E_EventType_Player_Pause);
    C_EventManager::GetEvnetManager()->ProcessEvent(event);
    
    pthread_mutex_lock(&m_Contextlock);
    
    if(m_formatCtx)
        av_read_pause(m_formatCtx);
    
    pthread_mutex_unlock(&m_Contextlock);
    
    return FFMPEGPLAYER_NOERROR;
}
//----------------------------------------------------------------------
int C_InputThread::Resume()
{
    m_pPlayerInfo->SetPlayRate(DEFAULT_PLAYRATE);
    
    C_Event event(E_EventType_Player_Play);
    C_EventManager::GetEvnetManager()->ProcessEvent(event);
    
    m_pPlayerInfo->reflashUpdateTime();
    
    pthread_mutex_lock(&m_Contextlock);
    
    if(m_formatCtx)
        av_read_play(m_formatCtx);
    
    pthread_mutex_unlock(&m_Contextlock);
    
    return FFMPEGPLAYER_NOERROR;
}
//----------------------------------------------------------------------
bool C_InputThread::Init(const char* path,const char* pOptions)
{
    strcpy(m_MediaPath, path);
    if(pOptions)
        strcpy(m_UserOptions, pOptions);
    
    return true;
}
//----------------------------------------------------------------------
int C_InputThread::ThreadFunction()
{
    m_pPlayerInfo->Reset();
    
    int i32Ret = _Init();
    if(i32Ret!=FFMPEGPLAYER_NOERROR)
        return i32Ret;
    
    AVPacket                packet;
    int64_t                 prevTime = GetClock();
    int64_t                 lastTime[2] = {0,0};
    
    while(m_pPlayerInfo->GetStatus()== E_PlayerStatus_Playing)
    {
        pthread_mutex_lock(&m_Contextlock);
        if(m_formatCtx)
            i32Ret = av_read_frame(m_formatCtx, &packet);
        pthread_mutex_unlock(&m_Contextlock);
        
        if(i32Ret>=0)
        {
            if(seekingFrame(&packet))
                continue;
            
            int64_t pts = av_rescale_q (packet.pts,
                                        m_pPlayerInfo->GetTrackInfo((E_Track)packet.stream_index)->time_base,
                                        AV_TIME_BASE_Q );
            
           // DEBUG_PRINT("Steam %d: pts %lld \n", packet.stream_index, pts);
            lastTime[packet.stream_index] = pts;
            
            m_pPlayerInfo->SetInputCnt(packet.size+m_pPlayerInfo->GetInputCnt());
            
            C_Event event(E_EventType_Input_Packet,(void*)&packet);
            C_EventManager::GetEvnetManager()->ProcessEvent(event);
            
            av_free_packet(&packet);
        }
        else
        {
            if(!m_pPlayerInfo->GetIsStreaming())
            {
                if(m_pPlayerInfo->GetRepeat() && !(m_pPlayerInfo->GetDisplaySize()>0) && !(m_pPlayerInfo->GetDecodeSize()>0))
                    Seek(0);
                else
                    DoDelay(DEFAULT_DELAY);
            }
            else
                break;
        }
        
        
        if(m_pPlayerInfo->GetSeekInfo()->bAssignSeek)
        {
            SeekInternal(m_pPlayerInfo->GetSeekInfo()->AssignSeekPosition);
            
            m_pPlayerInfo->SetAudioPlayedSample(m_pPlayerInfo->GetSeekInfo()->AssignSeekPosition);
            m_pPlayerInfo->GetSeekInfo()->SeekingPosition = m_pPlayerInfo->GetSeekInfo()->AssignSeekPosition;
            m_pPlayerInfo->GetSeekInfo()->bAssignSeek = false;
            m_pPlayerInfo->GetSeekInfo()->bSeeking = true;
            m_pPlayerInfo->GetSeekInfo()->bSeekingPreview = true;
            if(m_pPlayerInfo->GetTrackInfo(E_TrackAudio)->bValid)
                m_pPlayerInfo->GetSeekInfo()->bSeekAudio = true;
        }
        else
            inputDelay(prevTime,lastTime[0],lastTime[1]);
            
        prevTime = GetClock();
    }
    
    DEBUG_PRINT("input break\n");

    return FFMPEGPLAYER_NOERROR;
}
//----------------------------------------------------------------------
void C_InputThread::Close()
{
    pthread_mutex_lock(&m_Contextlock);
    if(m_formatCtx)
    {
        avformat_close_input(&m_formatCtx);
        m_formatCtx = NULL;
    }
    pthread_mutex_unlock(&m_Contextlock);
}
//----------------------------------------------------------------------
void C_InputThread::Seek(int64_t position)
{
    m_pPlayerInfo->GetSeekInfo()->bAssignSeek = true;
    m_pPlayerInfo->GetSeekInfo()->AssignSeekPosition = position;
}
//----------------------------------------------------------------------
void C_InputThread::SeekInternal(int64_t position)
{
    pthread_mutex_lock(&m_Contextlock);
    
    if(m_formatCtx)
    {
        C_Event event(E_EventType_Player_Seek);
        C_EventManager::GetEvnetManager()->ProcessEvent(event);
        
        C_Event eventRampDown(E_EventType_Audio_RampDown);
        C_EventManager::GetEvnetManager()->ProcessEvent(eventRampDown);
        
        C_Event eventClearQueue(E_EventType_Queue_Clear);
        C_EventManager::GetEvnetManager()->ProcessEvent(eventClearQueue);

        DEBUG_PRINT("Seek to %lld\n",position);

        int ret = 0;
        if(m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->CodecID  == AV_CODEC_ID_MJPEG)
        {
            avformat_flush(m_formatCtx);
            ret = av_seek_frame(m_formatCtx, -1, position, AVSEEK_FLAG_ANY);
        }
        else
        {
            avformat_seek_file(m_formatCtx, -1, INT64_MIN, position, INT64_MAX, 0);
        }

        if (ret < 0)
        {
            DEBUG_PRINT("Seek to %lld failed! Error: %d \n",position,ret);
        }
        else
        {
            m_pPlayerInfo->resetClock();
            
            while(m_pPlayerInfo->GetDisplaySize() >0 &&
                  m_pPlayerInfo->GetStatus()== E_PlayerStatus_Playing &&
                  m_pPlayerInfo->GetPlayRate() != PAUSE_PLAYRATE);
            
            m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->ClockInfo.frame_pts = position;
            m_pPlayerInfo->GetTrackInfo(E_TrackAudio)->ClockInfo.frame_pts = position;
        }
    }
    
    pthread_mutex_unlock(&m_Contextlock);
    
}
//----------------------------------------------------------------------
bool C_InputThread::seekingFrame(AVPacket *packet)
{
    int64_t pts = packet->pts;
    if((E_Track)packet->stream_index == E_TrackVideo)
    {
        if(m_pPlayerInfo->GetSeekInfo()->bSeeking)
        {
            if(m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->start_time != AV_NOPTS_VALUE)
                pts-=m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->start_time;
            
            pts = av_rescale_q (pts,
                                m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->time_base,
                                AV_TIME_BASE_Q);
            
            DEBUG_PRINT("Step Frame: %lld \n",pts);
            
            if(pts >= m_pPlayerInfo->GetSeekInfo()->SeekingPosition)
            {
                m_pPlayerInfo->GetSeekInfo()->bSeeking = false;
            }
            
            if(m_pPlayerInfo->GetSeekInfo()->bSeekingPreview)
            {
                m_pPlayerInfo->GetSeekInfo()->bSeekingPreview = false;
                m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->ClockInfo.update_time = GetClock();
                return false;
            }
            
        }
        
        return m_pPlayerInfo->GetSeekInfo()->bSeeking;
    }
    else
    {
        if(m_pPlayerInfo->GetSeekInfo()->bSeekAudio)
        {
            pts-= m_pPlayerInfo->GetTrackInfo(E_TrackAudio)->start_time;
            pts = av_rescale_q (pts,
                                m_pPlayerInfo->GetTrackInfo(E_TrackAudio)->time_base,
                                AV_TIME_BASE_Q);
            
            
            DEBUG_PRINT("Step Audio Frame: %lld \n",pts);
            
            if(pts >= m_pPlayerInfo->GetSeekInfo()->SeekingPosition)
            {
                m_pPlayerInfo->GetSeekInfo()->bSeekAudio = false;
                m_pPlayerInfo->GetTrackInfo(E_TrackAudio)->ClockInfo.frame_pts = pts;
                m_pPlayerInfo->GetTrackInfo(E_TrackAudio)->ClockInfo.update_time = GetClock();

                
                C_Event eventRampUp(E_EventType_Audio_RampUp);
                C_EventManager::GetEvnetManager()->ProcessEvent(eventRampUp);
            }
        }
        
        return m_pPlayerInfo->GetSeekInfo()->bSeekAudio && m_pPlayerInfo->GetSeekInfo()->bSeeking;
    }

}
//----------------------------------------------------------------------
void C_InputThread::inputDelay(int64_t prevTime,int64_t lastVideoTime,int64_t lastAudioTime)
{
    while(m_pPlayerInfo->GetPlayRate() == PAUSE_PLAYRATE &&
          !m_pPlayerInfo->GetSeekInfo()->bAssignSeek &&
          !m_pPlayerInfo->GetSeekInfo()->bSeeking &&
          m_pPlayerInfo->GetStatus()== E_PlayerStatus_Playing)
    {
        DoDelay(PAUSE_DEALY);
    }
    
    if(m_pPlayerInfo->GetIsStreaming())
        return ;
    
    if(m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->ClockInfo.packet_pts == 0)
        return;

    int64_t delayTime = m_pPlayerInfo->GetThreadTimeInfo()->AvgDecodeTime * m_pPlayerInfo->GetDecodeSize();
    if(m_pPlayerInfo->GetTrackInfo(E_TrackAudio)->bValid)
    {
        int64_t AudioBufferPts = m_pPlayerInfo->GetAudioQueueIndex() - m_pPlayerInfo->GetAudioPlayedSample();
        int64_t AudioBufferTime = av_rescale_q (AudioBufferPts,
                                                m_pPlayerInfo->GetTrackInfo(E_TrackAudio)->time_base,
                                                AV_TIME_BASE_Q);
        
        //DEBUG_PRINT("buffer size: %lld delayTime: %lld , AvgDecodeTime %lld DecodeSize %d\n",AudioBufferTime,delayTime,m_pPlayerInfo->GetThreadTimeInfo()->AvgDecodeTime,m_pPlayerInfo->GetDecodeSize());
        
        if(AudioBufferTime > SYNC_DURATION)
        {
            if(delayTime > AudioBufferTime)
                delayTime = AudioBufferTime * SYNC_FACTOR;
            //DEBUG_PRINT("delayTime: %lld\n",delayTime);
            ThreadDelay(delayTime);
        }
        else if(m_pPlayerInfo->GetAudioPlayedSample()==0 && AudioBufferTime<=0)
        {
            ThreadDelay(delayTime);
        }
    }
    else
    {
        ThreadDelay(delayTime);
    }

}
//----------------------------------------------------------------------
int  C_InputThread::_Init()
{
    C_Event event(E_EventType_Queue_Clear);
    C_EventManager::GetEvnetManager()->ProcessEvent(event);
    
    avcodec_register_all();
    av_register_all();
    avformat_network_init();
    
    //rtsp increase udp buffer size
    AVDictionary *opts = 0;
    av_dict_set(&opts, "buffer_size", "524288", 0);  //increase to 512KB from 64KB
    
    if(strstr(m_MediaPath, "http")!=NULL)
        av_dict_set(&opts, "timeout", "3000000", 0); //Set timeout to 3sec
    else if (strstr(m_MediaPath, "rtsp")!=NULL)
    {
        av_dict_set(&opts, "stimeout", "3000000", 0); //Set timeout to 3sec
        av_dict_set(&opts, "fflags", "nobuffer", 0);
        av_dict_set(&opts, "fflags", "flush_packets", 0);
        av_dict_set(&opts, "probesize", "32", 0);
        av_dict_set(&opts, "max_analyze_duration", "5000000", 0);
    }
    else if (strcmp(strrchr(m_MediaPath, '.'),".sdp") == 0)
    {
        av_dict_set(&opts, "protocol_whitelist", "file,rtp,tcp,udp", 0);
    }
    
    SetAVDictionary(m_UserOptions,opts);
    
    //av_log_set_level(AV_LOG_TRACE);
#if defined(DEBUG) && defined(FFMPEG_TRACE)
    av_log_set_level(AV_LOG_TRACE);
#endif
    DEVICE_PRINT("\n*** GP FFmpeg Video Player Library %s ***\n",VERSION_STR);
    DEVICE_PRINT("-------------------------------------------\n");
    DEVICE_PRINT("FFmpeg version:\n");
    print_all_libs_info(SHOW_VERSION, AV_LOG_INFO);
    DEVICE_PRINT("-------------------------------------------\n");
    DEVICE_PRINT("Play Path: %s\n\n",m_MediaPath);
    
    if (avformat_open_input(&m_formatCtx, m_MediaPath, NULL, &opts) != 0) {
        DEBUG_PRINT("avformat_open_input %s failed!\n", m_MediaPath);
        DEVICE_PRINT("Failed to play Path: %s\n",m_MediaPath);
        
        C_Event event(E_EventType_Player_Init_Failed);
        C_EventManager::GetEvnetManager()->ProcessEvent(event);
        
        return FFMPEGPLAYER_INITMEDIAFAILED;
    }

    av_dict_free(&opts);
    
    if (avformat_find_stream_info(m_formatCtx, NULL) < 0) {
        DEBUG_PRINT("avformat_find_stream_info failed!\n");
        
        C_Event event(E_EventType_Player_Init_Failed);
        C_EventManager::GetEvnetManager()->ProcessEvent(event);
        
        return FFMPEGPLAYER_INITMEDIAFAILED;
    }
    
    //Fill m_pPlayerInfo
    m_pPlayerInfo->SetIsStreaming(true);
    
    if(m_formatCtx->pb)
    {
        if(m_formatCtx->pb->seekable == AVIO_SEEKABLE_NORMAL)
            m_pPlayerInfo->SetIsStreaming(false);
    }
    
    SetupTrackInfo(E_TrackVideo,AVMEDIA_TYPE_VIDEO);
    SetupTrackInfo(E_TrackAudio,AVMEDIA_TYPE_AUDIO);
    
    AVRational frame_rate = GetFrameRate();
    m_pPlayerInfo->SetVideofps((float)frame_rate.num / (float)frame_rate.den);
    m_pPlayerInfo->SetDuration(m_formatCtx->duration);
    
    C_Event eventComplete(E_EventType_Player_Init_Complete);
    C_EventManager::GetEvnetManager()->ProcessEvent(eventComplete);
    
    return FFMPEGPLAYER_NOERROR;
}
//----------------------------------------------------------------------
void C_InputThread::SetupTrackInfo(E_Track track,AVMediaType type)
{
    int i32StreamID = -1;
    AVCodec *pCodec = NULL;
    if ((i32StreamID = av_find_best_stream(m_formatCtx,
                                           type, -1, -1, &pCodec, 0)) < 0) {
        DEBUG_PRINT("av_find_best_stream failed!\n");
        m_pPlayerInfo->GetTrackInfo(track)->bValid = false;
       m_pPlayerInfo->GetTrackInfo(track)->i32StreamID = -1;
        return ;
    }
    
    m_pPlayerInfo->GetTrackInfo(track)->duration = m_formatCtx->streams[i32StreamID]->duration;
    m_pPlayerInfo->GetTrackInfo(track)->start_time = m_formatCtx->streams[i32StreamID]->start_time;
    m_pPlayerInfo->GetTrackInfo(track)->time_base = m_formatCtx->streams[i32StreamID]->time_base;
    m_pPlayerInfo->GetTrackInfo(track)->r_frame_rate = m_formatCtx->streams[i32StreamID]->r_frame_rate;
    m_pPlayerInfo->GetTrackInfo(track)->i32StreamID = i32StreamID;
    m_pPlayerInfo->GetTrackInfo(track)->bValid = true;
    m_pPlayerInfo->GetTrackInfo(track)->CodecID = m_formatCtx->streams[i32StreamID]->codec->codec_id;
}
//----------------------------------------------------------------------
AVRational C_InputThread::GetFrameRate()
{
    AVRational frame_rate = av_guess_frame_rate(m_formatCtx ,m_formatCtx->streams[m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->i32StreamID], NULL);
    float fVideofps = (float)frame_rate.num / (float)frame_rate.den;
    if(fVideofps > MAX_FRAME_RATE || fVideofps < 0 )
        fVideofps = DEFAULT_FRAME_RATE;
    
    return av_make_q(fVideofps,1);
}

