//
//  PlayerInfo.h
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2017/10/3.
//  Copyright © 2017年 generalplus_sa1. All rights reserved.
//

#ifndef PlayerInfo_h
#define PlayerInfo_h

#include "CodecDefine.h"
//---------------------------------------------------------------------
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
    int64_t    packet_pts;
    
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

typedef struct TrackInfo
{
    bool        bValid;
    int         i32StreamID;
    int64_t     start_time;
    int64_t     duration;
    AVRational  time_base;
    AVRational  r_frame_rate;
    AVCodecID   CodecID;
    
    S_ClockInfo ClockInfo;
    
}S_TrackInfo;

#define PAUSE_PLAYRATE          0
#define DEFAULT_PLAYRATE        1
#define SPEEDUP_PLAYRATE        2

typedef enum
{
    E_TrackVideo = 0,
    E_TrackAudio
    
}E_Track;
//---------------------------------------------------------------------
class C_PlayerInfo
{
public:
    C_PlayerInfo():m_IsStreaming(true),m_bLowLatency(false),m_Repeat(true),m_BufferingTime(0){}
    ~C_PlayerInfo(){}
    
    void Reset()
    {
        memset(&m_SeekInfo,0x00,sizeof(m_SeekInfo));
        memset(&m_ThreadTimeInfo,0x00,sizeof(m_ThreadTimeInfo));
        
        m_Height = 0;
        m_Witdh = 0;
        m_FrameCnt = 0;
        m_InputCnt = 0;
        m_Videofps = 0;
        m_PlayRate = DEFAULT_PLAYRATE;
        m_Status = E_PlayerStatus_Stoped;
        m_AudioPlayedSample = 0;
        m_AudioQueueIndex = 0;
        m_AudioStartAlign = 0;
        m_DecodeSize = 0;
        m_DisplaySize = 0;

        memset(&m_Tracks,0x00,sizeof(m_Tracks));
    }
    
    void resetClock()
    {
        for(int i=0;i<2;i++)
            memset(&m_Tracks[i].ClockInfo,0x00,sizeof(S_ClockInfo));
    }
    
    void reflashUpdateTime()
    {
        for(int i=0;i<2;i++)
        {
            m_Tracks[i].ClockInfo.update_time = GetClock();
        }
    }
    
    ImpGetterSetter(int , Height)
    ImpGetterSetter(int , Witdh)
    ImpGetterSetter(int64_t , FrameCnt)
    ImpGetterSetter(int64_t , InputCnt)
    ImpGetterSetter(int64_t , Duration)
    ImpGetterSetter(int64_t , BufferingTime)
    ImpGetterSetter(float , Videofps)
    ImpGetterSetter(bool , IsStreaming)
    ImpGetterSetter(bool , bLowLatency)
    ImpGetterSetter(bool , Repeat)
    ImpGetterSetter(int , PlayRate)
    ImpGetterSetter(E_PlayerStatus , Status)
    ImpGetterSetter(int64_t , AudioPlayedSample)
    ImpGetterSetter(int64_t , AudioQueueIndex)
    ImpGetterSetter(int64_t , AudioStartAlign)
    ImpGetterSetter(int , DecodeSize)
    ImpGetterSetter(int , DisplaySize)
    
    S_TrackInfo* GetTrackInfo(E_Track eTrack) { return &m_Tracks[eTrack];}
    S_ThreadTimeInfo* GetThreadTimeInfo() { return &m_ThreadTimeInfo;}
    S_SeekInfo* GetSeekInfo() { return &m_SeekInfo;}
private:
    
    S_TrackInfo         m_Tracks[2];
    S_ThreadTimeInfo    m_ThreadTimeInfo;
    S_SeekInfo          m_SeekInfo;
    
};


#endif /* PlayerInfo_h */
