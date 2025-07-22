//
//  VideoEncoder.hpp
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2016/12/21.
//  Copyright © 2016年 generalplus_sa1. All rights reserved.
//

#ifndef VideoEncoder_hpp
#define VideoEncoder_hpp

#include "Encoder.h"


//----------------------------------------------------------------
typedef enum
{
    E_EncodeType_Transcode = 0,
    E_EncodeType_RawStream,
    
}E_EncodeType;

typedef enum
{
    E_EncodeContainer_MP4 = 0,
    E_EncodeContainer_AVI,
    
}E_EncodeContainer;

typedef struct EncodeInfo
{
    bool                bIsSetDiff;
    int64_t             AudioStartPts;
    int64_t             VideoStartPts;
    int64_t             Diff;
    int64_t             Last_Video_pts;
    E_EncodeContainer   Container;
    E_EncodeType        EncodeType;
    
}S_EncodeInfo;

typedef struct TimeFixInfo
{
    int64_t         FrameCnt;
    int64_t         prevTime;
    int64_t         prevLeft;
    
}S_TimeFixInfo;
//----------------------------------------------------------------
class C_VideoEncoder : public C_Encoder
{
public:
    
    C_VideoEncoder(C_PlayerInfo *pPlayerInfo):C_Encoder(pPlayerInfo),
    m_prevPacket(NULL),m_bForceToTransCode(false)
    {
        m_EncodeInfo.EncodeType = E_EncodeType_Transcode;
        m_EncodeInfo.Container = E_EncodeContainer_MP4;
    }
    
    ~C_VideoEncoder();
    
    void init(AVCodecContext *pVideoCodecCtx,
              AVCodecContext *pAudioCodecCtx);
    
    void SetEncodeType(E_EncodeType eType) { m_EncodeInfo.EncodeType = eType; }
    E_EncodeType GetEncodeType() { return m_EncodeInfo.EncodeType; }
    E_EncodeContainer GetContainerType() {return m_EncodeInfo.Container; }
    
    
    //Raw stream encode
    bool CreateEncodeRawStream(char *ptszPath,float fFPS,bool bIsRTSP,bool bUsingLocalTime);
    bool writeRawStream();
    bool pushRawPacket(AVPacket *pPacket);
    
    //Transcode encode
    bool CreateEncodeStream(AVCodecID CodecID ,
                            char *ptszPath ,
                            float fFPS ,
                            char *ptszOption);
    
    int writeTranscodeFrame();
    int pushTranscodeFrame(AVFrame* pOutFrame);

    void CloseVideo();
    
    void Stop()
    {
        UnLockQueue();
    }
    
    void UnLockQueue()
    {
        m_ScaledFrameQueue.UnLockQueue();
        m_RawPacketQueue.UnLockQueue();
    }
    
    void ClearQueue()
    {
        m_ScaledFrameQueue.ClearQueue();
        m_RawPacketQueue.ClearQueue();
    }
    
private:
    
    void writeRawVideoPacket(AVPacket *pPacket);
    
    int writePacket(AVPacket* pPacket);
    int writePacketDirectly(AVPacket* pPacket);
    int64_t GetPtsByTimeEsc();
    
    AVCodecID           m_EncodeID;
    AVOutputFormat      *m_outFmt;
    AVFormatContext     *m_outCtx;
    AVStream            *m_outStrm;
    AVStream            *m_outAudioStrm;
    struct SwsContext   *m_outsws_ctx;
    
    AVPacket            *m_prevPacket;
    
    AVCodecContext      *m_pVideoCodecCtx;
    AVCodecContext      *m_pAudioCodecCtx;
    
    S_EncodeInfo        m_EncodeInfo;
    S_TimeFixInfo       m_TimeFixInfo;
    
    float               m_fVideofps;
    bool                m_bIsRTSP;
    bool                m_bUsingLocalTime;
    bool                m_bForceToTransCode;
    
    C_FrameQueue        m_ScaledFrameQueue;
    C_PacketQueue       m_RawPacketQueue;
};


#endif /* VideoEncoder_hpp */
