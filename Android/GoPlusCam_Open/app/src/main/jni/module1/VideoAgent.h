

#ifndef EGLAGENT_H
#define EGLAGENT_H

#include "Defines.h"
#include "GLRenderer.h"
#include "FFMpegPlayerCore.h"
#include "AudioPlayer.h"

#define FFMPEG_STATUS_PLAYING                   0
#define FFMPEG_STATUS_STOPPED                   1
#define FFMPEG_STATUS_SAVESNAPSHOTCOMPLETE      2
#define FFMPEG_STATUS_SAVEVIDEOCOMPLETE         3
#define FFMPEG_STATUS_BUFFERING                 4


extern int ffmpegWrapper_StatusChange(int i32Status);

class C_VideoAgent : public I_FFmpegAgnet ,
                     public I_GLViewAgent ,
                     public I_FFmpegAudioAgnet
{

public:
    C_VideoAgent();
    virtual ~C_VideoAgent();

    bool init();
    void SetResolution(int width, int height,int format,int linesizes[4]);
    void SetViewSize(int width, int height);
    void DrawFrame();

    //FFMpegPlayerCore wrapper ---------------------------------------------
    int InitMedia(const char* path,const char* pOptions)
    {
        m_FFMpegPlayer.setAgent(this);
        int i32Ret = m_FFMpegPlayer.InitMedia(path,pOptions);

        return i32Ret;
    }

    const char* GetMediaInfo(const char* path)
    {
        return m_FFMpegPlayer.GetMediaInfo(path);
    }

    int SetTransCodeOptions(const char *options)
    {
        return m_FFMpegPlayer.SetTransCodeOptions(options);
    }

    int SetDecodeOptions(const char *options)
    {
        return m_FFMpegPlayer.SetDecodeOptions(options);
    }

    void PlayMedia()
    {
        if(!m_bInited)
            return;

        m_FFMpegPlayer.PlayMedia();
        m_AudioPlayer.setAgent(this);

        AVCodecContext *pAudioContext = m_FFMpegPlayer.GetCodecContext(E_ContextType_Audio);
        if(pAudioContext)
             m_AudioPlayer.Play();

        ffmpegWrapper_StatusChange(FFMPEG_STATUS_PLAYING);
    }

    void Stop()
    {
        m_bInited = false;
        m_FFMpegPlayer.Stop();
        m_AudioPlayer.Stop();
    }

    int     Pause()
    {
        m_AudioPlayer.Pause();
        m_FFMpegPlayer.Pause();

        return FFMPEGPLAYER_NOERROR;
    }

    int     Resume()
    {
        m_AudioPlayer.Resume();
        m_FFMpegPlayer.Resume();

        return FFMPEGPLAYER_NOERROR;
    }

    int Seek(int64_t position)
    {
        m_FFMpegPlayer.Seek(position);

        return FFMPEGPLAYER_NOERROR;
    }

    int64_t GetDuration()
    {
        return m_FFMpegPlayer.GetDuration();
    }

    int64_t GetPosition()
    {
        return m_FFMpegPlayer.GetPosition();
    };

    int GetWidth()                              { return m_FFMpegPlayer.GetWidth(); }
    int GetHeight()                             { return m_FFMpegPlayer.GetHeight(); }
    int GetStatus()                             { return m_FFMpegPlayer.GetStatus(); }
    int GetCodeID()                             { return m_FFMpegPlayer.GetCodecID(); }
    int SaveSnapshot(const char *path)          { return m_FFMpegPlayer.SaveSnapshot(path); }
    int SaveVideo(const char *path)             { return m_FFMpegPlayer.SaveVideo(path);    }
    int StopSaveVideo()                         { return m_FFMpegPlayer.StopSaveVideo();    }

    void SetLowLatency(bool bEnable)            { m_FFMpegPlayer.SetLowLatency(bEnable);}
    void SetForceToTranscode(bool bEnable)      { m_FFMpegPlayer.ForceToTranscode(bEnable);}
    void SetEncodeByLocalTime(bool bEnable)     { m_FFMpegPlayer.SetEncodeByLocalTime(bEnable);}
    void SetRepeat(bool bRepeat)                { m_FFMpegPlayer.setRepeat(bRepeat);}
    void SetScaleMode(int i32Mode)              { _render.SetScaleMode(i32Mode);}
    void SetZoomInRatio(float fRatio)           { _render.SetZoomInRatio(fRatio);}
    void SetBufferingTime(int64_t lTime)        { m_FFMpegPlayer.SetBufferingTime(lTime);}

    int64_t GetRevSizeCnt()                     {return m_FFMpegPlayer.GetRevSizeCnt();}
    int64_t GetFrameCnt()                       {return m_FFMpegPlayer.GetFrameCnt();}
    AVFrame *DupDecodedFrame()                  {return m_FFMpegPlayer.DupDecodedFrame();}

    void EnableDebugMessage(bool bEnable)       {m_FFMpegPlayer.EnableDebugMessage(bEnable); }

    int ExtractFrame(const char* VideoPath,
                     const char* SavePath,
                     int64_t frameIdx)
    {
        return m_FFMpegPlayer.ExtractFrame(VideoPath,SavePath,frameIdx);
    }


    //I_FFmpegAgnet ---------------------------------------------

    //Init complete
    virtual int InitComplete(int i32Result)
    {
        if(i32Result != FFMPEGPLAYER_NOERROR)
        {
            m_bInited = false;
            return ffmpegWrapper_StatusChange(FFMPEG_STATUS_STOPPED);
        }

        pthread_mutex_lock(&m_NewDatalock);

        int LinSize[4]  = { m_FFMpegPlayer.GetWidth(), m_FFMpegPlayer.GetWidth() / 2, m_FFMpegPlayer.GetWidth() / 2 ,0};
        _i32Format = AV_PIX_FMT_YUV420P;
        SetResolution(m_FFMpegPlayer.GetWidth(),m_FFMpegPlayer.GetHeight(),_i32Format,LinSize);
        pthread_mutex_unlock(&m_NewDatalock);

        AVCodecContext *pAudioContext = m_FFMpegPlayer.GetCodecContext(E_ContextType_Audio);
        if(pAudioContext)
            m_AudioPlayer.Init(pAudioContext->sample_rate , (int)pAudioContext->channels);

        m_bInited = true;

        PlayMedia();

        return 0;
    }

    //Core to GLView
    virtual int PlatformDisplay(uint8_t *pData[], int width, int height, int format , int lineSizes[4]);

    //Core to Audio player
    virtual int PlatformAudio(uint8_t **pData, int i32SampleCnt, int64_t i64SampleIndex);
    virtual int SetAudioRate(int i32Rate);
    virtual int ClearAudioQueue();
    virtual int DoAudioRampUp();
    virtual int DoAudioRampDown();
    virtual int SaveSanpshotComplete()  { ffmpegWrapper_StatusChange(FFMPEG_STATUS_SAVESNAPSHOTCOMPLETE); return 0;}
    virtual int SaveVideoComplete()     { ffmpegWrapper_StatusChange(FFMPEG_STATUS_SAVEVIDEOCOMPLETE); return 0;}
    virtual int StartBuffering()        { ffmpegWrapper_StatusChange(FFMPEG_STATUS_BUFFERING); return 0;}
    virtual int ContinuePlaying()       { ffmpegWrapper_StatusChange(FFMPEG_STATUS_PLAYING); return 0;}


    //I_FFmpegAudioAgnet ---------------------------------------------
    //Audio player to Core
    virtual int PlayedSample(int64_t i64SampleIndex);

    //I_GLViewAgent ---------------------------------------------
    virtual void LoadGLBuffer();

private:

    void ClearPicure()
    {
        if(m_pData[0]==NULL)
            return ;

        int i32Div = 4;
        if(_i32Format == AV_PIX_FMT_YUV422P || _i32Format == AV_PIX_FMT_YUVJ422P )
            i32Div = 2;
        if(_i32Format == AV_PIX_FMT_YUV444P || _i32Format == AV_PIX_FMT_YUVJ444P )
            i32Div = 1;

        memset(m_pData[0],0x00,_width*_height);
        memset(m_pData[1],0x80,_width*_height/i32Div);
        memset(m_pData[2],0x80,_width*_height/i32Div);
    }

    C_GLRenderer     _render;
    C_AudioPlayer    m_AudioPlayer;
    C_FFMpegPlayer   m_FFMpegPlayer;
    pthread_mutex_t  m_NewDatalock;

    int             _Viewwidth;
    int             _Viewheight;
    int             _width;
    int             _height;
    int             _i32Format;
    uint8_t         *m_pData[8];
    bool            m_bInited;
};

#endif // EGLAGENT_H
