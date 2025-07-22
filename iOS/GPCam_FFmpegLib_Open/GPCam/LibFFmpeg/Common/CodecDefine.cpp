//
//  CodecDefine.c
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2016/12/21.
//  Copyright © 2016年 generalplus_sa1. All rights reserved.
//

#include "CodecDefine.h"
#include "font.h"
#include "libavutil/intreadwrite.h"

#define JPEG_END_TAG  0xFFD9
#define GPDEBUG_TAG   "GDBG"
#define GPDEBUG_SIZE  12
#define FONT_SCALE    3
#define TOTAL_DIGIT_LEN     (21 + 5)

#define START_OFFSET    256
#define END_OFFSET      (GPDEBUG_SIZE + 2)

static int g_Scale = FONT_SCALE;

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

void print_all_libs_info(int flags, int level)
{
    PRINT_LIB_INFO(avutil,     AVUTIL,     flags, level);
    PRINT_LIB_INFO(avcodec,    AVCODEC,    flags, level);
    PRINT_LIB_INFO(avformat,   AVFORMAT,   flags, level);
    PRINT_LIB_INFO(swscale,    SWSCALE,    flags, level);
    PRINT_LIB_INFO(swresample, SWRESAMPLE, flags, level);
}
//----------------------------------------------------------------
void SetAVDictionary(char *pOptions , AVDictionary *opts)
{
    char *pOption,*pToken;
    char Key[256],Value[256];
    char temp[1024];
    strcpy(temp,pOptions);

    pOption = strtok (temp,";");
    while (pOption != NULL)
    {
        pToken= strchr(pOption,'=');
        if(pToken == NULL)
            break;

        strncpy(Key,pOption,pToken-pOption);
        Key[pToken-pOption] = 0;
        pToken++;

        strcpy(Value,pToken);
        av_dict_set(&opts, Key, Value , 0);
        pOption = strtok (NULL, ";");
    }
}
//----------------------------------------------------------------------
int64_t GetClock()
{
    return av_gettime_relative();
}
//----------------------------------------------------------------------
void DoDelay(int64_t i64Delay)
{
    int ms  = i64Delay % AV_TIME_BASE;
    int sec = (int)(i64Delay / AV_TIME_BASE);
    timeval   timeout = { sec, ms };
    select(0, NULL, NULL, NULL, &timeout);
}
//----------------------------------------------------------------------
void FrameCopy(AVFrame *pDstFrame,AVFrame *pSrcFrame)
{
    if(pSrcFrame == NULL)
        return ;

    pDstFrame->format = pSrcFrame->format;
    pDstFrame->width  = pSrcFrame->width;
    pDstFrame->height = pSrcFrame->height;

    int ret = av_image_alloc(pDstFrame->data, pDstFrame->linesize, pSrcFrame->width, pSrcFrame->height,(AVPixelFormat)pSrcFrame->format, 1);
    av_frame_copy(pDstFrame, pSrcFrame);
    av_frame_copy_props(pDstFrame,pSrcFrame);
}
//----------------------------------------------------------------------
const INT8U Font_Color[3] =
{
    //225 , 0  , 148  //Yellow
    149 , 43 , 21  //Green
};
//----------------------------------------------------------------------
const INT8U *g_pNumberTable[10] =
{
    acFontHZArial01100030,
    acFontHZArial01100031,
    acFontHZArial01100032,
    acFontHZArial01100033,
    acFontHZArial01100034,
    acFontHZArial01100035,
    acFontHZArial01100036,
    acFontHZArial01100037,
    acFontHZArial01100038,
    acFontHZArial01100039
};
//----------------------------------------------------------------------
void MarkPoint(AVFrame *pFrame,int i32X,int i32Y)
{
    int Pos = i32Y * pFrame->linesize[0] + i32X;
    int PosUV = Pos;

    if(pFrame->format == AV_PIX_FMT_YUV422P || pFrame->format == AV_PIX_FMT_YUVJ422P )
    {
        PosUV = PosUV /2;
    }
    else if(pFrame->format == AV_PIX_FMT_YUV444P || pFrame->format == AV_PIX_FMT_YUVJ444P )
    {

    }
    else if(pFrame->format == AV_PIX_FMT_YUV420P || pFrame->format == AV_PIX_FMT_YUVJ420P )
    {
        PosUV = (i32Y/4 * pFrame->linesize[0]) + (i32X/2);
    }


    //Y
    pFrame->data[0][Pos] = Font_Color[0];
    //U
    pFrame->data[1][PosUV] = Font_Color[1];
    //V
    pFrame->data[2][PosUV] = Font_Color[2];

}
//----------------------------------------------------------------------
void WriteFont(AVFrame *pFrame,int i32Position,int i32VideoH,const INT8U *pCharArry)
{
    for(int y=0;y<FONT_HEIGHT;y++)
    {
        for(int x=0;x<FONT_WIDTH;x++)
        {
            if((pCharArry[y] >> (7-x)) & 0x01)
            {
                for(int i=0;i<g_Scale;i++)
                    for(int j=0;j<g_Scale;j++)
                    {
                        MarkPoint(pFrame,(g_Scale*x)+i32Position+i,(g_Scale*y) + j+ (i32VideoH - (2*g_Scale*FONT_HEIGHT-1)));
                    }
            }
        }
    }
}
//----------------------------------------------------------------------
int WriteNumber(AVFrame *pFrame,int i32PositionStart,int i32VideoH, int64_t i64Number,int i32Digit)
{
    int i32Position = i32PositionStart;
    INT8U DigitArray[10] = {0};
    for(int i=i32Digit-1;i>=0;i--)
    {
        DigitArray[i] = i64Number % 10;
        i64Number /=10;
    }
    for(int i=0;i<i32Digit;i++)
    {
        const INT8U *pCharArry = g_pNumberTable[DigitArray[i]];
        WriteFont(pFrame,i32Position,i32VideoH,pCharArry);
        i32Position+=g_Scale*FONT_WIDTH;
    }

    return i32Position;
}
//----------------------------------------------------------------------
void InsertMessage(AVFrame *pFrame ,int i32VideoH , int64_t i64Number,INT8U byU,INT8U byUV,INT16U u16Index)
{
    int i32Position = g_Scale*FONT_WIDTH;

    i32Position = WriteNumber(pFrame,i32Position,i32VideoH,i64Number,10);
    WriteFont(pFrame,i32Position,i32VideoH,acFontHZArialDot);
    i32Position+=g_Scale*FONT_WIDTH;

    i32Position = WriteNumber(pFrame,i32Position,i32VideoH,byU,3);
    WriteFont(pFrame,i32Position,i32VideoH,acFontHZArialDot);
    i32Position+=g_Scale*FONT_WIDTH;

    i32Position = WriteNumber(pFrame,i32Position,i32VideoH,byUV,3);
    WriteFont(pFrame,i32Position,i32VideoH,acFontHZArialDot);
    i32Position+=g_Scale*FONT_WIDTH;

    i32Position = WriteNumber(pFrame,i32Position,i32VideoH,u16Index,5);
    WriteFont(pFrame,i32Position,i32VideoH,acFontHZArialDot);
    i32Position+=g_Scale*FONT_WIDTH;

}
//----------------------------------------------------------------------
void InsertDebugMessage(AVPacket *pPacket,
                        AVFrame *pFrame ,
                        int i32VideoW,
                        int i32VideoH)
{
    int i32End = pPacket->size - END_OFFSET;
    if(i32End<0)
        return;

    int i32Start = pPacket->size - START_OFFSET;
    if(i32Start<0)
        return;

    const char* pDebugMsg = GPDEBUG_TAG;

    g_Scale = i32VideoW / (TOTAL_DIGIT_LEN * FONT_WIDTH);
    if(g_Scale<=0)
        g_Scale = 1;

    for(int i=i32Start;i<=i32End;i++)
    {
        uint8_t *buf = &pPacket->data[i];
        INT16U u16Tag = AV_RB16(buf);
        if(u16Tag == JPEG_END_TAG)
        {
            for(int j=i+2;j<=i32End;j++)
            {
                if(pPacket->data[j]   == pDebugMsg[0] && pPacket->data[j+1] == pDebugMsg[1] &&
                   pPacket->data[j+2] == pDebugMsg[2] && pPacket->data[j+3] == pDebugMsg[3]
                   )
                {
                    uint8_t *buf = &pPacket->data[j];
                    int64_t i64Number = AV_RL32(buf+4);
                    INT8U byU  = AV_RB8(buf+8);
                    INT8U byUV = AV_RB8(buf+9);
                    INT16U u16Index = AV_RL16(buf+10);

                    InsertMessage(pFrame,i32VideoH,i64Number,byU,byUV,u16Index);
                    break;
                }
            }

            break;
        }
    }

}
//----------------------------------------------------------------------
