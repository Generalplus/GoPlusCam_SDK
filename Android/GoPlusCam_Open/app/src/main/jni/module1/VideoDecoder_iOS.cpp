//
//  VideoDecoder.cpp
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2016/12/21.
//  Copyright © 2016年 generalplus_sa1. All rights reserved.
//
#if defined(__APPLE__)
#include "VideoDecoder_iOS.h"
#include "font.h"

#define REINITHWACCEL_TIME      AV_TIME_BASE * 3

//----------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
    
#  include "libavcodec/videotoolbox.h"
    
#ifdef __cplusplus
}
#endif

//----------------------------------------------------------------------
AVPixelFormat get_format(AVCodecContext* context, AVPixelFormat const formats[]) {
    
    for (auto i = 0; formats[i] != AV_PIX_FMT_NONE; ++i)
    {
        const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(formats[i]);
        
        if (!(desc->flags & AV_PIX_FMT_FLAG_HWACCEL))
            break;
        
        
        AVHWAccel *hwaccel=NULL;
        while((hwaccel= av_hwaccel_next(hwaccel)))
        {
            DEBUG_PRINT("name:%s type:%d id:%u pix:%d\n",hwaccel->name,hwaccel->type,hwaccel->id,hwaccel->pix_fmt);
            
            if(hwaccel->id == context->codec_id &&
               hwaccel->pix_fmt == formats[i] &&
               formats[i] == AV_PIX_FMT_VIDEOTOOLBOX)
                
            {
                auto result = av_videotoolbox_default_init(context);
                if (result >=0)
                {
                    return AV_PIX_FMT_VIDEOTOOLBOX;
                }
                else
                    av_videotoolbox_default_free(context);
            }
        }
        
    }
    return formats[0];
}
//----------------------------------------------------------------------
C_VideoDecoder::~C_VideoDecoder()
{
    if(m_sws_ctx)
    {
        sws_freeContext(m_sws_ctx);
        m_sws_ctx = NULL;
    }
    
}
//----------------------------------------------------------------------
int C_VideoDecoder::OpenCodec(AVMediaType StreamType, char* ptszOption)
{
    int i32Ret = C_Decoder::OpenCodec(StreamType, ptszOption);
    
    if(m_sws_ctx)
    {
        sws_freeContext(m_sws_ctx);
        m_sws_ctx = NULL;
    }
    
    return i32Ret;
}
//----------------------------------------------------------------------
bool C_VideoDecoder::Decode(AVPacket *pPacket)
{
    int            			frameFinished;
    bool bRet = false;
    
    pthread_mutex_lock(&m_lock);

    int i32Ret = avcodec_decode_video2(m_pCodeContext, m_TempFrame, &frameFinished, pPacket);
    if(i32Ret<0)
        ReinitHWAccel();
     
    pthread_mutex_unlock(&m_lock);
    
    if(frameFinished)
    {
        if(m_hwaccel)
        {
            videotoolbox_retrieve_data(m_pCodeContext);
            return true;
        }

        AVFrame *pFrame = av_frame_alloc();
        FrameCopy(pFrame,m_TempFrame);
        SaveDecodedFrame(m_TempFrame);
        
        if(m_bDebugMessage)
            InsertDebugMessage(pPacket,pFrame,m_pCodeContext->width,m_pCodeContext->height);
        
        m_DisplayFrameQueue.PushOject(pFrame);

        //DEBUG_PRINT("Get Frame: %.10lld PTS: %.10lld\n",GetClock(),pPacket->pts);
        
        bRet = true;
    }
    
    return bRet;
}
//----------------------------------------------------------------------
int C_VideoDecoder::videotoolbox_retrieve_data(AVCodecContext *pCodecContext)
{
    AVPixelFormat SW_format;
    bool bClosed = false;
    
    pthread_mutex_lock(&m_lock);
    if(pCodecContext)
        SW_format = pCodecContext->sw_pix_fmt;
    else
        bClosed = true;
    pthread_mutex_unlock(&m_lock);
    
    if(bClosed)
        return -1;
    
    AVFrame *frame = av_frame_alloc();
    
    CVPixelBufferRef pixbuf = (CVPixelBufferRef)m_TempFrame->data[3];
    OSType pixel_format = CVPixelBufferGetPixelFormatType(pixbuf);
    CVReturn err;
    uint8_t *data[4] = { 0 };
    int linesize[4] = { 0 };
    int planes, ret, i;
    
    switch (pixel_format) {
        case kCVPixelFormatType_420YpCbCr8Planar: m_TempFrame->format = AV_PIX_FMT_YUV420P; break;
        case kCVPixelFormatType_422YpCbCr8:       m_TempFrame->format = AV_PIX_FMT_UYVY422; break;
        case kCVPixelFormatType_32BGRA:           m_TempFrame->format = AV_PIX_FMT_BGRA; break;
#ifdef kCFCoreFoundationVersionNumber10_7
        case kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange: m_TempFrame->format = AV_PIX_FMT_NV12; break;
#endif
        default:
            return AVERROR(ENOSYS);
    }
    
    err = CVPixelBufferLockBaseAddress(pixbuf, kCVPixelBufferLock_ReadOnly);
    if (err != kCVReturnSuccess) {
        av_log(NULL, AV_LOG_ERROR, "Error locking the pixel buffer.\n");
        return AVERROR_UNKNOWN;
    }
    
    if (CVPixelBufferIsPlanar(pixbuf)) {
        
        planes = (int)CVPixelBufferGetPlaneCount(pixbuf);
        for (i = 0; i < planes; i++) {
            data[i]     = (uint8_t *)CVPixelBufferGetBaseAddressOfPlane(pixbuf, i);
            linesize[i] = (int)CVPixelBufferGetBytesPerRowOfPlane(pixbuf, i);
        }
    } else {
        data[0] = (uint8_t *)CVPixelBufferGetBaseAddress(pixbuf);
        linesize[0] = (int)CVPixelBufferGetBytesPerRow(pixbuf);
    }

    av_image_alloc(frame->data, frame->linesize, m_TempFrame->width, m_TempFrame->height,SW_format, 1);
    av_frame_copy_props(frame,m_TempFrame);
    frame->format = SW_format;
    frame->width  = m_TempFrame->width;
    frame->height = m_TempFrame->height;
    
    if(m_sws_ctx==NULL)
    {
        static int sws_flags =  SWS_FAST_BILINEAR;
        m_sws_ctx = sws_getContext(
                                   frame->width,
                                   frame->height,
                                   (AVPixelFormat)m_TempFrame->format ,
                                   frame->width,
                                   frame->height,
                                   SW_format,
                                   sws_flags, NULL, NULL, NULL);
    }
    
    ret = sws_scale(m_sws_ctx,
                    data,
                    linesize,
                    0,
                    frame->height,
                    frame->data,
                    frame->linesize);
    
    CVPixelBufferUnlockBaseAddress(pixbuf, kCVPixelBufferLock_ReadOnly);
    if (ret < 0)
        return ret;
    
    SaveDecodedFrame(frame);
    m_DisplayFrameQueue.PushOject(frame);
    
    return 0;
}
//----------------------------------------------------------------------
void  C_VideoDecoder::FlushBuffer()
{
    if(!m_hwaccel)
        C_Decoder::FlushBuffer();
}
//----------------------------------------------------------------------
void C_VideoDecoder::Close()
{
    if(m_hwaccel)
    {
        pthread_mutex_lock(&m_lock);
        if(m_pCodeContext)
            av_videotoolbox_default_free(m_pCodeContext);
        pthread_mutex_unlock(&m_lock);
    }
    
     C_Decoder::Close();
}
//----------------------------------------------------------------------
void C_VideoDecoder::InitHWAccelGetFormat()
{
    m_pCodeContext->get_format = get_format;
}
//----------------------------------------------------------------------
void C_VideoDecoder::ReinitHWAccel()
{
    if(m_hwaccel)
    {
        if(GetClock() -  m_PrevReInitTime > REINITHWACCEL_TIME )
        {
            
            DEVICE_PRINT("Reinit HWaccel...\n");
            av_videotoolbox_default_free(m_pCodeContext);
            auto result = av_videotoolbox_default_init(m_pCodeContext);
            if (result < 0)
            {
                DEBUG_PRINT("Failed to reinit HWaccel!!!\n");
            }

            m_PrevReInitTime = GetClock();
        }
        
    }
}

#endif


