//
//  VideoDecoder.hpp
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2016/12/21.
//  Copyright © 2016年 generalplus_sa1. All rights reserved.
//

#ifndef VideoDecoder_IOS_hpp
#define VideoDecoder_IOS_hpp

#include "Decoder.h"

class C_VideoDecoder : public C_Decoder
{
public:
    C_VideoDecoder(I_ContextManager *pContextManager):
                   C_Decoder(pContextManager),
                   m_sws_ctx(NULL),m_PrevReInitTime(0),m_bDebugMessage(false)
    {
        
    }
    
    
    ~C_VideoDecoder();
    
    virtual int OpenCodec(AVMediaType StreamType, char* ptszOption);
    
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
    
    int videotoolbox_retrieve_data(AVCodecContext *s);
    
    virtual void FlushBuffer();
    virtual void Close();
    virtual void InitHWAccelGetFormat();
    
    void ReinitHWAccel();
    void EnableDebugMessage(bool bEnable) { m_bDebugMessage = bEnable; }
    
private:
    
    C_FrameQueue        m_DisplayFrameQueue;
    C_PacketQueue       m_DecodePacketQueue;
    struct SwsContext   *m_sws_ctx;
    int64_t             m_PrevReInitTime;
    bool                m_bDebugMessage;
    

};


#endif /* VideoDecoder_hpp */
