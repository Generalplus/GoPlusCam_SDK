//
//  PictureEncoder.hpp
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2016/12/21.
//  Copyright © 2016年 generalplus_sa1. All rights reserved.
//

#ifndef PictureEncoder_hpp
#define PictureEncoder_hpp

#include "Encoder.h"

class C_PictureEncoder : public C_Encoder
{
public:

    C_PictureEncoder(C_PlayerInfo *pPlayerInfo):C_Encoder(pPlayerInfo)
    {

    }

    ~C_PictureEncoder();

    bool Encode(AVFrame *pInFrame, AVCodecContext *pCodecCtx, const char *ptszPath);
    
};


#endif /* PictureEncoder_hpp */
