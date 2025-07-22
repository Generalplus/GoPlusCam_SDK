//
// Created by generalplus_sa1 on 4/29/16.
//

#include "FFMpegPlayerCore.h"
#include "Defines.h"
#include <unistd.h>
#include <vector>
#include <sys/time.h>
#include <signal.h>

#define VERSION_STR             "V0.9.9.30"

#define MAX_DISPLAY_FACTOR      1.0                         // 1 Sec
#define LOW_LATENCY_FACTOR      0.25                        // 250 ms
#define MAX_DELAY_FACTOR        1.5
#define MIN_DELAY_FACTOR        0.8
#define MAX_STEP_FACTOR         1.05
#define MIN_STEP_FACTOR         1.0

#define FORCE_SYNC_TIME         (AV_TIME_BASE * 0.5)        // 500 ms
#define DEFAULT_DELAY           (0.005 * AV_TIME_BASE)      // 5ms
#define MAX_DECODE_SIZE         1024 * 100                  // 100MB

#define MAX_FRAME_RATE          120
#define DEFAULT_FRAME_RATE      24

#define SYNC_DURATION           (AV_TIME_BASE * 0.1)        //100 ms
#define SYNC_FACTOR             0.3
#define SYNC_DELAY_DIV          10

#define MAX_QUEUED_SIZE         30

//#define FFMPEG_TRACE
//#define FRAME_TIME_TRACE

#define INDENT        1
#define SHOW_VERSION  2
#define SHOW_CONFIG   4
#define SHOW_COPYRIGHT 8

#define PRINT_LIB_INFO(libname, LIBNAME, flags, level)                  \
    {                                             \
        const char *indent = flags & INDENT? "  " : "";                 \
        if (flags & SHOW_VERSION) {                                     \
            unsigned int version = libname##_version();                 \
        DEVICE_PRINT(                                      \
                   "%slib%-11s %2d.%3d.%3d / %2d.%3d.%3d\n",            \
                   indent, #libname,                                    \
                   LIB##LIBNAME##_VERSION_MAJOR,                        \
                   LIB##LIBNAME##_VERSION_MINOR,                        \
                   LIB##LIBNAME##_VERSION_MICRO,                        \
                   AV_VERSION_MAJOR(version), AV_VERSION_MINOR(version),\
                   AV_VERSION_MICRO(version));                          \
        }                                                               \
    }                                                                   \
                                                                \

static void print_all_libs_info(int flags, int level)
{
    PRINT_LIB_INFO(avutil,     AVUTIL,     flags, level);
    PRINT_LIB_INFO(avcodec,    AVCODEC,    flags, level);
    PRINT_LIB_INFO(avformat,   AVFORMAT,   flags, level);
    PRINT_LIB_INFO(swscale,    SWSCALE,    flags, level);
    PRINT_LIB_INFO(swresample, SWRESAMPLE, flags, level);
}

//----------------------------------------------------------------------
C_FFMpegPlayer::C_FFMpegPlayer():
        m_Status(E_PlayerStatus_Stoped),
        m_formatCtx(NULL),m_videoStream(0),
        m_bSaveSnapshot(false),m_bSaveVideo(false),
        m_writeThread(0),m_EncodeID(AV_CODEC_ID_H264),m_FFmpegAgnet(NULL),
        m_sws_ctx(NULL),m_i32PlayRate(1),m_bLowLatency(true),m_fVideofps(0),m_initThread(0),
        m_VideoDecoder(this),m_AudioDecoder(this),
        m_VideoEncoder(this),m_SnapshotEncoder(this),m_bIsStreaming(true),m_bRepeat(false),
        m_bEncodeUsingLocalTime(false),m_bForceToTransCode(false),m_BufferMSTime(0)
{
        m_UserOptions[0] = m_snapShotPath[0] = m_VideoPath[0] = 0;
        pthread_mutex_init(&m_Startlock, NULL);
        pthread_mutex_init(&m_Initlock, NULL);
        pthread_mutex_init(&m_Ratelock, NULL);
        pthread_mutex_init(&m_Contextlock, NULL);
        m_TransCodeOptions[0] = 0;
}
//----------------------------------------------------------------------
C_FFMpegPlayer::~C_FFMpegPlayer()
{
        Stop();
        m_VideoEncoder.CloseVideo();
        Releaseffmpeg();
}
//----------------------------------------------------------------------
int C_FFMpegPlayer::InitMedia(const char* path,const char* pOptions)
{
        strcpy(m_MediaPath, path);
        if(pOptions)
            strcpy(m_UserOptions, pOptions);
    
        bool biniting = IsIniting();
            
        if(!biniting)
        {
            pthread_create(&m_initThread, NULL, initThreadFunction, this);
            return FFMPEGPLAYER_NOERROR;
        }
        else
            return FFMPEGPLAYER_INITMEDIAFAILED;
}
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
//----------------------------------------------------------------------
int C_FFMpegPlayer::PlayMedia()
{
    int i32Ret = FFMPEGPLAYER_NOERROR;
    
    if(pthread_mutex_trylock(&m_Startlock)==0)
    {
        if (m_Status == E_PlayerStatus_Playing)
        {
            pthread_mutex_unlock(&m_Startlock);
            return FFMPEGPLAYER_MEDIAISPLAYING;
        }
        else if (m_Status == E_PlayerStatus_Stoping) {
            void *ret = NULL;
            pthread_join(m_inputThread, &ret);
        }
        
        if (m_formatCtx == NULL) {
            int i32Ret = InitMedia(m_MediaPath);
            pthread_mutex_unlock(&m_Startlock);
            return i32Ret;

        }

        m_Status = E_PlayerStatus_Playing;
        AVCodecContext* pVideoCtx = m_VideoDecoder.GetCodecContext();
        if(!pVideoCtx)
        {
            pthread_mutex_unlock(&m_Startlock);
            m_Status = E_PlayerStatus_Stoped;
            return FFMPEGPLAYER_INITMEDIAFAILED;
        }

        pthread_mutex_init(&m_Ratelock, NULL);
        
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
            
            pthread_create(&m_inputThread, NULL, inputThreadFunction, this);
            pthread_create(&m_decodeThread, NULL, decodeThreadFunction, this);
            pthread_create(&m_displayThread, NULL, displayThreadFunction, this);
        }
        else
        {
            Releaseffmpeg();
            i32Ret = FFMPEGPLAYER_INITMEDIAFAILED;
        }
 
        pthread_mutex_unlock(&m_Startlock);
    }

    return i32Ret;
}
//----------------------------------------------------------------------
int C_FFMpegPlayer::Stop()
{
        if(m_Status == E_PlayerStatus_Playing)
            m_Status = E_PlayerStatus_Stoping;
    
        StopSaveVideo();
        ClearQueue();
        UnLockQueue();
    
        return FFMPEGPLAYER_NOERROR;
}
//----------------------------------------------------------------------
int C_FFMpegPlayer::SaveVideo(const char *path)
{
        if(m_Status != E_PlayerStatus_Playing)
            return FFMPEGPLAYER_SAVEVIDEOFAILED;
    
        if(m_writeThread!=0)
        {
                StopSaveVideo();
                void* ret = NULL;
                pthread_join(m_writeThread,&ret);
                m_writeThread = 0;
        }
    
        m_VideoEncoder.init(m_VideoDecoder.GetCodecContext(), m_AudioDecoder.GetCodecContext());
        strcpy(m_VideoPath, path);
        pthread_create(&m_writeThread, NULL, WriteThreadFunction, this);
    
        return FFMPEGPLAYER_NOERROR;
}
//----------------------------------------------------------------------
void* C_FFMpegPlayer::initThreadFunction(void *param)
{
    C_FFMpegPlayer* pPlayer = (C_FFMpegPlayer*)param;
    
    pPlayer->init();
    
    return FFMPEGPLAYER_NOERROR;
}
//----------------------------------------------------------------------
void* C_FFMpegPlayer::decodeThreadFunction(void *param)
{
        C_FFMpegPlayer* pPlayer = (C_FFMpegPlayer*)param;

        pPlayer->decode();

        return FFMPEGPLAYER_NOERROR;
}
//----------------------------------------------------------------------
void* C_FFMpegPlayer::WriteThreadFunction(void *param)
{
        C_FFMpegPlayer* pPlayer = (C_FFMpegPlayer*)param;

        pPlayer->writeVideo();

        return FFMPEGPLAYER_NOERROR;
}
//----------------------------------------------------------------------
void* C_FFMpegPlayer::displayThreadFunction(void *param)
{
        C_FFMpegPlayer* pPlayer = (C_FFMpegPlayer*)param;
    
        pPlayer->display();
    
        return FFMPEGPLAYER_NOERROR;
}
//----------------------------------------------------------------------
void* C_FFMpegPlayer::inputThreadFunction(void *param)
{
        C_FFMpegPlayer* pPlayer = (C_FFMpegPlayer*)param;
    
        pPlayer->input();
    
        return FFMPEGPLAYER_NOERROR;
}
//----------------------------------------------------------------------
void C_FFMpegPlayer::PlayedSample(int64_t i64SampleIndex)
{
    m_AudioPlayedSample = i64SampleIndex;
    
    if(m_AudioDecoder.GetDecodeFrame())
    {
    
        int64_t AudioClock = av_rescale_q (i64SampleIndex,
                                           m_formatCtx->streams[m_audioStream]->time_base,
                                           AV_TIME_BASE_Q);
        
        if(AudioClock>0)
        {
            int64_t now = GetClock();
            m_Clock[AUDIO_CLOCK].frame_pts = AudioClock;
            m_Clock[AUDIO_CLOCK].update_time = now;
        }
        else
        {
            //DEBUG_PRINT("Should not happen!!!\n");
        }
    
    }
}
//----------------------------------------------------------------------
void C_FFMpegPlayer::SeekInternal(int64_t position)
{
    pthread_mutex_lock(&m_Contextlock);
    
    if(m_formatCtx)
    {
        m_FFmpegAgnet->DoAudioRampDown();
        ClearQueue();
        
        DEBUG_PRINT("Seek to %lld\n",position);
 
        int ret = avformat_seek_file(m_formatCtx, -1, INT64_MIN, position, INT64_MAX, 0);
        //avformat_flush(m_formatCtx);
        //int ret = av_seek_frame(m_formatCtx, -1, position, AVSEEK_FLAG_ANY);
        
        if (ret < 0)
        {
            DEBUG_PRINT("Seek to %lld failed! Error: %d \n",position,ret);
        }
        else
        {
            resetClock();
            
            while(m_VideoDecoder.IsDisplaying() && m_Status== E_PlayerStatus_Playing && m_i32PlayRate != PAUSE_PLAYRATE);
            
            m_Clock[VIDEO_CLOCK].frame_pts = position;
            m_Clock[AUDIO_CLOCK].frame_pts = position;
            
        }
    }
    
    pthread_mutex_unlock(&m_Contextlock);
    
}
//----------------------------------------------------------------------
int C_FFMpegPlayer::init()
{
    DEBUG_PRINT("init start\n");
    
    if(pthread_mutex_trylock(&m_Initlock)==0) {
        
        if (m_formatCtx != NULL) {
            Stop();
            void *ret = NULL;
            pthread_join(m_inputThread, &ret);
        }
        
        ClearQueue();
        UnLockQueue();
        
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
            pthread_mutex_unlock(&m_Initlock);
            m_Status = E_PlayerStatus_Stoped;
            m_FFmpegAgnet->InitComplete(FFMPEGPLAYER_INITMEDIAFAILED);
            return FFMPEGPLAYER_INITMEDIAFAILED;
        }
        av_dict_free(&opts);
        
        if (avformat_find_stream_info(m_formatCtx, NULL) < 0) {
            DEBUG_PRINT("avformat_find_stream_info failed!\n");
            pthread_mutex_unlock(&m_Initlock);
            
            m_FFmpegAgnet->InitComplete(FFMPEGPLAYER_INITMEDIAFAILED);
            return FFMPEGPLAYER_INITMEDIAFAILED;
        }
        
        m_bIsStreaming = true;
        
        if(m_formatCtx->pb)
        {
            if(m_formatCtx->pb->seekable == AVIO_SEEKABLE_NORMAL)
                m_bIsStreaming = false;
        }
        
        m_videoStream = m_VideoDecoder.OpenCodec(AVMEDIA_TYPE_VIDEO,m_DecodeCodeOptions);
        if(m_videoStream<0)
            return FFMPEGPLAYER_INITMEDIAFAILED;
        
        m_audioStream = m_AudioDecoder.OpenCodec(AVMEDIA_TYPE_AUDIO,m_DecodeCodeOptions);
        if(m_audioStream>=0)
            m_AudioDecoder.initAudioResample();
        
        AVRational frame_rate = GetFrameRate();
        m_fVideofps = (float)frame_rate.num / (float)frame_rate.den;
        
        pthread_mutex_unlock(&m_Initlock);
        
        m_FFmpegAgnet->InitComplete(FFMPEGPLAYER_NOERROR);
    }
    
    DEBUG_PRINT("init end\n");
    
    return FFMPEGPLAYER_NOERROR;
}
//----------------------------------------------------------------------
AVRational C_FFMpegPlayer::GetFrameRate()
{
    AVRational frame_rate = av_guess_frame_rate(m_formatCtx ,m_formatCtx->streams[m_videoStream], NULL);
    float fVideofps = (float)frame_rate.num / (float)frame_rate.den;
    if(fVideofps > MAX_FRAME_RATE || fVideofps < 0 )
        fVideofps = DEFAULT_FRAME_RATE;
    
    return av_make_q(fVideofps,1);
}
//----------------------------------------------------------------------
int C_FFMpegPlayer::input()
{
    
    AVPacket        		packet;
    m_FrameCnt              = 0;
    m_InputCnt              = 0;
    m_AudioPlayedSample     = 0;
    m_lastDecodeSize        = 0;

    int64_t                 prevTime = GetClock();
    int64_t                 lastVideo = 0;
    int                     i32Ret = -1;
    
    memset(&m_SeekInfo,0x00,sizeof(m_SeekInfo));
    memset(&m_ThreadTimeInfo,0x00,sizeof(m_ThreadTimeInfo));
    
    resetClock();
    
    while(m_Status== E_PlayerStatus_Playing) {
        
        pthread_mutex_lock(&m_Contextlock);
        if(m_formatCtx)
            i32Ret = av_read_frame(m_formatCtx, &packet);
        pthread_mutex_unlock(&m_Contextlock);
        
        if(i32Ret>=0)
        {
            m_InputCnt+=packet.size;
            
            if(m_bSaveVideo && m_VideoEncoder.GetEncodeType() == E_EncodeType_RawStream)
            {
                AVPacket *tempkt = new AVPacket;
                av_init_packet(tempkt);
                av_copy_packet(tempkt, &packet);
                m_VideoEncoder.pushRawPacket(tempkt);
            }
            
            if(packet.stream_index==m_videoStream && m_VideoDecoder.GetCodecContext())
            {
                AVPacket *tempkt = new AVPacket;
                av_init_packet(tempkt);
                av_copy_packet(tempkt, &packet);
                m_VideoDecoder.pushDecodePacket(tempkt);
                lastVideo = packet.pts;
                m_lastDecodeSize = packet.size;
                
            }
            else if(packet.stream_index==m_audioStream && m_AudioDecoder.GetCodecContext())
            {
                if(!seekingAudio(&packet))
                {
                    m_AudioDecoder.Decode(&packet);
                }

            }
            
            av_free_packet(&packet);
        }
        else
        {
            if(!m_bIsStreaming)
            {
                if(m_bRepeat && !m_VideoDecoder.IsDisplaying() && !m_VideoDecoder.IsDecoding())
                    Seek(0);
                else
                    DoDelay(DEFAULT_DELAY);
            }
            else
                break;
        }
        
        inputDelay(prevTime,lastVideo);

        if(m_SeekInfo.bAssignSeek)
        {
            m_VideoDecoder.FlushBuffer();
            if(m_AudioDecoder.GetCodecContext())
                m_AudioDecoder.FlushBuffer();
            
            SeekInternal(m_SeekInfo.AssignSeekPosition);
            
            m_AudioPlayedSample = 0;
            
            m_SeekInfo.SeekingPosition = m_SeekInfo.AssignSeekPosition;
            m_SeekInfo.bAssignSeek = false;
            m_SeekInfo.bSeeking = true;
            m_SeekInfo.bSeekingPreview = true;
            if(m_AudioDecoder.GetCodecContext())
            {
                m_SeekInfo.bSeekAudio = true;
            }
        }
        
        prevTime = GetClock();
    }

    DEBUG_PRINT("input break\n");
    
    // wait for display and decode queue is empty
    while((m_VideoDecoder.IsDisplaying() || m_VideoDecoder.IsDecoding()) &&
           m_Status == E_PlayerStatus_Playing)
    {
        DoDelay(DEFAULT_DELAY);
    }
    
    Stop();
    
    void *ret = NULL;
    pthread_join(m_decodeThread, &ret);
    pthread_join(m_displayThread, &ret);
    
    m_FFmpegAgnet->ClearAudioQueue();
    
    Releaseffmpeg();
    m_Status = E_PlayerStatus_Stoped;
    
    DEBUG_PRINT("input End\n");
    
    return FFMPEGPLAYER_NOERROR;
}
//----------------------------------------------------------------------
void C_FFMpegPlayer::inputDelay(int64_t prevTime,int64_t lastVideoTime)
{
    while(m_i32PlayRate == PAUSE_PLAYRATE &&
          !m_SeekInfo.bAssignSeek &&
          !m_SeekInfo.bSeeking &&
          m_Status== E_PlayerStatus_Playing)
    {
        DoDelay(PAUSE_DEALY);
    }
    
    if(m_bIsStreaming)
        return ;
    
    if(m_AudioDecoder.GetCodecContext())
    {

        int64_t AudioBufferPts = m_AudioDecoder.GetQueueIndex() - m_AudioPlayedSample;
        
        int64_t AudioBufferTime = av_rescale_q (AudioBufferPts,
                                                m_formatCtx->streams[m_audioStream]->time_base,
                                                AV_TIME_BASE_Q);
        
        if(AudioBufferTime > SYNC_DURATION)
        {
            int64_t delayTime = m_ThreadTimeInfo.AvgDecodeTime * m_VideoDecoder.GetDecodeSize();
            
            if(delayTime > AudioBufferTime)
                delayTime = AudioBufferTime * SYNC_FACTOR;
            
            //if(!m_SeekInfo.bAssignSeek &&!m_SeekInfo.bSeeking)
            //    DEBUG_PRINT("delayTime: %lld\n",delayTime);
            
            int64_t sleepUnit = delayTime/SYNC_DELAY_DIV;
            if(sleepUnit>SYNC_DURATION)
                sleepUnit = SYNC_DURATION;
            
            int64_t timeTarget = GetClock() + delayTime;
            
            while(!m_SeekInfo.bAssignSeek &&
                  !m_SeekInfo.bSeeking &&
                  timeTarget>GetClock() &&
                  m_Status== E_PlayerStatus_Playing)
            {
                DoDelay(sleepUnit);
            }
            
        }
        
    }
    else
    {
        int64_t delayTime = m_ThreadTimeInfo.AvgDecodeTime * m_VideoDecoder.GetDecodeSize();
        int64_t timeTarget = GetClock() + delayTime;
        
        int64_t sleepUnit = delayTime/SYNC_DELAY_DIV;
        if(sleepUnit>SYNC_DURATION)
            sleepUnit = SYNC_DURATION;
        
        while(!m_SeekInfo.bAssignSeek &&
              !m_SeekInfo.bSeeking &&
              timeTarget>GetClock() &&
              m_Status== E_PlayerStatus_Playing)
        {
            DoDelay(sleepUnit);
        }
    }
    
}
//----------------------------------------------------------------------
int C_FFMpegPlayer::decode()
{
        while(m_Status ==E_PlayerStatus_Playing)
        {
            CheckAndDoPause();
            if(m_Status!= E_PlayerStatus_Playing)
                break;
            
            AVPacket *pPacket = m_VideoDecoder.PopDecodePacket();
            if(pPacket == NULL)
                continue;
            
            int64_t timeBefore = GetClock();
        
            if(m_VideoDecoder.Decode(pPacket))
            {
                if(m_bSaveSnapshot)
                    EncodeSnapshot();
            }
            
            m_ThreadTimeInfo.DecodeTime = GetClock() - timeBefore;
            //DEBUG_PRINT("decode time: %lld\n",m_ThreadTimeInfo.DecodeTime);
            UpdateAvgThreadTime(m_ThreadTimeInfo.DecodeTime,&m_ThreadTimeInfo.AvgDecodeTime);
            
            if(!m_bIsStreaming && !m_SeekInfo.bSeeking)
                ThreadDelay(m_ThreadTimeInfo.AvgDisplayTime - m_ThreadTimeInfo.AvgDecodeTime,m_VideoDecoder.GetDisplaySize());
            
            av_free_packet(pPacket);
            delete pPacket;

            int i32DecodeSize = m_VideoDecoder.GetDecodeSize();
            int i32DisplaySize= m_VideoDecoder.GetDisplaySize();

            if(((i32DecodeSize > MAX_QUEUED_SIZE || i32DisplaySize > MAX_QUEUED_SIZE) &&
               m_bIsStreaming) && m_BufferMSTime == 0)
            {
                DEVICE_PRINT("Over max queue size !!! Decode: %d Display: %d Decode Time: %lld Avg %lld Display Time: %lld Avg %lld\n",
                             i32DecodeSize,
                             i32DisplaySize,
                             m_ThreadTimeInfo.DecodeTime,
                             m_ThreadTimeInfo.AvgDecodeTime,
                             m_ThreadTimeInfo.DisplayTime,
                             m_ThreadTimeInfo.AvgDisplayTime);
            }
            
        }
        DEBUG_PRINT("decode End\n");
        return FFMPEGPLAYER_NOERROR;
}
//----------------------------------------------------------------------
int C_FFMpegPlayer::display()
{

        while(m_Status == E_PlayerStatus_Playing)
        {
            CheckAndDoPause();
            if(m_Status!= E_PlayerStatus_Playing)
                break;
            
            AVFrame *pFrame = m_VideoDecoder.PopDisplay();
            if(m_Status!=E_PlayerStatus_Playing)
                break;
        
            if(pFrame == NULL)
                continue;
            
            int64_t timeBefore = GetClock();
            
            m_FrameCnt++;
            if(!seekingFrame(pFrame))
            {
                displayFrame(pFrame);
                syncFrame(pFrame);
                DoLowLatency();
            }
            
            av_freep(&pFrame->data[0]);
            av_frame_unref(pFrame);
            av_frame_free(&pFrame);
        
            m_ThreadTimeInfo.DisplayTime = GetClock() - timeBefore;
            UpdateAvgThreadTime(m_ThreadTimeInfo.DisplayTime,&m_ThreadTimeInfo.AvgDisplayTime);

        }
    
        DEBUG_PRINT("display end\n");
    
    
        return FFMPEGPLAYER_NOERROR;
}
//----------------------------------------------------------------------
int C_FFMpegPlayer::displayFrame(AVFrame* pFrame)
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
    
        pushTranscodeFrame(pDisplayFrame);
    
        m_FFmpegAgnet->PlatformDisplay(pDisplayFrame->data,
                                       GetWidth(),
                                       GetHeight(),
                                       pDisplayFrame->format,
                                       pDisplayFrame->linesize);
    

        return FFMPEGPLAYER_NOERROR;
}
//----------------------------------------------------------------------
int C_FFMpegPlayer::pushTranscodeFrame(AVFrame* pFrame)
{
        if(m_bSaveVideo && m_VideoEncoder.GetEncodeType() == E_EncodeType_Transcode)
        {
            m_VideoEncoder.pushTranscodeFrame(pFrame);
        }
    
        return FFMPEGPLAYER_NOERROR;
}
//----------------------------------------------------------------------
int C_FFMpegPlayer::syncFrame(AVFrame* pFrame)
{
        if(m_BufferMSTime == 0)
        {
            if(m_bLowLatency &&
               !m_AudioDecoder.GetCodecContext())
                return FFMPEGPLAYER_NOERROR;
        }

        int64_t pts,ptsOrg,during;
        ptsOrg = av_frame_get_best_effort_timestamp(pFrame);
    
        pts = ptsOrg;
        if(m_formatCtx->streams[m_videoStream]->start_time != AV_NOPTS_VALUE)
            pts-=m_formatCtx->streams[m_videoStream]->start_time;
    
        pts = av_rescale_q (pts,
                            m_formatCtx->streams[m_videoStream]->time_base,
                            AV_TIME_BASE_Q );
    
        during = av_rescale_q (pFrame->pkt_duration,
                               m_formatCtx->streams[m_videoStream]->time_base,
                               AV_TIME_BASE_Q );
    
        int64_t delay = 0;
    
        if(m_Clock[VIDEO_CLOCK].frame_pts==0)
            m_Clock[VIDEO_CLOCK].frame_pts = pts;
    
        if(ptsOrg != AV_NOPTS_VALUE && m_Clock[VIDEO_CLOCK].frame_pts!=AV_NOPTS_VALUE)
            delay = pts - m_Clock[VIDEO_CLOCK].frame_pts;
    
        int64_t now = GetClock();
    
        if(m_Clock[VIDEO_CLOCK].update_time==0)
            m_Clock[VIDEO_CLOCK].update_time = now;
    
        if(delay<0)
        {
            DEBUG_PRINT("delay < 0 , New frame time stamp is too old! Reset the time stamp!\n");
            delay = 0;
            m_Clock[VIDEO_CLOCK].update_time = now;
        }
    
        int64_t escTime = now -  m_Clock[VIDEO_CLOCK].update_time;
        int64_t timeDelay =  delay - escTime;

        int64_t FrameDiff = 0;
        int64_t AudioPts = m_Clock[AUDIO_CLOCK].frame_pts;
    
        if(m_AudioDecoder.GetCodecContext())
            FrameDiff += m_AudioDecoder.GetAudioStartAlign();
    
        if(m_Clock[AUDIO_CLOCK].update_time!=0 &&
           ptsOrg != AV_NOPTS_VALUE &&
           m_Clock[AUDIO_CLOCK].Sync_pts != m_Clock[AUDIO_CLOCK].frame_pts)
        {
            FrameDiff += pts - AudioPts;
            timeDelay = FrameDiff;
        }
    
        if(m_bIsStreaming && m_BufferMSTime>0)
        {
            if(timeDelay<0)
            {
                int i32QueueSize = m_fVideofps * ((float)m_BufferMSTime / 1000.0);
                if(m_VideoDecoder.GetDisplaySize() == 0)
                {
                    int64_t frameTime = AV_TIME_BASE * (1 / m_fVideofps);
                    timeDelay = ((i32QueueSize - m_VideoDecoder.GetDisplaySize()) * frameTime)*2;
                    m_FFmpegAgnet->StartBuffering();
                    int64_t StartCheck = GetClock();
                    
                    while(m_Status== E_PlayerStatus_Playing &&
                          i32QueueSize > m_VideoDecoder.GetDisplaySize() &&
                          timeDelay > GetClock() - StartCheck
                          )
                    {
                        DEBUG_PRINT("Buffering...\n");
                        DoDelay(frameTime);
                    }
                    m_Clock[VIDEO_CLOCK].update_time = 0;
                    m_FFmpegAgnet->ContinuePlaying();
                }
                
            }
            else
            {
                DoDelay(timeDelay);
                m_Clock[VIDEO_CLOCK].update_time = GetClock();
            }
        }
        else
        {
            if(timeDelay > MAX_DELAY_FACTOR * during)
                timeDelay = MAX_DELAY_FACTOR * during;
            //else if(timeDelay < 0)
            //    timeDelay = 0;
            
            int i32PlayRate = m_i32PlayRate; // trick to get rate with no mutex
            if(i32PlayRate!=0 && timeDelay>0)
                timeDelay = timeDelay / i32PlayRate;
            
            if(timeDelay>0)
                DoDelay(timeDelay);
            
            m_Clock[VIDEO_CLOCK].update_time = GetClock();
            
            if(timeDelay<0)
                m_Clock[VIDEO_CLOCK].update_time+=timeDelay;
        }
    
#if defined(DEBUG) && defined(FRAME_TIME_TRACE)
    
    DEBUG_PRINT("now: %.10lld timeDelay: %.10lld FrameDiff: %.10lld pts: %lld prev: %lld audio: %lld delay %lld escTime %lld ptsOrg %lld Packet %d Display %d\n",
           now,
           timeDelay,
           FrameDiff,
           pts,
           m_Clock[VIDEO_CLOCK].frame_pts,
           m_Clock[AUDIO_CLOCK].frame_pts,
           delay,
           escTime,
           ptsOrg,
           m_VideoDecoder.GetDecodeSize(),
           m_VideoDecoder.GetDisplaySize()
           );
#endif
    
        m_Clock[VIDEO_CLOCK].frame_pts = pts;
        m_Clock[VIDEO_CLOCK].Sync_pts = pts;
        m_Clock[AUDIO_CLOCK].Sync_pts = AudioPts;
    
        return FFMPEGPLAYER_NOERROR;
}
//----------------------------------------------------------------------
bool C_FFMpegPlayer::seekingFrame(AVFrame* pFrame)
{
    if(m_SeekInfo.bSeeking)
    {
        
        int64_t pts;
        pts = av_frame_get_best_effort_timestamp(pFrame);
        if(m_formatCtx->streams[m_videoStream]->start_time != AV_NOPTS_VALUE)
            pts-=m_formatCtx->streams[m_videoStream]->start_time;
        
        pts = av_rescale_q (pts,
                            m_formatCtx->streams[m_videoStream]->time_base,
                            AV_TIME_BASE_Q);
        
        //DEBUG_PRINT("Step Frame: %lld \n",pts);
        
        if(pts >= m_SeekInfo.SeekingPosition)
        {
            /*if(m_AudioDecoder.GetCodecContext())
            {
                if(pts - m_Clock[AUDIO_CLOCK].frame_pts>=0)
                    m_SeekInfo.bSeeking = false;
            }
            else*/
                m_SeekInfo.bSeeking = false;
        }
        
        if(m_SeekInfo.bSeekingPreview)
        {
            m_SeekInfo.bSeekingPreview = false;
            m_Clock[VIDEO_CLOCK].update_time = GetClock();
            return false;
        }
        
    }
    
    
    return m_SeekInfo.bSeeking;
}
//----------------------------------------------------------------------
bool C_FFMpegPlayer::seekingAudio(AVPacket* pPacket)
{
    if(m_SeekInfo.bSeekAudio)
    {
        
        int64_t pts;
        pts = pPacket->pts;
        if(m_formatCtx->streams[m_audioStream]->start_time != AV_NOPTS_VALUE)
            pts-=m_formatCtx->streams[m_audioStream]->start_time;
        
        pts = av_rescale_q (pts,
                            m_formatCtx->streams[m_audioStream]->time_base,
                            AV_TIME_BASE_Q);
        
    
        //DEBUG_PRINT("Step Audio Frame: %lld \n",pts);
        
        if(pts >= m_SeekInfo.SeekingPosition)
        {
            m_SeekInfo.bSeekAudio = false;
            m_Clock[AUDIO_CLOCK].frame_pts = pts;
            m_Clock[AUDIO_CLOCK].update_time = GetClock();
            m_FFmpegAgnet->ClearAudioQueue(); // Clear audio queue again , make sure this is no queuing sample.
            m_FFmpegAgnet->DoAudioRampUp();
        }
    }
    
    return m_SeekInfo.bSeekAudio;
}
//----------------------------------------------------------------------
void C_FFMpegPlayer::DoLowLatency()
{
    if(pthread_mutex_trylock(&m_Ratelock)==0)
    {
        if(m_i32PlayRate != PAUSE_PLAYRATE)
        {
            if(m_bLowLatency)
            {
                int i32QueueSize = m_VideoDecoder.GetDisplaySize();
                if(i32QueueSize > m_fVideofps * LOW_LATENCY_FACTOR )
                {
                    m_i32PlayRate = SPEEDUP_PLAYRATE;
                    m_FFmpegAgnet->SetAudioRate(m_i32PlayRate);
                }
                else
                {
                    m_i32PlayRate = DEFAULT_PLAYRATE;
                    m_FFmpegAgnet->SetAudioRate(m_i32PlayRate);
                }
            }
            else
            {
                m_i32PlayRate = DEFAULT_PLAYRATE;
                m_FFmpegAgnet->SetAudioRate(m_i32PlayRate);
            }
        }
        
        pthread_mutex_unlock(&m_Ratelock);
    }
    
}
//----------------------------------------------------------------------
int C_FFMpegPlayer::writeVideo()
{
        if(m_VideoDecoder.GetCodecContext()->codec_id == AV_CODEC_ID_MJPEG && m_bForceToTransCode)
            m_VideoEncoder.SetEncodeType(E_EncodeType_Transcode);
        else
            m_VideoEncoder.SetEncodeType(E_EncodeType_RawStream);

        if(m_VideoEncoder.GetEncodeType() == E_EncodeType_Transcode)
            m_bSaveVideo = m_VideoEncoder.CreateEncodeStream(m_EncodeID,m_VideoPath,m_fVideofps,m_TransCodeOptions);
        else
        {
            bool bIsRTSP = true;
            bool bUsingLocalTime = m_bEncodeUsingLocalTime;
            if (strstr(m_MediaPath, "rtsp")==NULL &&
                strcmp(strrchr(m_MediaPath, '.'),".sdp") != 0)
            {
                bIsRTSP = false;
                bUsingLocalTime = true;
            }
            
            m_bSaveVideo = m_VideoEncoder.CreateEncodeRawStream(m_VideoPath,m_fVideofps,bIsRTSP,bUsingLocalTime);
        }
    
        while(m_bSaveVideo && m_Status == E_PlayerStatus_Playing)
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

        m_writeThread = 0;
        m_FFmpegAgnet->SaveVideoComplete();
    
        return FFMPEGPLAYER_NOERROR;
}
//----------------------------------------------------------------------
int C_FFMpegPlayer::EncodeSnapshot()
{
        m_bSaveSnapshot = false;
    
        m_SnapshotEncoder.Encode(m_VideoDecoder.GetDecodeFrame(),
                                 m_VideoDecoder.GetCodecContext(),
                                 m_snapShotPath);
    
        m_FFmpegAgnet->SaveSanpshotComplete();
    
        return FFMPEGPLAYER_NOERROR;
}
//----------------------------------------------------------------------
int C_FFMpegPlayer::Releaseffmpeg()
{
        if(m_i32PlayRate == PAUSE_PLAYRATE)
            Resume();
    
        pthread_mutex_lock(&m_Contextlock);
        m_VideoDecoder.Close();
        m_AudioDecoder.Close();

        if(m_formatCtx)
        {
            avformat_close_input(&m_formatCtx);
            m_formatCtx = NULL;
        }
        pthread_mutex_unlock(&m_Contextlock);
    
        return FFMPEGPLAYER_NOERROR;
}
//----------------------------------------------------------------------

