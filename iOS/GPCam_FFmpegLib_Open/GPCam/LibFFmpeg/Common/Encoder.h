//
//  Encoder.hpp
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2016/12/21.
//  Copyright © 2016年 generalplus_sa1. All rights reserved.
//

#ifndef Encoder_hpp
#define Encoder_hpp

#include "CodecDefine.h"
#include "EventManager.h"
#include "PlayerInfo.h"
//----------------------------------------------------------------------
class C_Encoder
{
public:
    C_Encoder(C_PlayerInfo *pPlayerInfo);
    ~C_Encoder();
    
    
    void Close()
    {
        if(m_pCodeContext)
        {
            avcodec_close(m_pCodeContext);
            m_pCodeContext = NULL;
        }
    }
    
    
protected:
    C_PlayerInfo        *m_pPlayerInfo;
    AVCodecContext      *m_pCodeContext;
};

//----------------------------------------------------------------------

#endif /* Encoder_hpp */
