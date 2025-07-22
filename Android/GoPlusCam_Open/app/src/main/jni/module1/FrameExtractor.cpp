//
//  FrameExtractor.cpp
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2017/11/17.
//  Copyright © 2017年 generalplus_sa1. All rights reserved.
//

#include "FrameExtractor.h"
#define EXTRACTOR_OK                            0
#define EXTRACTOR_BUSY                          1
#define EXTRACTOR_READFILEFAILED                2
#define EXTRACTOR_DECODEFAILED                  3
#define EXTRACTOR_NOSUCHFRAME                   4
//----------------------------------------------------------------------
C_FrameExtractor::C_FrameExtractor():
m_bIsExtracting(false),m_SnapshotEncoder(NULL)
{
    
}
//----------------------------------------------------------------------
C_FrameExtractor::~C_FrameExtractor()
{
    
}
//----------------------------------------------------------------------
int C_FrameExtractor::ExtractFrame(const char* VideoPath,const char* SavePath, int64_t frameIdx)
{
    if(m_bIsExtracting)
        return EXTRACTOR_BUSY;
    
    AVFormatContext     *pFormatCtx = NULL;
    AVCodecContext      *pCodeContext = NULL;
    AVCodec             *pCodec = NULL;
    
    m_bIsExtracting = true;

    avcodec_register_all();
    av_register_all();
    avformat_network_init();
    
    if (avformat_open_input(&pFormatCtx, VideoPath, NULL, NULL) != 0) {
        DEBUG_PRINT("avformat_open_input %s failed!\n", VideoPath);
        m_bIsExtracting = false;
        return EXTRACTOR_READFILEFAILED;
    }

    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        DEBUG_PRINT("avformat_find_stream_info failed!\n");
        m_bIsExtracting = false;
        return EXTRACTOR_READFILEFAILED;
    }
    
    int i32StreamID = -1;
    if ((i32StreamID = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1,
                                           &pCodec, 0)) < 0) {
        DEBUG_PRINT("av_find_best_stream failed!\n");
        m_bIsExtracting = false;
        return EXTRACTOR_READFILEFAILED;
    }
    
    pCodeContext= pFormatCtx->streams[i32StreamID]->codec;
    pCodec = avcodec_find_decoder(pCodeContext->codec_id);
    
    if (avcodec_open2(pCodeContext, pCodec, NULL) < 0) {
        DEBUG_PRINT("avcodec_open2 failed!\n");
        m_bIsExtracting = false;
        return EXTRACTOR_READFILEFAILED;
    }
    
    int       i32Result = EXTRACTOR_OK;
    int64_t   frameCount=0;
    AVPacket   packet;
    AVFrame    *decodeFrame = av_frame_alloc();
    int        frameFinished;
    
    while(1)
    {
        int i32Ret = av_read_frame(pFormatCtx, &packet);
        if(i32Ret<0)
        {
            i32Result = EXTRACTOR_NOSUCHFRAME;
            break;
        }
        
        if(packet.stream_index == i32StreamID)
        {
            int i32Decode = avcodec_decode_video2(pCodeContext, decodeFrame, &frameFinished, &packet);
            if(i32Decode<0)
            {
                i32Result = EXTRACTOR_DECODEFAILED;
                break;
            }
            
            if(frameFinished)
            {
                if(frameCount == frameIdx)
                {
                    m_SnapshotEncoder.Encode(decodeFrame,
                                             pCodeContext,
                                             SavePath);
                    av_free_packet(&packet);
                    break;
                }
                frameCount++;
            }
            
        }
        
        av_free_packet(&packet);
    }
    
    
    if(decodeFrame)
        av_free(decodeFrame);
    
    if(pCodeContext)
        avcodec_close(pCodeContext);
    
    if(pFormatCtx)
        avformat_close_input(&pFormatCtx);
    
    m_bIsExtracting = false;
    
    return i32Result;
}
//----------------------------------------------------------------------
