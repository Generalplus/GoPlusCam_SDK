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

#include "EventManager.h"
#include "CustomInputThread.h"
#include "InputThread.h"
#include "DecodeThread.h"
#include "AudioDecodeThread.h"
#include "DisplayThread.h"
#include "EncodeThread.h"
#include "FrameExtractor.h"

class C_ThreadQueue : public T_Queue<C_ThreadBase>
{
public:
    C_ThreadQueue(){}
    ~C_ThreadQueue(){}

    virtual void FreeObjectContent(C_ThreadBase* pObject)
    {

    }
};


//----------------------------------------------------------------------
class C_FFMpegPlayer
{
public:
    C_FFMpegPlayer();

    ~C_FFMpegPlayer();

    int InitMedia(const char* path,  const char* pOptions = NULL);
    int InitCustomProtocol(const char* VideoCode, const char* AudioCode, const char* pOptions = NULL);
    const char* GetMediaInfo(const char* path);
    int PushCustomPacket(const unsigned char* pbyData,int i32Size,int i32Type,int64_t lTimeStamp = 0);

    int PlayMedia();

    int Stop();

    int Pause()
    {
        return m_pInputThread->Pause();
    }

    int Resume()
    {
        return m_pInputThread->Resume();
    }

    int ForceToTranscode(bool bForce)
    {
        m_EncodeThread.SetForceToTranscode(bForce);

        return FFMPEGPLAYER_NOERROR;
    }

    int SaveSnapshot(const char *path)
    {
        return m_DecodeThread.SaveSnapshot(path);
    }

    int SaveVideo(const char *path)
    {
        StopSaveVideo();

        AVCodecContext *pVideoCodecCtx = m_pInputThread->GetCodecContext(AVMEDIA_TYPE_VIDEO);
        if(pVideoCodecCtx == NULL || pVideoCodecCtx->width == 0 || pVideoCodecCtx->height == 0)
            return FFMPEGPLAYER_SAVEVIDEOFAILED;

        m_EncodeThread.Init(m_pInputThread->GetCodecContext(AVMEDIA_TYPE_VIDEO),
                            m_pInputThread->GetCodecContext(AVMEDIA_TYPE_AUDIO));
        m_EncodeThread.SetPath(path, m_pInputThread->GetPath());
        m_EncodeThread.Start();
        return FFMPEGPLAYER_NOERROR;
    }

    int StopSaveVideo()
    {
        m_EncodeThread.SetStop();
        return m_EncodeThread.GetContainerType();
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

    int64_t GetPosition()
    {
        if( m_PlayInfo.GetSeekInfo()->bAssignSeek ||  m_PlayInfo.GetSeekInfo()->bSeeking)
            return m_PlayInfo.GetSeekInfo()->AssignSeekPosition;

        return m_PlayInfo.GetTrackInfo(E_TrackVideo)->ClockInfo.frame_pts;
    }

    int GetCodecID()
    {
        return m_PlayInfo.GetTrackInfo(E_TrackVideo)->CodecID;
    }

    void Seek(int64_t position)
    {
        m_pInputThread->Seek(position);
    }

    int GetWidth()                                  { return m_PlayInfo.GetWitdh();}
    int GetHeight()                                 { return m_PlayInfo.GetHeight();}
    int GetStatus()                                 { return m_PlayInfo.GetStatus();}
    int64_t GetRevSizeCnt()                         { return m_PlayInfo.GetInputCnt();}
    int64_t GetFrameCnt()                           { return m_PlayInfo.GetFrameCnt();}
    int64_t GetDuration()                           { return m_PlayInfo.GetDuration();}
    void SetLowLatency(bool bEnable)                { m_PlayInfo.SetbLowLatency(bEnable);}
    void SetEncodeByLocalTime(bool bEnable)         { m_EncodeThread.SetEncodeUsingLocalTime(bEnable);}
    void SetBufferingTime(int64_t msTime)           { m_PlayInfo.SetBufferingTime(msTime);}
    void setAgent(I_FFmpegAgnet *pAgent)            { m_FFmpegAgnet = pAgent;}
    void setRepeat(bool bRepeat)                    { m_PlayInfo.SetRepeat(bRepeat); }
    AVFrame* DupDecodedFrame()                      { return m_DecodeThread.DupDecodedFrame();}
    void EnableDebugMessage(bool bEnable)           { m_DecodeThread.EnableDebugMessage(bEnable); }


    int ExtractFrame(const char* VideoPath,
                     const char* SavePath,
                     int64_t frameIdx)
    {
        return m_FrameExtractor.ExtractFrame(VideoPath,SavePath,frameIdx);
    }

protected:

    void WaitThreadFinish();

    void onInitComplete(C_Event &event);
    void onInitFailed(C_Event &event);
    void onStop(C_Event &event);
    void onSaveSanpshotComplete(C_Event &event);
    void onSaveVideoComplete(C_Event &event);
    void onBufferStart(C_Event &event);
    void onBufferComplete(C_Event &event);
    void onDisplayFrame(C_Event &event);
    void onPlayedSample(C_Event &event);
    void onThreadEnd(C_Event &event);

private:

    C_InputThreadBase   *m_pInputThread;
    C_InputThread       m_InputThread;
    C_CustomInputThread m_CustomInputThread;
    C_DecodeThread      m_DecodeThread;
    C_AudioDecodeThread m_AudioDecodeThread;
    C_DisplayThread     m_DisplayThread;
    C_EncodeThread      m_EncodeThread;

    C_ThreadQueue       m_ThreadQueue;

    C_PlayerInfo        m_PlayInfo;
    C_FrameExtractor    m_FrameExtractor;

    I_FFmpegAgnet       *m_FFmpegAgnet;

    char                m_TransCodeOptions[1024];
    char                m_DecodeCodeOptions[1024];
};


#endif //FFMPEGTEST_C_FFMPEGPLAYER_H
