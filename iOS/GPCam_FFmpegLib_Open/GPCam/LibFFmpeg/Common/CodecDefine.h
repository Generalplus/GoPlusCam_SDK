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

#define VERSION_STR             "V1.0.0.4"
#define MAX_FRAME_RATE          120
#define DEFAULT_FRAME_RATE      24
#define DEFAULT_DELAY           (0.005 * AV_TIME_BASE)      // 5ms
#define PAUSE_DEALY             (AV_TIME_BASE * 0.1)        // 100 ms
#define SYNC_DURATION           (AV_TIME_BASE * 0.1)        // 100 ms
#define SHOW_VERSION  2
#define SHOW_CONFIG   4
#define SHOW_COPYRIGHT 8

#include "Defines.h"
#include "QueueManager.h"
//----------------------------------------------------------------

#define ImpGetter(TypeD , FuncName , Variable) TypeD Get##FuncName()  { return Variable; }
#define ImpSetter(TypeD , FuncName , Variable) void Set##FuncName(TypeD inputD)  { Variable = inputD; }
#define ImpGetterSetter(TypeD , FuncName) \
public: ImpGetter(TypeD , FuncName , m_##FuncName)\
ImpSetter(TypeD , FuncName , m_##FuncName)\
private: \
TypeD m_##FuncName; \
public:

//----------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

void SetAVDictionary(char *pOptions , AVDictionary *opts);
int64_t GetClock();
void DoDelay(int64_t i64Delay);
void FrameCopy(AVFrame *pDstFrame,AVFrame *pSrcFrame);
void InsertDebugMessage(AVPacket *pPacket,
                        AVFrame *pFrame ,
                        int i32VideoW,
                        int i32VideoH);
void print_all_libs_info(int flags, int level);
#ifdef __cplusplus
}
#endif

//----------------------------------------------------------------------

#endif /* CodecDefine_h */
