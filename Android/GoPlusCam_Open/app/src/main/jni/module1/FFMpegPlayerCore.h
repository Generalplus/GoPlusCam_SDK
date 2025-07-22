//
// Created by generalplus_sa1 on 4/29/16.
//

#ifndef FFMPEGTEST_C_FFMPEGPLAYER_H
#define FFMPEGTEST_C_FFMPEGPLAYER_H

#include "CodecDefine.h"

#include "PictureEncoder.h"
#include "VideoEncoder.h"
#include "AudioDecoder.h"
#include "FrameExtractor.h"
#if defined(__APPLE__)
#include "VideoDecoder_iOS.h"
#else
#include "VideoDecoder_Android.h"
#endif
#include "QueueManager.h"
//----------------------------------------------------------------------

typedef enum
{
    E_PlayerStatus_Stoped,
    E_PlayerStatus_Playing,
    E_PlayerStatus_Stoping,

}E_PlayerStatus;

typedef struct ClockInfo
{
    int64_t    update_time;
    int64_t    frame_pts;
    int64_t    Sync_pts;
    
}S_ClockInfo;

typedef struct SeekInfo
{
    bool       bAssignSeek;
    int64_t    AssignSeekPosition;
    bool       bSeeking;
    bool       bSeekingPreview;
    int64_t    SeekingPosition;
    bool       bSeekAudio;
}S_SeekInfo;

typedef struct ThreadTimeInfo
{
    int64_t    DecodeTime;
    int64_t    DisplayTime;
    
    int64_t    AvgDecodeTime;
    int64_t    AvgDisplayTime;
    
    
}S_ThreadTimeInfo;

#define VIDEO_CLOCK             0
#define AUDIO_CLOCK             1

#define PAUSE_PLAYRATE          0
#define DEFAULT_PLAYRATE        1
#define SPEEDUP_PLAYRATE        2

#define PAUSE_DEALY             (AV_TIME_BASE * 0.1)        // 100 ms
//----------------------------------------------------------------------
class C_FFMpegPlayer : public I_ContextManager
{
public:
    C_FFMpegPlayer();

    ~C_FFMpegPlayer();

    int InitMedia(const char* path,  const char* pOptions = NULL);
    const char* GetMediaInfo(const char* path);
    
    int PlayMedia();

    int Stop();

    int Pause()
    {
        pthread_mutex_lock(&m_Ratelock);
        m_i32PlayRate = PAUSE_PLAYRATE;
        if(m_FFmpegAgnet)
            m_FFmpegAgnet->SetAudioRate(m_i32PlayRate);
        pthread_mutex_unlock(&m_Ratelock);
        
        pthread_mutex_lock(&m_Contextlock);
        
        if(m_formatCtx)
            av_read_pause(m_formatCtx);
        
        pthread_mutex_unlock(&m_Contextlock);
        
        return FFMPEGPLAYER_NOERROR;
    }
    
    int Resume()
    {
        pthread_mutex_lock(&m_Ratelock);
        m_i32PlayRate = DEFAULT_PLAYRATE;
        if(m_FFmpegAgnet)
            m_FFmpegAgnet->SetAudioRate(m_i32PlayRate);
        pthread_mutex_unlock(&m_Ratelock);
        
        reflashUpdateTime();
        
        pthread_mutex_lock(&m_Contextlock);
        
        if(m_formatCtx)
            av_read_play(m_formatCtx);
        
        pthread_mutex_unlock(&m_Contextlock);
        
        return FFMPEGPLAYER_NOERROR;
    }
    
    int SaveSnapshot(const char *path)
    {
        strcpy(m_snapShotPath, path);
        m_bSaveSnapshot = true;
        
        return FFMPEGPLAYER_NOERROR;
    }

    int ForceToTranscode(bool bForce)
    {
        m_bForceToTransCode = bForce;
        
        return FFMPEGPLAYER_NOERROR;
    }
    
    int SaveVideo(const char *path);

    int StopSaveVideo()
    {
        m_bSaveVideo = false;
        m_VideoEncoder.Stop();

        return m_VideoEncoder.GetContainerType();
    }

    int SetTransCodeOptions(const char *options)
    {
        strcpy(m_TransCodeOptions, options);
        return FFMPEGPLAYER_NOERROR;
    }
    
    int SetDecodeOptions(const char *options)
    {
        strcpy(m_DecodeCodeOptions, options);
        return FFMPEGPLAYER_NOERROR;
    }
    
    
    int GetWidth()
    {
       AVCodecContext* pCodecCtx = m_VideoDecoder.GetCodecContext();
        
        if(pCodecCtx==NULL)
            return 0;

        return pCodecCtx->width;
    }

    int GetHeight()
    {
        AVCodecContext* pCodecCtx = m_VideoDecoder.GetCodecContext();
        
        if(pCodecCtx==NULL)
            return 0;

        return pCodecCtx->height;
    }
    
    int GetStatus()
    {
        return m_Status;
    }
    
    AVCodecContext *GetCodecContext(E_Contextype eType)
    {
        if(E_ContextType_Video == eType)
            return m_VideoDecoder.GetCodecContext();
        else
            return m_AudioDecoder.GetCodecContext();
    }
    
    int GetCodecID()
    {
        int i32CodecID = 0;
        if( m_VideoDecoder.GetCodecContext())
            i32CodecID =  m_VideoDecoder.GetCodecContext()->codec_id;
        
        return i32CodecID;
    }
    
    void PlayedSample(int64_t i64SampleIndex);

    
    int64_t GetRevSizeCnt()     {return m_InputCnt;}
    int64_t GetFrameCnt()       {return m_FrameCnt;}
    
    int64_t GetDuration()
    {
        if(m_formatCtx==NULL)
            return 0;
        
        return m_formatCtx->duration;
    }
    
    int64_t GetPosition()
    {
        if(m_formatCtx==NULL)
            return 0;
        
        if(IsIniting())
            return 0;
        
        if(m_SeekInfo.bAssignSeek || m_SeekInfo.bSeeking)
            return m_SeekInfo.AssignSeekPosition;
        
        return m_Clock[VIDEO_CLOCK].frame_pts;
    }
    
    void Seek(int64_t position)
    {
        m_SeekInfo.bAssignSeek = true;
        m_SeekInfo.AssignSeekPosition = position;
    }
    
    void SetLowLatency(bool bEnable)                { m_bLowLatency = bEnable;}
    void SetEncodeByLocalTime(bool bEnable)         { m_bEncodeUsingLocalTime = bEnable;}
    void SetBufferingTime(int64_t msTime)           { m_BufferMSTime = msTime;}
    
    int ExtractFrame(const char* VideoPath,
                     const char* SavePath,
                     int64_t frameIdx)
    {
        return m_FrameExtractor.ExtractFrame(VideoPath,SavePath,frameIdx);
    }
    
    static void* initThreadFunction(void *param);
    static void* inputThreadFunction(void *param);
    static void* decodeThreadFunction(void *param);
    static void* displayThreadFunction(void *param);
    static void* WriteThreadFunction(void *param);
    
    void setAgent(I_FFmpegAgnet *pAgent)
    {
        m_FFmpegAgnet = pAgent;
        m_VideoDecoder.setAgent(pAgent);
        m_AudioDecoder.setAgent(pAgent);
    }
    
    //I_ContextManager
    virtual AVFormatContext *GetFormat() { return m_formatCtx; }
    virtual  AVStream *GetStream(E_StreamType eType)
    {
        AVStream *pCodec = NULL;
        
        if(E_StreamType_Video == eType)
            pCodec = m_formatCtx->streams[m_videoStream];
        else
            pCodec = m_formatCtx->streams[m_audioStream];
        
        return pCodec;
    }
    
    void setRepeat(bool bRepeat) { m_bRepeat = bRepeat; }
    
    virtual int  GetVideoStreamID() { return m_videoStream; }
    virtual int  GetAudioStreamID() { return m_audioStream; }
    
    AVFrame* DupDecodedFrame()      { return m_VideoDecoder.DupDecodedFrame();}
    
    void EnableDebugMessage(bool bEnable) { m_VideoDecoder.EnableDebugMessage(bEnable); }
    
private:
    
    AVRational  GetFrameRate();
    
    bool IsIniting()
    {
        bool biniting = false;
        
        if(m_initThread != 0)
            if(pthread_kill(m_initThread, 0) == 0)
                biniting = true;
        
        return biniting;
    }
    
    int EncodeSnapshot();
    
    int init();
    int input();
    int decode();
    int display();
    
    int pushTranscodeFrame(AVFrame* pFrame);
    int displayFrame(AVFrame* pFrame);
    int syncFrame(AVFrame* pFrame);
    bool seekingFrame(AVFrame* pFrame);
    bool seekingAudio(AVPacket* pPacket);
    
    int writeVideo();
    int Releaseffmpeg();
    
    void inputDelay(int64_t prevTime,int64_t lastVideoTime);
    void DoLowLatency();
    
    void SeekInternal(int64_t position);
    
    void resetClock()
    {
        memset(m_Clock,0x00,sizeof(m_Clock));
    }
    
    void reflashUpdateTime()
    {
        for(int i=0;i<2;i++)
           m_Clock[i].update_time = GetClock();
    }
    
    void CheckAndDoPause()
    {
        while(m_i32PlayRate == PAUSE_PLAYRATE &&
              !m_SeekInfo.bSeeking &&
              m_Status== E_PlayerStatus_Playing)
            DoDelay(PAUSE_DEALY);
    }
    
    void DoDelay(int64_t i64Delay)
    {
        
        int ms  = i64Delay % AV_TIME_BASE;
        int sec = (int)(i64Delay / AV_TIME_BASE);
        timeval   timeout = { sec, ms };
        select(0, NULL, NULL, NULL, &timeout);
    }

    void UnLockQueue()
    {
        m_VideoEncoder.UnLockQueue();
        m_VideoDecoder.UnLockQueue();
    }
    
    void ClearQueue()
    {
        m_VideoEncoder.ClearQueue();
        m_VideoDecoder.ClearQueue();
    }
    
    void UpdateAvgThreadTime(int64_t NewTime,int64_t *aAvgTime)
    {
        if(*aAvgTime == 0)
            *aAvgTime = NewTime;
        
        *aAvgTime += NewTime;
        *aAvgTime /= 2;
    }
    
    void ThreadDelay(int64_t UnitTime, int i32UnitCnt)
    {
        if(UnitTime > 0)
        {
            int64_t delaytime = (UnitTime * i32UnitCnt) ;
            
            if(delaytime>0)
            {
                int64_t timeBefore = GetClock();
                DoDelay((int)delaytime);
                int64_t timeEsc = GetClock() - timeBefore;
                delaytime-=timeEsc;
            }
        }
    }
    
    E_PlayerStatus      m_Status;

    pthread_t           m_initThread;
    pthread_t           m_inputThread;
    pthread_t           m_decodeThread;
    pthread_t           m_displayThread;
    pthread_t           m_writeThread;
    
    //Video
    AVFormatContext 	*m_formatCtx;
    AVFormatContext 	*m_AudioformatCtx;
    int 				m_videoStream;
    AVFrame             *m_covertFrame;
    struct SwsContext   *m_sws_ctx;
    int 				m_audioStream;

    AVCodecID           m_EncodeID;
    S_ClockInfo         m_Clock[2];
    
    S_ThreadTimeInfo    m_ThreadTimeInfo;
    S_SeekInfo          m_SeekInfo;
    
    int64_t             m_FrameCnt;
    int64_t             m_InputCnt;
    
    int64_t             m_AudioPlayedSample;
    
    int64_t             m_BufferMSTime;
    int                 m_i32PlayRate;
    float               m_fVideofps;

    pthread_mutex_t     m_Initlock;
    pthread_mutex_t     m_Startlock;
    pthread_mutex_t     m_Ratelock;
    pthread_mutex_t     m_Contextlock;
    
    C_PictureEncoder    m_SnapshotEncoder;
    C_VideoEncoder      m_VideoEncoder;
    C_AudioDecoder      m_AudioDecoder;
    C_VideoDecoder      m_VideoDecoder;
    C_FrameExtractor    m_FrameExtractor;
    
    bool                m_bSaveSnapshot;
    bool                m_bSaveVideo;
    bool                m_bLowLatency;
    bool                m_bIsStreaming;
    bool                m_bRepeat;
    bool                m_bEncodeUsingLocalTime;
    bool                m_bForceToTransCode;
    
    char                m_snapShotPath[1024];
    char                m_VideoPath[1024];
    char                m_MediaPath[1024];
    char                m_UserOptions[1024];
    char                m_TransCodeOptions[1024];
    char                m_DecodeCodeOptions[1024];
    
    int64_t             m_lastDecodeSize;
    
    I_FFmpegAgnet       *m_FFmpegAgnet;
};


#endif //FFMPEGTEST_C_FFMPEGPLAYER_H
