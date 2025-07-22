//
//  Encoder.cpp
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2016/12/21.
//  Copyright © 2016年 generalplus_sa1. All rights reserved.
//

#include "Encoder.h"
//----------------------------------------------------------------------
C_Encoder::C_Encoder(C_PlayerInfo *pPlayerInfo):
m_pPlayerInfo(pPlayerInfo),
m_pCodeContext(NULL)
{
    
}
//----------------------------------------------------------------------
C_Encoder::~C_Encoder()
{
    Close();
}
