//
//  QueueManager.hpp
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2016/12/21.
//  Copyright © 2016年 generalplus_sa1. All rights reserved.
//

#ifndef QueueManager_hpp
#define QueueManager_hpp

#ifdef __cplusplus
extern "C" {
#endif
    
#include <libavcodec/avcodec.h>

#ifdef __cplusplus
}
#endif

#include "Defines.h"
//----------------------------------------------------------------------
#include "QueueTemplate.h"
class C_FrameQueue : public T_Queue<AVFrame>
{
public:
    C_FrameQueue(){}
    ~C_FrameQueue(){}
    
    
    virtual void FreeObjectContent(AVFrame* pObject)
    {
        av_freep(&pObject->data[0]);
        av_frame_unref(pObject);
    }
};

class C_PacketQueue : public T_Queue<AVPacket>
{
public:
    C_PacketQueue(){}
    ~C_PacketQueue(){}
    
    
    virtual void FreeObjectContent(AVPacket* pObject)
    {
        av_free_packet(pObject);
    }
};
//----------------------------------------------------------------------
#endif /* QueueManager_hpp */
