//
//  VideoDecoder.cpp
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2016/12/21.
//  Copyright © 2016年 generalplus_sa1. All rights reserved.
//

#include "VideoDecoder_Android.h"

//----------------------------------------------------------------------
C_VideoDecoder::~C_VideoDecoder()
{

}
//----------------------------------------------------------------------
bool C_VideoDecoder::Decode(AVPacket *pPacket)
{
    int            			frameFinished;
    bool bRet = false;

    pthread_mutex_lock(&m_lock);
    //DEBUG_PRINT("Decode packet: %.10lld PTS: %.10lld\n",GetClock(),pPacket->pts);
    avcodec_decode_video2(m_pCodeContext, m_TempFrame, &frameFinished, pPacket);

    pthread_mutex_unlock(&m_lock);

    if(frameFinished)
    {
        if(m_bDebugMessage)
            InsertDebugMessage(pPacket,m_TempFrame,m_pCodeContext->width,m_pCodeContext->height);

        C_Event event(E_EventType_Decode_VideoFrame,(void*)m_TempFrame);
        C_EventManager::GetEvnetManager()->ProcessEvent(event);
        SaveDecodedFrame(m_TempFrame);

        //DEBUG_PRINT("Get Frame: %.10lld PTS: %.10lld\n",GetClock(),pFrame->pts);

        bRet = true;
    }

    return bRet;
}
//----------------------------------------------------------------------
