//
//  CodecDefine.h
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2016/12/21.
//  Copyright © 2016年 generalplus_sa1. All rights reserved.
//

#ifndef CodecDefine_h
#define CodecDefine_h

//----------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
    
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/pixfmt.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
#include <libavutil/error.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
    
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
    
#ifdef __cplusplus
}
#endif

#include "Defines.h"
#include "QueueManager.h"
#include "font.h"

enum E_StreamType
{
    E_StreamType_Video = 0,
    E_StreamType_Audio
    
};

enum E_Contextype
{
    E_ContextType_Video = 0,
    E_ContextType_Audio
    
};

//----------------------------------------------------------------
class I_ContextManager
{
public:
    
    virtual AVFormatContext *GetFormat() = 0;
    virtual AVStream        *GetStream(E_StreamType eType) = 0;
    virtual int             GetVideoStreamID() = 0;
    virtual int             GetAudioStreamID() = 0;
};
//----------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
    
void SetAVDictionary(char *pOptions , AVDictionary *opts);
int64_t GetClock();
void InsertDebugMessage(AVPacket *pPacket,
                        AVFrame *pFrame ,
                        int i32VideoW,
                        int i32VideoH);
#ifdef __cplusplus
}
#endif

//----------------------------------------------------------------------

#endif /* CodecDefine_h */
