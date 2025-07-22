//
//  AudioDecoder.hpp
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2016/12/21.
//  Copyright © 2016年 generalplus_sa1. All rights reserved.
//

#ifndef AudioDecoder_hpp
#define AudioDecoder_hpp

#include "Decoder.h"

class C_AudioDecoder : public C_Decoder
{
public:
    
    C_AudioDecoder(C_PlayerInfo *pPlayerInfo):C_Decoder(pPlayerInfo),
    m_QueueIndex(0),m_bRampUp(false),m_RampUpStep(0)
    {
        
    }
    
    ~C_AudioDecoder();
    bool Decode(AVPacket *pPacket);
    void initAudioResample();
    int64_t GetAudioStartAlign()    { return m_AudioStartAlign; }
    int64_t GetQueueIndex()         { return m_QueueIndex; }
    virtual void  FlushBuffer();
    
    
private:
     
    SwrContext          *m_swr_audio_stx;
    int64_t             m_AudioStartAlign;
    int64_t             m_QueueIndex;
    
    bool                m_bRampUp;
    float               m_RampUpStep;
    
};

#endif /* AudioDecoder_hpp */
