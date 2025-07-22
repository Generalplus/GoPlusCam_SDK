//
//  VideoDecoder.hpp
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2016/12/21.
//  Copyright © 2016年 generalplus_sa1. All rights reserved.
//

//#define HWDECODE_H264

#ifndef VideoDecoder_Android_hpp
#define VideoDecoder_Android_hpp

#include "Decoder.h"

class C_VideoDecoder : public C_Decoder
{
public:
    C_VideoDecoder(I_ContextManager *pContextManager):
                   C_Decoder(pContextManager),m_bDebugMessage(false)
    {
        
    }
    
    
    ~C_VideoDecoder();
    
    bool Decode(AVPacket *pPacket);
    
    void pushDecodePacket(AVPacket *pPacket) { m_DecodePacketQueue.PushOject(pPacket); }
    
    bool IsDisplaying()
    {
        return !m_DisplayFrameQueue.IsHaveWaiting();
    }
    
    bool IsDecoding()
    {
        return !m_DecodePacketQueue.IsHaveWaiting();
    }
    
    AVPacket *PopDecodePacket() {  return m_DecodePacketQueue.PopObject(true); }
    AVFrame  *PopDisplay()      {  return m_DisplayFrameQueue.PopObject(true); }
    
    int GetDisplaySize()    { return m_DisplayFrameQueue.GetSize(); }
    int GetDecodeSize()     { return m_DecodePacketQueue.GetSize(); }
    
    void UnLockQueue()
    {
        m_DisplayFrameQueue.UnLockQueue();
        m_DecodePacketQueue.UnLockQueue();
    }
    
    void ClearQueue()
    {
        m_DisplayFrameQueue.ClearQueue();
        m_DecodePacketQueue.ClearQueue();
    }
    
    void EnableDebugMessage(bool bEnable) { m_bDebugMessage = bEnable; }

#ifdef HWDECODE_H264

    virtual void GetHWDecoder(AVCodecID codeID,AVCodec  **ppCodec)
    {
        if(codeID == AV_CODEC_ID_H264)
            *ppCodec = avcodec_find_decoder_by_name("h264_mediacodec");
    }

    virtual void  FlushBuffer() {}

#endif

private:
    
    C_FrameQueue        m_DisplayFrameQueue;
    C_PacketQueue       m_DecodePacketQueue;
    bool                m_bDebugMessage;

};


#endif /* VideoDecoder_hpp */
