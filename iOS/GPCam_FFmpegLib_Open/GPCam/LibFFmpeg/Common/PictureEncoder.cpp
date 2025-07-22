//
//  PictureEncoder.cpp
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2016/12/21.
//  Copyright © 2016年 generalplus_sa1. All rights reserved.
//

#include "PictureEncoder.h"

//----------------------------------------------------------------------
C_PictureEncoder::~C_PictureEncoder()
{

}
//----------------------------------------------------------------------
bool C_PictureEncoder::Encode(AVFrame *pInFrame, AVCodecContext *pCodecCtx , const char *ptszPath)
{

    AVCodec *codec;
    int ret,got_output;
    AVPacket pkt;
    DEBUG_PRINT("Save Snapshot\n");

    codec = avcodec_find_encoder( AV_CODEC_ID_MJPEG);
    if (!codec) {
        DEBUG_PRINT("Codec not found\n");
        return false;
    }
    m_pCodeContext = avcodec_alloc_context3(codec);
    if (!m_pCodeContext) {
        DEBUG_PRINT("Could not allocate video codec context\n");
        return false;
    }
    /* put sample parameters */
    m_pCodeContext->bit_rate = pCodecCtx->bit_rate;
    m_pCodeContext->width = pCodecCtx->width;
    m_pCodeContext->height = pCodecCtx->height;
    m_pCodeContext->time_base = m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->time_base;
    if(pCodecCtx->codec_id == AV_CODEC_ID_MJPEG)
        m_pCodeContext->pix_fmt = (AVPixelFormat)pInFrame->format;
    else
        m_pCodeContext->pix_fmt = AV_PIX_FMT_YUVJ420P;

    /* open it */
    if (avcodec_open2(m_pCodeContext, codec, NULL) < 0) {
        DEBUG_PRINT("Could not open codec\n");
        return false;
    }

    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    //conver format
    struct SwsContext   *outsws_ctx;
    outsws_ctx = sws_getContext(
                                m_pCodeContext->width,
                                m_pCodeContext->height,
                                m_pCodeContext->pix_fmt,
                                m_pCodeContext->width,
                                m_pCodeContext->height,
                                m_pCodeContext->pix_fmt,
                                SWS_FAST_BILINEAR, NULL, NULL, NULL);
    //add to queue
    AVFrame *pFrame = av_frame_alloc();
    pFrame->format = m_pCodeContext->pix_fmt;
    pFrame->width  = m_pCodeContext->width;
    pFrame->height = m_pCodeContext->height;

    av_image_alloc(pFrame->data, pFrame->linesize, m_pCodeContext->width, m_pCodeContext->height, m_pCodeContext->pix_fmt, 1);
    ret = sws_scale(outsws_ctx,
                    pInFrame->data,
                    pInFrame->linesize,
                    0,
                    m_pCodeContext->height,
                    pFrame->data,
                    pFrame->linesize);

    ret = avcodec_encode_video2(m_pCodeContext, &pkt, pFrame, &got_output);
    if (ret < 0) {
        DEBUG_PRINT("Error encoding frame\n");
        return false;
    }
    if (got_output) {
        DEBUG_PRINT("Write frame (size=%5d)\n", pkt.size);

        FILE *fp = fopen(ptszPath,"wb");
        if(fp)
        {
            fwrite(pkt.data,1,pkt.size,fp);
            fclose(fp);
        }
        else
        {
            DEBUG_PRINT("Failed to open file: %s\n",ptszPath);
            return false;
        }
    }

    sws_freeContext(outsws_ctx);

    av_freep(&pFrame->data[0]);
    av_frame_unref(pFrame);
    av_frame_free(&pFrame);

    av_free_packet(&pkt);
    avcodec_close(m_pCodeContext);
    av_free(m_pCodeContext);
    m_pCodeContext = NULL;

    return true;
}
//----------------------------------------------------------------------
