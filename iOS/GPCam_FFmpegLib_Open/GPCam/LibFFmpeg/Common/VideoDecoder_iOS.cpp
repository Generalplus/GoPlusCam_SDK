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
int C_VideoDecoder::OpenCodec(AVCodecContext *pCodeContext,AVCodecID CodeID,char* ptszOption)
{
    int i32Ret = C_Decoder::OpenCodec(pCodeContext, CodeID, ptszOption);

    if(m_sws_ctx)
    {
        sws_freeContext(m_sws_ctx);
        m_sws_ctx = NULL;
    }

    return i32Ret;
}
//----------------------------------------------------------------------
void C_VideoDecoder::didDecompress( void *decompressionOutputRefCon, void *sourceFrameRefCon, OSStatus status, VTDecodeInfoFlags infoFlags, CVImageBufferRef imageBuffer, CMTime presentationTimeStamp, CMTime presentationDuration )
{
    C_VideoDecoder *pDecoder = (C_VideoDecoder*)decompressionOutputRefCon;

    if (pDecoder->m_frame) {
        CVPixelBufferRelease(pDecoder->m_frame);
        pDecoder->m_frame = NULL;
    }

    if (status == noErr && imageBuffer) {

        pDecoder->m_frame = CVPixelBufferRetain(imageBuffer);

    }
}
//----------------------------------------------------------------------
static CFDictionaryRef videotoolbox_buffer_attributes_create(int width,
                                                             int height,
                                                             OSType pix_fmt)
{
    CFMutableDictionaryRef buffer_attributes;
    CFMutableDictionaryRef io_surface_properties;
    CFNumberRef cv_pix_fmt;
    CFNumberRef w;
    CFNumberRef h;

    w = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &width);
    h = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &height);
    cv_pix_fmt = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &pix_fmt);

    buffer_attributes = CFDictionaryCreateMutable(kCFAllocatorDefault,
                                                  4,
                                                  &kCFTypeDictionaryKeyCallBacks,
                                                  &kCFTypeDictionaryValueCallBacks);
    io_surface_properties = CFDictionaryCreateMutable(kCFAllocatorDefault,
                                                      0,
                                                      &kCFTypeDictionaryKeyCallBacks,
                                                      &kCFTypeDictionaryValueCallBacks);

    if (pix_fmt)
        CFDictionarySetValue(buffer_attributes, kCVPixelBufferPixelFormatTypeKey, cv_pix_fmt);
    CFDictionarySetValue(buffer_attributes, kCVPixelBufferIOSurfacePropertiesKey, io_surface_properties);
    CFDictionarySetValue(buffer_attributes, kCVPixelBufferWidthKey, w);
    CFDictionarySetValue(buffer_attributes, kCVPixelBufferHeightKey, h);
#if TARGET_OS_IPHONE
    CFDictionarySetValue(buffer_attributes, kCVPixelBufferOpenGLESCompatibilityKey, kCFBooleanTrue);
#else
    CFDictionarySetValue(buffer_attributes, kCVPixelBufferIOSurfaceOpenGLTextureCompatibilityKey, kCFBooleanTrue);
#endif

    CFRelease(io_surface_properties);
    CFRelease(cv_pix_fmt);
    CFRelease(w);
    CFRelease(h);

    return buffer_attributes;
}
//----------------------------------------------------------------------
static CMSampleBufferRef videotoolbox_sample_buffer_create(CMFormatDescriptionRef fmt_desc,
                                                           void *buffer,
                                                           int size)
{
    OSStatus status;
    CMBlockBufferRef  block_buf;
    CMSampleBufferRef sample_buf;

    block_buf  = NULL;
    sample_buf = NULL;

    status = CMBlockBufferCreateWithMemoryBlock(kCFAllocatorDefault,// structureAllocator
                                                buffer,             // memoryBlock
                                                size,               // blockLength
                                                kCFAllocatorNull,   // blockAllocator
                                                NULL,               // customBlockSource
                                                0,                  // offsetToData
                                                size,               // dataLength
                                                0,                  // flags
                                                &block_buf);

    if (!status) {
        status = CMSampleBufferCreate(kCFAllocatorDefault,  // allocator
                                      block_buf,            // dataBuffer
                                      TRUE,                 // dataReady
                                      0,                    // makeDataReadyCallback
                                      0,                    // makeDataReadyRefcon
                                      fmt_desc,             // formatDescription
                                      1,                    // numSamples
                                      0,                    // numSampleTimingEntries
                                      NULL,                 // sampleTimingArray
                                      0,                    // numSampleSizeEntries
                                      NULL,                 // sampleSizeArray
                                      &sample_buf);
    }

    if (block_buf)
        CFRelease(block_buf);

    return sample_buf;
}
//----------------------------------------------------------------------
bool C_VideoDecoder::HWDecodeMJpeg(AVPacket *pPacket, int *pResult)
{
    *pResult = 0;

    if(m_pCodeContext->width>0 && m_pCodeContext->codec_id == AV_CODEC_ID_MJPEG)
    {
        //int64_t timeBefore = GetClock();

        OSStatus status;
        if(!m_session)
        {
            int videoWidth = m_pCodeContext->width;
            int videoHeight = m_pCodeContext->height;

            CMVideoFormatDescriptionCreate(kCFAllocatorDefault, kCMVideoCodecType_JPEG, videoWidth, videoHeight, NULL, &m_videoFormatDescr);

            VTDecompressionOutputCallbackRecord callback;
            callback.decompressionOutputCallback = didDecompress;
            callback.decompressionOutputRefCon = this;


            CFDictionaryRef buf_attr = videotoolbox_buffer_attributes_create(videoWidth,
                                                                             videoHeight,
                                                                             kCVPixelFormatType_420YpCbCr8Planar);


            status = VTDecompressionSessionCreate(NULL, m_videoFormatDescr, NULL, buf_attr, &callback, &m_session);
            if (status != noErr){
                DEBUG_PRINT("VTDecompressionSessionCreate failed!\n");
                return false;
            }

        }

        CMSampleBufferRef sampleBuffer = videotoolbox_sample_buffer_create(m_videoFormatDescr,pPacket->data,pPacket->size);

        status = VTDecompressionSessionDecodeFrame(m_session, sampleBuffer, 0, NULL, 0);
        if (status == noErr)
            status = VTDecompressionSessionWaitForAsynchronousFrames(m_session);

        CFRelease(sampleBuffer);

        if(!m_frame)
            return true;

        CVReturn err;
        CVPixelBufferRef pixbuf = m_frame;
        OSType pixel_format = CVPixelBufferGetPixelFormatType(pixbuf);

        uint8_t *data[4] = { 0 };
        int linesize[4] = { 0 };
        int planes, ret, i;

        err = CVPixelBufferLockBaseAddress(pixbuf, kCVPixelBufferLock_ReadOnly);
        if (err != kCVReturnSuccess) {
            av_log(NULL, AV_LOG_ERROR, "Error locking the pixel buffer.\n");
            return true;
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

        if(m_TempFrame->format != AV_PIX_FMT_YUV420P)
        {
            AVFrame *frame = av_frame_alloc();

            frame->format = AV_PIX_FMT_YUV420P;
            frame->width  = m_TempFrame->width;
            frame->height = m_TempFrame->height;

            av_image_alloc(frame->data, frame->linesize, m_TempFrame->width, m_TempFrame->height,AV_PIX_FMT_YUV420P, 1);
            av_frame_copy_props(frame,m_TempFrame);
            frame->format = AV_PIX_FMT_YUV420P;
            frame->pts = pPacket->pts;

            av_frame_free(&m_TempFrame);//Do not free &m_TempFrame->data[0] ,
                                        //that memory block is alloced by avcodec_decode_video2(), and it will be released when calling avcodec_close()

            m_TempFrame = frame;
        }

        av_image_copy(m_TempFrame->data,linesize,(const uint8_t **)data,linesize,(AVPixelFormat)AV_PIX_FMT_YUV420P, m_TempFrame->width , m_TempFrame->height);
        CVPixelBufferUnlockBaseAddress(pixbuf, kCVPixelBufferLock_ReadOnly);

        *pResult = 1;

        return true;

    }
    else
        return false;
}
//----------------------------------------------------------------------
bool C_VideoDecoder::Decode(AVPacket *pPacket)
{
    int            			frameFinished;
    bool bRet = false;

    pthread_mutex_lock(&m_lock);

    //int64_t timeBefore = GetClock();

    if(m_pCodeContext)
    {
#if __LP64__
        //bool bHWDecode = HWDecodeMJpeg(pPacket,&frameFinished);
        //if(!bHWDecode)
#endif
        {
            int i32Ret = avcodec_decode_video2(m_pCodeContext, m_TempFrame, &frameFinished, pPacket);
            if(i32Ret<0)
                ReinitHWAccel();
        }
    }

    //DEBUG_PRINT("Decode time: %lld\n",GetClock() - timeBefore);
    //DEBUG_PRINT("Get Frame: %.10lld PTS: %.10lld\n",GetClock(),pPacket->pts);

    pthread_mutex_unlock(&m_lock);

    if(frameFinished)
    {
       //DEBUG_PRINT("Get Frame: %.10lld PTS: %.10lld\n",GetClock(),pPacket->pts);

        if(m_hwaccel)
        {
            videotoolbox_retrieve_data(m_pCodeContext);
            return true;
        }

        if(m_bDebugMessage)
            InsertDebugMessage(pPacket,m_TempFrame,m_pCodeContext->width,m_pCodeContext->height);

        C_Event event(E_EventType_Decode_VideoFrame,(void*)m_TempFrame);
        C_EventManager::GetEvnetManager()->ProcessEvent(event);
        SaveDecodedFrame(m_TempFrame);

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

    if(bClosed)
    {
        pthread_mutex_unlock(&m_lock);
        return -1;
    }

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

    pthread_mutex_unlock(&m_lock);

    CVPixelBufferUnlockBaseAddress(pixbuf, kCVPixelBufferLock_ReadOnly);
    if (ret < 0)
        return ret;

    SaveDecodedFrame(frame);
    C_Event event(E_EventType_Decode_VideoFrame,(void*)frame);
    C_EventManager::GetEvnetManager()->ProcessEvent(event);

    av_freep(&frame->data[0]);
    av_frame_unref(frame);
    av_frame_free(&frame);

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
    if (m_session) {
        VTDecompressionSessionInvalidate(m_session);
        CFRelease(m_session);
        m_session = NULL;
    }

    if (m_frame)
    {
        CVPixelBufferRelease(m_frame);
        m_frame = NULL;

        //release the memory block alloc at HWDecodeMJpeg
        av_freep(&m_TempFrame->data[0]);
        av_frame_unref(m_TempFrame);
    }

    C_Decoder::Close();

}
//----------------------------------------------------------------------
void C_VideoDecoder::SetupHWAccel()
{
    m_hwaccel = NULL;
#if TARGET_OS_IPHONE
    if (kCFCoreFoundationVersionNumber >= kCFCoreFoundationVersionNumber_iOS_8_0)
    {
        AVHWAccel *hwaccel=NULL;
        while((hwaccel= av_hwaccel_next(hwaccel)))
        {
            if(hwaccel->id == m_pCodeContext->codec_id)
            {
                m_hwaccel = hwaccel;
                break;
            }
        }

        m_pCodeContext->get_format = get_format;
    }
#endif
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
