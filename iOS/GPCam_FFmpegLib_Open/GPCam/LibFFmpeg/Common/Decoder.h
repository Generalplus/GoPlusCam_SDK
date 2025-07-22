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
#include "EventManager.h"
#include "PlayerInfo.h"
//----------------------------------------------------------------------

class C_Decoder
{
public:
    C_Decoder(C_PlayerInfo *pPlayerInfo);
    ~C_Decoder();
    
    
    virtual int OpenCodec(AVCodecContext *pCodeContext,AVCodecID CodeID,char* ptszOption);
    AVCodecContext* GetCodecContext()       { return m_pCodeContext;}
    virtual AVFrame* GetDecodeFrame()       { return m_DecodedFrame;}
    
    virtual void SaveDecodedFrame(AVFrame *pFrame);
    AVFrame* DupDecodedFrame();
    
    virtual void Close()
    {
        pthread_mutex_lock(&m_lock);

        if(m_pCodeContext)
        {
            avcodec_close(m_pCodeContext);
            m_pCodeContext = NULL;
        }
        
        pthread_mutex_unlock(&m_lock);

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
    }
    
    virtual void  FlushBuffer()
    {
        pthread_mutex_lock(&m_lock);
        
        avcodec_flush_buffers(m_pCodeContext);
        
        pthread_mutex_unlock(&m_lock);
    }
    
    virtual void SetupHWAccel() {}
    virtual void GetHWDecoder(AVCodecID codeID,AVCodec  **ppCodec) {}
    
    void pushDecodePacket(AVPacket *pPacket) { m_DecodePacketQueue.PushOject(pPacket); }
    AVPacket *PopDecodePacket() {  return m_DecodePacketQueue.PopObject(true); }
    int GetDecodeSize()     { return m_DecodePacketQueue.GetSize(); }
    
    
    virtual void UnLockQueue()
    {
        m_DecodePacketQueue.UnLockQueue();
    }
    
    virtual void ClearQueue()
    {
        m_DecodePacketQueue.ClearQueue();
    }
    
    
protected:
    C_PlayerInfo        *m_pPlayerInfo;
    AVCodecContext      *m_pCodeContext;
    AVFrame         	*m_TempFrame;
    AVFrame             *m_DecodedFrame;
    pthread_mutex_t     m_lock;
    C_PacketQueue       m_DecodePacketQueue;
};


//----------------------------------------------------------------------

#endif /* Decoder_hpp */
