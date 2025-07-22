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

    C_VideoDecoder(C_PlayerInfo *pPlayerInfo):C_Decoder(pPlayerInfo),m_bDebugMessage(false)
    {

    }


    ~C_VideoDecoder();

    bool Decode(AVPacket *pPacket);
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
    bool                m_bDebugMessage;
};


#endif /* VideoDecoder_hpp */
