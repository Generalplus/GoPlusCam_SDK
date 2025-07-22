//
//  Decoder.hpp
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2016/12/21.
//  Copyright © 2016年 generalplus_sa1. All rights reserved.
//

#ifndef Decoder_hpp
#define Decoder_hpp

#include "CodecDefine.h"

//----------------------------------------------------------------------

class C_Decoder
{
public:
    C_Decoder(I_ContextManager *pContextManager);
    ~C_Decoder();
    
    
    virtual int OpenCodec(AVMediaType StreamType, char* ptszOption);
    AVCodecContext* GetCodecContext()       { return m_pCodeContext;}
    virtual AVFrame* GetDecodeFrame()       { return m_DecodedFrame;}
    
    virtual void FrameCopy(AVFrame *pDstFrame,AVFrame *pSrcFrame);
    virtual void SaveDecodedFrame(AVFrame *pFrame);
    AVFrame* DupDecodedFrame();
    
    void setAgent(I_FFmpegAgnet *pAgent)
    {
        m_pFFmpegAgent = pAgent;
    }
    
    virtual void Close()
    {
        if(m_TempFrame)
        {
            av_free(m_TempFrame);
            m_TempFrame = NULL;
        }
    
        if(m_DecodedFrame)
        {
            av_freep(&m_DecodedFrame->data[0]);
            av_frame_unref(m_DecodedFrame);
            av_free(m_DecodedFrame);
            m_DecodedFrame = NULL;
        }
        
        pthread_mutex_lock(&m_lock);

        if(m_pCodeContext)
        {
            avcodec_close(m_pCodeContext);
            m_pCodeContext = NULL;
        }
        
        pthread_mutex_unlock(&m_lock);

    }
    
    virtual void  FlushBuffer()
    {
        pthread_mutex_lock(&m_lock);
        
        avcodec_flush_buffers(m_pCodeContext);
        
        pthread_mutex_unlock(&m_lock);
    }

    virtual void GetHWDecoder(AVCodecID codeID,AVCodec  **ppCodec) {}
    virtual void InitHWAccelGetFormat() {}
protected:
    
    I_ContextManager    *m_pContextManager;
    I_FFmpegAgnet       *m_pFFmpegAgent;
    
    AVCodecContext      *m_pCodeContext;
    AVFrame         	*m_TempFrame;
    AVFrame             *m_DecodedFrame;
    pthread_mutex_t     m_lock;
    AVHWAccel           *m_hwaccel;
};


//----------------------------------------------------------------------

#endif /* Decoder_hpp */
