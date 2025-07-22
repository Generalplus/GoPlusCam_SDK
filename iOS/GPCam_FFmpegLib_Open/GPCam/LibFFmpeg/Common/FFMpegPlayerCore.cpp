//
// Created by generalplus_sa1 on 4/29/16.
//

#include "FFMpegPlayerCore.h"
#include "Defines.h"
#include <unistd.h>
#include <vector>
#include <sys/time.h>
#include <signal.h>
//----------------------------------------------------------------------
C_FFMpegPlayer::C_FFMpegPlayer():
m_pInputThread(&m_InputThread),
m_InputThread(&m_PlayInfo),
m_CustomInputThread(&m_PlayInfo),
m_DecodeThread(&m_PlayInfo),
m_AudioDecodeThread(&m_PlayInfo),
m_DisplayThread(&m_PlayInfo),
m_EncodeThread(&m_PlayInfo)

{
    SET_EVENT_HANDLER(E_EventType_Player_Init_Complete,C_FFMpegPlayer,onInitComplete)
    SET_EVENT_HANDLER(E_EventType_Player_Init_Failed,C_FFMpegPlayer,onInitFailed)
    SET_EVENT_HANDLER(E_EventType_Save_SnapshotComplete,C_FFMpegPlayer,onSaveSanpshotComplete)
    SET_EVENT_HANDLER(E_EventType_Save_VideoComplete,C_FFMpegPlayer,onSaveVideoComplete)
    SET_EVENT_HANDLER(E_EventType_Buffer_Start,C_FFMpegPlayer,onBufferStart)
    SET_EVENT_HANDLER(E_EventType_Buffer_Complete,C_FFMpegPlayer,onBufferComplete)
    SET_EVENT_HANDLER(E_EventType_Decode_DisplayFrame,C_FFMpegPlayer,onDisplayFrame)
    SET_EVENT_HANDLER(E_EventType_Audio_PlayedSample,C_FFMpegPlayer,onPlayedSample)
    SET_EVENT_HANDLER(E_EventType_Player_Stop,C_FFMpegPlayer,onStop)
    SET_EVENT_HANDLER(E_EventType_Thread_End,C_FFMpegPlayer,onThreadEnd)

    m_ThreadQueue.PushOject(&m_DecodeThread);
    m_ThreadQueue.PushOject(&m_AudioDecodeThread);
    m_ThreadQueue.PushOject(&m_DisplayThread);
    m_ThreadQueue.PushOject(&m_EncodeThread);
    m_ThreadQueue.PushOject(&m_InputThread);
    m_ThreadQueue.PushOject(&m_CustomInputThread);
}
//----------------------------------------------------------------------
C_FFMpegPlayer::~C_FFMpegPlayer()
{
    Stop();
}
//----------------------------------------------------------------------
void  C_FFMpegPlayer::onInitComplete(C_Event &event)
{
    //Setup decoder and display thread
    AVCodecContext *pVideoCodeContext = m_pInputThread->GetCodecContext(AVMEDIA_TYPE_VIDEO);
    if(pVideoCodeContext)
    {
        m_DecodeThread.OpenCodec(pVideoCodeContext,pVideoCodeContext->codec_id, m_DecodeCodeOptions);
        m_DisplayThread.Init(pVideoCodeContext);
    }

    AVCodecContext *pAudioCodeContext = m_pInputThread->GetCodecContext(AVMEDIA_TYPE_AUDIO);
    if(pAudioCodeContext)
        m_AudioDecodeThread.OpenCodec(pAudioCodeContext,pAudioCodeContext->codec_id, m_DecodeCodeOptions);

    m_PlayInfo.SetStatus(E_PlayerStatus_Playing);

    m_DecodeThread.Start();
    m_AudioDecodeThread.Start();
    m_DisplayThread.Start();

    m_FFmpegAgnet->InitComplete(FFMPEGPLAYER_NOERROR);
}
//----------------------------------------------------------------------
void  C_FFMpegPlayer::onInitFailed(C_Event &event)
{
    m_FFmpegAgnet->InitComplete(FFMPEGPLAYER_INITMEDIAFAILED);
}
//----------------------------------------------------------------------
void C_FFMpegPlayer::onSaveSanpshotComplete(C_Event &event)
{
    m_FFmpegAgnet->SaveSanpshotComplete();
}
//----------------------------------------------------------------------
void C_FFMpegPlayer::onSaveVideoComplete(C_Event &event)
{
    m_FFmpegAgnet->SaveVideoComplete();
}
//----------------------------------------------------------------------
void C_FFMpegPlayer::onBufferStart(C_Event &event)
{
    m_FFmpegAgnet->StartBuffering();
}
//----------------------------------------------------------------------
void C_FFMpegPlayer:: onBufferComplete(C_Event &event)
{
    m_FFmpegAgnet->ContinuePlaying();
}
//----------------------------------------------------------------------
void C_FFMpegPlayer::onDisplayFrame(C_Event &event)
{
    AVFrame *pDisplayFrame = (AVFrame *)event.GetData();

    m_FFmpegAgnet->PlatformDisplay(pDisplayFrame->data,
                                   pDisplayFrame->width,
                                   pDisplayFrame->height,
                                   pDisplayFrame->format,
                                   pDisplayFrame->linesize);
}
//----------------------------------------------------------------------
void C_FFMpegPlayer::onPlayedSample(C_Event &event)
{
    int64_t *pi64SampleIndex = (int64_t *)event.GetData();
    m_PlayInfo.SetAudioPlayedSample(*pi64SampleIndex);

    if(m_PlayInfo.GetTrackInfo(E_TrackAudio)->bValid)
    {

        int64_t AudioClock = av_rescale_q (*pi64SampleIndex,
                                           m_PlayInfo.GetTrackInfo(E_TrackAudio)->time_base,
                                           AV_TIME_BASE_Q);

        if(AudioClock>0)
        {
            int64_t now = GetClock();
            m_PlayInfo.GetTrackInfo(E_TrackAudio)->ClockInfo.frame_pts = AudioClock;
            m_PlayInfo.GetTrackInfo(E_TrackAudio)->ClockInfo.update_time = now;
        }
        else
        {
            //DEBUG_PRINT("Should not happen!!!\n");
        }
    }
}
//----------------------------------------------------------------------
void C_FFMpegPlayer::onStop(C_Event &event)
{
    m_PlayInfo.SetStatus(E_PlayerStatus_Stoped);
}
//----------------------------------------------------------------------
void C_FFMpegPlayer::onThreadEnd(C_Event &event)
{
    C_ThreadBase *pThread = (C_ThreadBase *)event.GetData();

    bool bAllFinish = true;
    for(int i=0;i<m_ThreadQueue.GetSize();i++)
    {
        C_ThreadBase *pThread = m_ThreadQueue.GetAt(i);
        if(!pThread->IsFinish())
        {
            bAllFinish = false;
            break;
        }
    }

    if(bAllFinish)
    {
        C_Event eventStop(E_EventType_Player_Stop);
        C_EventManager::GetEvnetManager()->ProcessEvent(eventStop);

        m_pInputThread->Close();
    }
}
//----------------------------------------------------------------------
int C_FFMpegPlayer::InitMedia(const char* path,const char* pOptions)
{
    if(m_pInputThread != &m_InputThread)
        Stop();

    if(path == NULL)
        return FFMPEGPLAYER_INITMEDIAFAILED;
    
    m_pInputThread = &m_InputThread;
    m_InputThread.Init(path, pOptions);

    return PlayMedia();
}
//----------------------------------------------------------------------
int C_FFMpegPlayer::InitCustomProtocol(const char* VideoCode, const char* AudioCode , const char* pOptions)
{
    if(m_pInputThread != &m_CustomInputThread)
        Stop();

    if(VideoCode == NULL || AudioCode == NULL)
        return FFMPEGPLAYER_INITMEDIAFAILED;
    
    m_pInputThread = &m_CustomInputThread;
    m_CustomInputThread.Init(VideoCode, AudioCode, pOptions);

    return PlayMedia();
}
//----------------------------------------------------------------------
int C_FFMpegPlayer::PushCustomPacket(const unsigned char* pbyData,int i32Size,int i32Type,int64_t lTimeStamp)
{
    if(m_pInputThread != &m_CustomInputThread)
        return FFMPEGPLAYER_INITMEDIAFAILED;

    return m_CustomInputThread.PushPacket(pbyData, i32Size, i32Type, lTimeStamp);
}
//----------------------------------------------------------------------
int C_FFMpegPlayer::PlayMedia()
{
    if(m_PlayInfo.GetStatus() == E_PlayerStatus_Stoping)
        WaitThreadFinish();

    m_pInputThread->Start();

    return FFMPEGPLAYER_NOERROR;
}
//----------------------------------------------------------------------
int C_FFMpegPlayer::Stop()
{
    if(m_PlayInfo.GetStatus() == E_PlayerStatus_Playing)
        m_PlayInfo.SetStatus(E_PlayerStatus_Stoping);

    StopSaveVideo();

    return FFMPEGPLAYER_NOERROR;
}
//----------------------------------------------------------------------
void C_FFMpegPlayer::WaitThreadFinish()
{

    for(int i=0;i<m_ThreadQueue.GetSize();i++)
    {
        C_ThreadBase *pThread = m_ThreadQueue.GetAt(i);
        pThread->WaitFinish();
    }

}
//----------------------------------------------------------------------
//----------------------------------------------------------------------
const char* C_FFMpegPlayer::GetMediaInfo(const char* path)
{
    static char s_Info[2048] = {0};

    avcodec_register_all();
    av_register_all();
    avformat_network_init();

    s_Info[0] = 0;
    AVFormatContext     *formatCtx= NULL;
    if (avformat_open_input(&formatCtx, path, NULL, NULL) != 0) {
        DEBUG_PRINT("avformat_open_input %s failed!\n", path);
        DEVICE_PRINT("Failed to play Path: %s\n",path);

        return s_Info;
    }

    if (avformat_find_stream_info(formatCtx, NULL) < 0) {
        DEBUG_PRINT("avformat_find_stream_info failed!\n");

        return s_Info;
    }

    char Info[1024] = {0};
    for(int i = 0; i<formatCtx->nb_streams;i++)
    {
        AVCodecContext *avctx;
        AVStream *st = formatCtx->streams[i];
        int ret;

        avctx = avcodec_alloc_context3(NULL);
        if (!avctx)
            break;

        ret = avcodec_parameters_to_context(avctx, st->codecpar);
        if (ret < 0) {
            avcodec_free_context(&avctx);
            break;
        }

        // Fields which are missing from AVCodecParameters need to be taken from the AVCodecContext
        avctx->properties = st->codec->properties;
        avctx->codec      = st->codec->codec;
        avctx->qmin       = st->codec->qmin;
        avctx->qmax       = st->codec->qmax;
        avctx->coded_width  = st->codec->coded_width;
        avctx->coded_height = st->codec->coded_height;

        const char* codec_type = av_get_media_type_string(avctx->codec_type);
        const char* codec_name = avcodec_get_name(avctx->codec_id);

        if(avctx->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            sprintf(Info,"%s{\"type\":\"%s\",\"format\":\"%s\",\"width\":%d,\"height\":%d,\"FPS\":%.2f}",
                    Info,codec_type,codec_name,avctx->width,avctx->height,av_q2d(st->avg_frame_rate));
        }
        else if(avctx->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            sprintf(Info,"%s{\"type\":\"%s\",\"format\":\"%s\",\"samplerate\":%d,\"channel\":%d}",
                    Info,codec_type,codec_name,avctx->sample_rate,avctx->channels);
        }
        else
        {
            sprintf(Info,"%s{\"type\":\"%s\",\"format\":\"%s\"}",
                    Info,codec_type,codec_name);
        }

        if(i!=formatCtx->nb_streams-1)
            sprintf(Info,"%s,",Info);

        avcodec_free_context(&avctx);
    }

    sprintf(s_Info,"{\"streams\":[%s]}",Info);
    avformat_close_input(&formatCtx);

    return s_Info;
}
