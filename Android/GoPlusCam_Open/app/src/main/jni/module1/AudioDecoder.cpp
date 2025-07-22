//
//  AudioDecoder.cpp
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2016/12/21.
//  Copyright © 2016年 generalplus_sa1. All rights reserved.
//

#include "AudioDecoder.h"


#define SIZE_OF_SAMPLE          2       //16bit ,AV_SAMPLE_FMT_S16
#define RAMP_TIME               0.5     //50ms
#define RAMP_DOWN_FACTOR        0.95
//----------------------------------------------------------------------
C_AudioDecoder::~C_AudioDecoder()
{
    
}
//----------------------------------------------------------------------
void C_AudioDecoder::initAudioResample()
{
    if(m_swr_audio_stx)
        swr_free(&m_swr_audio_stx);
    
    uint64_t chlayout = m_pCodeContext->channel_layout;
    if(chlayout == 0)
        chlayout = m_pCodeContext->channels+1;
    
    m_swr_audio_stx = swr_alloc_set_opts(
                                         m_swr_audio_stx,
                                         chlayout,
                                         AV_SAMPLE_FMT_S16,
                                         m_pCodeContext->sample_rate,
                                         chlayout,
                                         m_pCodeContext->sample_fmt,
                                         m_pCodeContext->sample_rate,
                                         0, 0);
    
    swr_init(m_swr_audio_stx);
    
    m_AudioStartAlign       = 0;
    
    AVStream *pVideoStream = m_pContextManager->GetStream(E_StreamType_Video);
    AVStream *pAudioStream = m_pContextManager->GetStream(E_StreamType_Audio);
    
    if(pVideoStream->duration != AV_NOPTS_VALUE &&
       pAudioStream->duration != AV_NOPTS_VALUE)
    {
        int64_t VideoStart = av_rescale_q (pVideoStream->start_time,
                                           pVideoStream->time_base,
                                           AV_TIME_BASE_Q );
        
        int64_t AudioStart = av_rescale_q (pAudioStream->start_time,
                                           pAudioStream->time_base,
                                           AV_TIME_BASE_Q );
        
        m_AudioStartAlign = VideoStart - AudioStart;
    }
    
}

//----------------------------------------------------------------------
bool C_AudioDecoder::Decode(AVPacket *pPacket)
{
    int     frameFinished;
    
    pthread_mutex_lock(&m_lock);
    
    avcodec_decode_audio4(m_pCodeContext, m_TempFrame, &frameFinished, pPacket);
    
    pthread_mutex_unlock(&m_lock);

    if(frameFinished)
    {
        SaveDecodedFrame(m_TempFrame);
        
        int64_t pts;
        pts = av_frame_get_best_effort_timestamp(m_TempFrame);

        uint8_t **output = NULL;

        av_samples_alloc_array_and_samples(&output,
                         m_TempFrame->linesize,
                         m_TempFrame->channels,
                         m_TempFrame->nb_samples,
                         AV_SAMPLE_FMT_S16,
                         0);
        
        int ret = swr_convert(m_swr_audio_stx,
                              output,
                              m_TempFrame->nb_samples,
                              (const uint8_t **)m_TempFrame->data,
                              m_TempFrame->nb_samples);
        
        AVStream *pStream = m_pContextManager->GetStream(E_StreamType_Audio);
        
        int64_t sampleIndex = pts - pStream->start_time - m_TempFrame->nb_samples;
        
        if(ret>0)
        {
            m_QueueIndex = sampleIndex;
            m_pFFmpegAgent->PlatformAudio(output,ret,sampleIndex);
        }
        else
            DEBUG_PRINT("swr_convert Failed!************************************\n");
        //DEBUG_PRINT("ret: %d pts: %lld \n",ret,pts);
        
        return true;
        
    }
    else
        return false;
}
//----------------------------------------------------------------------
void  C_AudioDecoder::FlushBuffer()
{
    m_QueueIndex = 0;
    C_Decoder::FlushBuffer();
}

