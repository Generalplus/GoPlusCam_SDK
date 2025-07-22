//
//  Decoder.cpp
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2016/12/21.
//  Copyright © 2016年 generalplus_sa1. All rights reserved.
//

#include "Decoder.h"

//----------------------------------------------------------------------
C_Decoder::C_Decoder(I_ContextManager *pContextManager):
m_pContextManager(pContextManager),
m_pFFmpegAgent(NULL),m_pCodeContext(NULL),m_TempFrame(NULL),m_hwaccel(NULL),m_DecodedFrame(NULL)
{
    pthread_mutex_init(&m_lock, NULL);
}
//----------------------------------------------------------------------
C_Decoder::~C_Decoder()
{
    Close();
}
//----------------------------------------------------------------------
int C_Decoder::OpenCodec(AVMediaType StreamType,char* ptszOption)
{
    
    AVCodec         *pCodec = NULL;
    AVDictionary    *optionsDict = NULL;
    AVFormatContext *pformatCtx = m_pContextManager->GetFormat();

    int i32StreamID = -1;
    
    if ((i32StreamID = av_find_best_stream(pformatCtx, StreamType, -1, -1,
                                          &pCodec, 0)) < 0) {
        DEBUG_PRINT("av_find_best_stream failed!\n");
        return i32StreamID;
    }
    m_pCodeContext= pformatCtx->streams[i32StreamID]->codec;
    
    m_hwaccel = NULL;
    AVHWAccel *hwaccel=NULL;
    while((hwaccel= av_hwaccel_next(hwaccel)))
    {
        if(hwaccel->id == m_pCodeContext->codec_id)
        {
            m_hwaccel = hwaccel;
            break;
        }
    }

    GetHWDecoder(m_pCodeContext->codec_id,&pCodec);

    if(!pCodec)
        pCodec = avcodec_find_decoder(m_pCodeContext->codec_id);

    if (pCodec == NULL) {
        DEBUG_PRINT("Unsupported codec!\n");
        return i32StreamID;
    }
    
    AVDictionary *opts = 0;
    av_dict_set(&opts, "dummy", "0", 0);
    SetAVDictionary(ptszOption,opts);
    av_opt_set_dict2(m_pCodeContext , &opts, AV_OPT_SEARCH_CHILDREN );
    av_dict_free(&opts);
    
    InitHWAccelGetFormat();
    
    if (avcodec_open2(m_pCodeContext, pCodec, &optionsDict) < 0) {
        DEBUG_PRINT("avcodec_open2 failed!\n");
        return i32StreamID;
    }
    
    m_TempFrame = av_frame_alloc();
    m_DecodedFrame = av_frame_alloc();
    
    return i32StreamID;
}
//----------------------------------------------------------------------
void C_Decoder::FrameCopy(AVFrame *pDstFrame,AVFrame *pSrcFrame)
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
void C_Decoder::SaveDecodedFrame(AVFrame *pFrame)
{
    pthread_mutex_lock(&m_lock);
    
    av_freep(&m_DecodedFrame->data[0]);
    av_frame_unref(m_DecodedFrame);
    
    FrameCopy(m_DecodedFrame,pFrame);

    pthread_mutex_unlock(&m_lock);
}
//----------------------------------------------------------------------
AVFrame* C_Decoder::DupDecodedFrame()
{
    AVFrame* pDupFrame = av_frame_alloc();
    
    pthread_mutex_lock(&m_lock);
    FrameCopy(pDupFrame,m_DecodedFrame);
    pthread_mutex_unlock(&m_lock);
    
    return pDupFrame;
}
//----------------------------------------------------------------------







