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
#include <VideoToolbox/VideoToolbox.h>

class C_VideoDecoder : public C_Decoder
{
public:
    C_VideoDecoder(C_PlayerInfo *pPlayerInfo):C_Decoder(pPlayerInfo),
    m_sws_ctx(NULL),m_PrevReInitTime(0),m_hwaccel(NULL),m_bDebugMessage(false)
    {

    }

    ~C_VideoDecoder();

    virtual int OpenCodec(AVCodecContext *pCodeContext,AVCodecID CodeID,char* ptszOption);

    bool Decode(AVPacket *pPacket);

    int videotoolbox_retrieve_data(AVCodecContext *s);

    virtual void FlushBuffer();
    virtual void Close();
    virtual void SetupHWAccel();

    void ReinitHWAccel();
    void EnableDebugMessage(bool bEnable) { m_bDebugMessage = bEnable; }


private:

    static void didDecompress(void *decompressionOutputRefCon, void *sourceFrameRefCon, OSStatus status, VTDecodeInfoFlags infoFlags, CVImageBufferRef imageBuffer, CMTime presentationTimeStamp, CMTime presentationDuration );

    bool HWDecodeMJpeg(AVPacket *pPacket, int *pResult);

    CMFormatDescriptionRef      m_videoFormatDescr;
    VTDecompressionSessionRef   m_session;
    CVPixelBufferRef            m_frame;

    
    struct SwsContext           *m_sws_ctx;
    int64_t                     m_PrevReInitTime;
    AVHWAccel                   *m_hwaccel;
    bool                        m_bDebugMessage;

};


#endif /* VideoDecoder_hpp */
