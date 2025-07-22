//
//  Decoder.cpp
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2016/12/21.
//  Copyright © 2016年 generalplus_sa1. All rights reserved.
//

#include "Decoder.h"

//----------------------------------------------------------------------
C_Decoder::C_Decoder(C_PlayerInfo  *pPlayerInfo):
m_pPlayerInfo(pPlayerInfo),m_pCodeContext(NULL),m_TempFrame(NULL),m_DecodedFrame(NULL)
{
    pthread_mutex_init(&m_lock, NULL);
}
//----------------------------------------------------------------------
C_Decoder::~C_Decoder()
{
    Close();
}
//----------------------------------------------------------------------
int C_Decoder::OpenCodec(AVCodecContext *pCodeContext,AVCodecID CodeID,char* ptszOption)
{
    
    AVCodec         *pCodec = NULL;
    AVDictionary    *optionsDict = NULL;
    
    if(!pCodeContext)
    {
        if(!pCodec)
            pCodec = avcodec_find_decoder(CodeID);
        
        if (pCodec == NULL) {
            DEBUG_PRINT("Unsupported codec!\n");
            return -1;
        }
        m_pCodeContext = avcodec_alloc_context3(pCodec);

        if (pCodec->capabilities & AV_CODEC_CAP_TRUNCATED)
            m_pCodeContext->flags |= AV_CODEC_FLAG_TRUNCATED; // we do not send complete frames
    }
    else
        m_pCodeContext= pCodeContext;
    
    GetHWDecoder(m_pCodeContext->codec_id,&pCodec);

    if(!pCodec)
        pCodec = avcodec_find_decoder(m_pCodeContext->codec_id);

    if (pCodec == NULL) {
        DEBUG_PRINT("Unsupported codec!\n");
        return FFMPEGPLAYER_INITMEDIAFAILED;
    }
    
    AVDictionary *opts = 0;
    av_dict_set(&opts, "dummy", "0", 0);
    SetAVDictionary(ptszOption,opts);
    av_opt_set_dict2(m_pCodeContext , &opts, AV_OPT_SEARCH_CHILDREN );
    av_dict_free(&opts);
    
    SetupHWAccel();
    
    if (avcodec_open2(m_pCodeContext, pCodec, &optionsDict) < 0) {
        DEBUG_PRINT("avcodec_open2 failed!\n");
        return FFMPEGPLAYER_INITMEDIAFAILED;
    }
    
    m_TempFrame = av_frame_alloc();
    m_DecodedFrame = av_frame_alloc();
    
    return FFMPEGPLAYER_NOERROR;
}
//----------------------------------------------------------------------
void C_Decoder::SaveDecodedFrame(AVFrame *pFrame)
{
    pthread_mutex_lock(&m_lock);
    
    if(m_pCodeContext)
    {
        av_freep(&m_DecodedFrame->data[0]);
        av_frame_unref(m_DecodedFrame);
        
        FrameCopy(m_DecodedFrame,pFrame);
    }

    pthread_mutex_unlock(&m_lock);
}
//----------------------------------------------------------------------
AVFrame* C_Decoder::DupDecodedFrame()
{
    AVFrame* pDupFrame = av_frame_alloc();
    
    pthread_mutex_lock(&m_lock);
    if(m_pCodeContext)
    {
        FrameCopy(pDupFrame,m_DecodedFrame);
    }
    pthread_mutex_unlock(&m_lock);
    
    return pDupFrame;
}
//----------------------------------------------------------------------







