//
//  VideoEncoder.cpp
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2016/12/21.
//  Copyright © 2016年 generalplus_sa1. All rights reserved.
//

#include "VideoEncoder.h"

#define JFIF_TAG_SIZE       0x14
#define SKIP_SIZE           0x25F
#define COPY_SIZE           (SKIP_SIZE - JFIF_TAG_SIZE)
//----------------------------------------------------------------------
#define MOVING_AVERAGE_ORDER 25

typedef struct fir_s {
			 unsigned int init_flag;
			 int64_t filter[MOVING_AVERAGE_ORDER];
			 unsigned int idx;
} fir_t;

fir_t g_FIR;

int64_t fir_run_core(fir_t *instance, int64_t in)
{
    unsigned int i;
    int64_t sum = 0;
    double sum_floating;

    instance->filter[instance->idx++] = in;
    for (i=0; i<MOVING_AVERAGE_ORDER; ++i)
    {
        sum += instance->filter[i];
    }
    sum_floating = (double)sum/(double)MOVING_AVERAGE_ORDER;

    if (instance->idx>=MOVING_AVERAGE_ORDER)
        instance->idx = 0;
    return (int64_t)sum_floating;
}

int64_t fir_run(fir_t *instance, int64_t in)
{
    int64_t out = 0;

    if (instance->init_flag == 0)
    {
        int i;
        for (i=0; i<MOVING_AVERAGE_ORDER; ++i)
        {
            out = fir_run_core(instance, in);
        }
        instance->init_flag = 1;
    }
    else
    {
        out = fir_run_core(instance, in);
    }

    //DEBUG_PRINT("out = %lld\n",out);
    return out;
}

void fir_init(fir_t *instance)
{
    memset((void *)instance, 0, sizeof(fir_t));
}
//----------------------------------------------------------------------
const char *GetFileExt(const char *filename)
{
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename)
        return "";

    return dot + 1;
}
//----------------------------------------------------------------------
C_VideoEncoder::~C_VideoEncoder()
{
    ClearQueue();
    UnLockQueue();
    CloseVideo();
}
//----------------------------------------------------------------------
void C_VideoEncoder::init(AVCodecContext *pVideoCodecCtx,
                          AVCodecContext *pAudioCodecCtx)
{
    m_pVideoCodecCtx = pVideoCodecCtx;
    m_pAudioCodecCtx = pAudioCodecCtx;

    memset(&m_TimeFixInfo,0x00,sizeof(m_TimeFixInfo));

    if(m_outsws_ctx)
    {
        sws_freeContext(m_outsws_ctx);
        m_outsws_ctx = NULL;
    }

    AVPixelFormat format = pVideoCodecCtx->sw_pix_fmt;
    if(format == AV_PIX_FMT_NONE)
        format = pVideoCodecCtx->pix_fmt;

    // Setup scaler
    static int sws_flags =  SWS_FAST_BILINEAR;
    m_outsws_ctx = sws_getContext(
                                  pVideoCodecCtx->width,
                                  pVideoCodecCtx->height,
                                  format,
                                  pVideoCodecCtx->width,
                                  pVideoCodecCtx->height,
                                  AV_PIX_FMT_YUV420P,
                                  sws_flags, NULL, NULL, NULL);
    ClearQueue();
    UnLockQueue();

}
//----------------------------------------------------------------------
bool C_VideoEncoder::CreateEncodeRawStream(char *ptszPath,float fFPS,bool bIsRTSP,bool bUsingLocalTime)
{
    m_fVideofps = fFPS;
    m_bIsRTSP = bIsRTSP;
    m_bUsingLocalTime = bUsingLocalTime;

    if(m_pVideoCodecCtx->codec_id == AV_CODEC_ID_MJPEG)
    {
        m_outFmt = av_guess_format("avi", NULL, NULL);
        if(strcmp(GetFileExt(ptszPath),"avi") !=0)
            strcat(ptszPath,".avi");

        m_EncodeInfo.Container = E_EncodeContainer_AVI;
    }
    else
    {
        m_fVideofps = fFPS;

        m_outFmt = av_guess_format("mp4", NULL, NULL);
        if(strcmp(GetFileExt(ptszPath),"mp4") !=0)
            strcat(ptszPath,".mp4");

        m_EncodeInfo.Container = E_EncodeContainer_MP4;
    }


    if (!m_outFmt)
    {
        DEBUG_PRINT("av_guess_format failed\n");
        return false;
    }

    int err = avformat_alloc_output_context2(&m_outCtx, m_outFmt, NULL, NULL);

    if (err < 0 || !m_outCtx)
    {
        DEBUG_PRINT("avformat_alloc_output_context2 failed\n");
        return false;
    }

    //header format
    m_outStrm = avformat_new_stream(m_outCtx, NULL);
    avcodec_copy_context( m_outStrm->codec, m_pVideoCodecCtx );

    m_outStrm->sample_aspect_ratio.num = m_pVideoCodecCtx->sample_aspect_ratio.num;
    m_outStrm->sample_aspect_ratio.den = m_pVideoCodecCtx->sample_aspect_ratio.den;

    // Assume r_frame_rate is accurate
    if(m_EncodeInfo.Container == E_EncodeContainer_AVI)
        m_outStrm->r_frame_rate = av_make_q(m_fVideofps,1);
    else
        m_outStrm->r_frame_rate = m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->r_frame_rate;

    m_outStrm->avg_frame_rate = m_outStrm->r_frame_rate;
    m_outStrm->time_base = av_inv_q( m_outStrm->r_frame_rate );
    m_outStrm->codec->time_base = m_outStrm->time_base;


    if(m_pAudioCodecCtx)
    {
        m_outAudioStrm = avformat_new_stream(m_outCtx, NULL);
        avcodec_copy_context( m_outAudioStrm->codec, m_pAudioCodecCtx );

        m_outAudioStrm->sample_aspect_ratio.num = m_pAudioCodecCtx->sample_aspect_ratio.num;
        m_outAudioStrm->sample_aspect_ratio.den = m_pAudioCodecCtx->sample_aspect_ratio.den;

        m_outAudioStrm->time_base = m_pPlayerInfo->GetTrackInfo(E_TrackAudio)->time_base;
    }

    err = avio_open(&m_outCtx->pb, ptszPath  , AVIO_FLAG_WRITE);
    if (err < 0)
    {
        DEBUG_PRINT("avio_open2 failed %s\n",ptszPath);
        return false;
    }

    avformat_write_header(m_outCtx, NULL);
    av_dump_format( m_outCtx, 0, m_outCtx->filename, 1 );

    m_EncodeInfo.AudioStartPts = -1;
    m_EncodeInfo.VideoStartPts = -1;
    m_EncodeInfo.Diff = 0;
    m_EncodeInfo.Last_Video_pts = -1;
    m_EncodeInfo.bIsSetDiff = false;

    return true;
}
//----------------------------------------------------------------------
bool C_VideoEncoder::CreateEncodeStream(AVCodecID CodecID , char *ptszPath, float fFPS, char *ptszOption)
{
    m_EncodeID = CodecID;
    m_fVideofps = fFPS;
    m_EncodeInfo.Container = E_EncodeContainer_MP4;

    m_outFmt = av_guess_format("mp4", NULL, NULL);
    if (!m_outFmt)
    {
        DEBUG_PRINT("av_guess_format failed\n");
        return false;
    }

    if(strcmp(GetFileExt(ptszPath),"mp4") !=0)
        strcat(ptszPath,".mp4");

    int err = avformat_alloc_output_context2(&m_outCtx, m_outFmt, NULL, NULL);

    if (err < 0 || !m_outCtx)
    {
        DEBUG_PRINT("avformat_alloc_output_context2 failed\n");
        return false;
    }

    AVCodec *codec;
    codec = avcodec_find_encoder( m_EncodeID);
    if (!codec) {
        DEBUG_PRINT("Codec not found\n");
        return false;
    }

    m_pCodeContext = avcodec_alloc_context3(codec);
    if (!m_pCodeContext) {
        DEBUG_PRINT("Could not allocate video codec context\n");
        return false;
    }

    //streaming format
    m_pCodeContext->width = m_pVideoCodecCtx->width;
    m_pCodeContext->height = m_pVideoCodecCtx->height;
    m_pCodeContext->sample_aspect_ratio = m_pVideoCodecCtx->sample_aspect_ratio;
    m_pCodeContext->time_base = av_make_q(1,m_fVideofps);

    if(m_EncodeID == AV_CODEC_ID_MJPEG)
    {
        m_pCodeContext->bit_rate = m_pVideoCodecCtx->bit_rate;
        m_pCodeContext->pix_fmt = m_pVideoCodecCtx->sw_pix_fmt;

    }else
    {
        //Kush Gauge: pixel count  x motion factor(1,2 or 4)  x 0.07/ 1000 = bit rate in kbps
        int motionfactor = 4;

        m_pCodeContext->bit_rate = m_pCodeContext->width * m_pCodeContext->height * m_fVideofps * motionfactor * 0.07;
        m_pCodeContext->pix_fmt = AV_PIX_FMT_YUV420P;
        m_pCodeContext->gop_size = 32;
        m_pCodeContext->qmin = 3;                                              // minimum quantizer
        m_pCodeContext->qmax = 31;                                             // maximum quantizer
    }

    AVDictionary *opts = 0;
    av_dict_set(&opts, "dummy", "0", 0);
    SetAVDictionary(ptszOption,opts);
    av_opt_set_dict2(m_pCodeContext , &opts, AV_OPT_SEARCH_CHILDREN );
    av_dict_free(&opts);

    if(m_outCtx->oformat->flags & AVFMT_GLOBALHEADER)
        m_pCodeContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    if (avcodec_open2(m_pCodeContext, codec, NULL) < 0) {
        DEBUG_PRINT("Could not open codec\n");
        avformat_free_context(m_outCtx);
        m_outCtx = NULL;
        Close();
        return false;
    }

    //header format
    m_outStrm = avformat_new_stream(m_outCtx, codec);

    m_outStrm->codec->thread_count = 1;
    m_outStrm->codec->coder_type = AVMEDIA_TYPE_VIDEO;
    if(m_outCtx->oformat->flags & AVFMT_GLOBALHEADER)
        m_outStrm->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    m_outStrm->codec->sample_aspect_ratio = m_outStrm->sample_aspect_ratio = m_pCodeContext->sample_aspect_ratio;
    m_outStrm->codec->bits_per_raw_sample = m_pCodeContext->bits_per_raw_sample;
    m_outStrm->codec->chroma_sample_location = m_pCodeContext->chroma_sample_location;
    m_outStrm->codec->codec_id = m_EncodeID;
    m_outStrm->codec->width = m_pCodeContext->width;
    m_outStrm->codec->height = m_pCodeContext->height;
    m_outStrm->codec->time_base = m_pCodeContext->time_base;

    err = avio_open(&m_outCtx->pb, ptszPath  , AVIO_FLAG_WRITE);
    if (err < 0)
    {
        DEBUG_PRINT("avio_open2 failed %s\n",ptszPath);
        return false;
    }

    if(m_pCodeContext->extradata)
    {
        // Apple Quick time needs the MPEG4 header in extradata
        unsigned char *newData = (unsigned char *)malloc(m_pCodeContext->extradata_size);
        memcpy(newData,m_pCodeContext->extradata,m_pCodeContext->extradata_size);
        m_outStrm->codec->extradata = newData;
        m_outStrm->codec->extradata_size = m_pCodeContext->extradata_size;
    }

    avformat_write_header(m_outCtx, NULL);

    fir_init(&g_FIR);

    return true;
}
//----------------------------------------------------------------------
bool C_VideoEncoder::pushRawPacket(AVPacket *pPacket)
{
    m_RawPacketQueue.PushOject(pPacket);

    return true;
}
//----------------------------------------------------------------------
bool C_VideoEncoder::writeRawStream()
{
    int64_t pts =0;

    AVPacket *pPacket = m_RawPacketQueue.PopObject(true);
    if(pPacket == NULL)
        return true;

    do
    {
        if(pPacket->stream_index == m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->i32StreamID)
        {

            if(m_EncodeInfo.VideoStartPts == -1)
            {
                if(pPacket->flags & AV_PKT_FLAG_KEY)
                {
                    m_EncodeInfo.VideoStartPts = pPacket->pts;
                }
                else
                    break;
            }

            if(!m_EncodeInfo.bIsSetDiff && m_EncodeInfo.AudioStartPts != -1)
            {
                int64_t AudioToVideo = av_rescale_q (m_EncodeInfo.AudioStartPts,
                                                     m_pPlayerInfo->GetTrackInfo(E_TrackAudio)->time_base,
                                                     m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->time_base);

                m_EncodeInfo.Diff = m_EncodeInfo.VideoStartPts - AudioToVideo;
                m_EncodeInfo.bIsSetDiff = true;
            }

            writeRawVideoPacket(pPacket);

        }
        else if(pPacket->stream_index == m_pPlayerInfo->GetTrackInfo(E_TrackAudio)->i32StreamID)
        {
            if(m_EncodeInfo.AudioStartPts == -1 )
            {
                if(pPacket->flags & AV_PKT_FLAG_KEY)
                {
                    m_EncodeInfo.AudioStartPts = pPacket->pts;
                }
                else
                    break;
            }

            if(pPacket->pts != AV_NOPTS_VALUE)
            {
                pts = pPacket->pts - m_EncodeInfo.AudioStartPts;
                pPacket->pts = pts;
                av_packet_rescale_ts(pPacket,m_outAudioStrm->codec->time_base,m_outAudioStrm->time_base);
            }

            //DEBUG_PRINT("Audio pts: %lld\n",pPacket->pts);
            pPacket->dts = pPacket->pts;

            int i32Ret = av_interleaved_write_frame(m_outCtx, pPacket);
            if(i32Ret!=0)
            {
                char errstr[256];

                av_strerror(i32Ret,errstr,sizeof(errstr));
                DEBUG_PRINT("av_interleaved_write_frame %d %s\n" ,i32Ret,errstr);
            }

        }

    }while(0);

    av_free_packet(pPacket);
    delete pPacket;

    return true;
}
//----------------------------------------------------------------------
void C_VideoEncoder::writeRawVideoPacket(AVPacket *pPacket)
{
    int64_t pts =0;
    int64_t lastpts = 0;
    bool bNeedDelete = false;

    AVPacket *pWritePkt = pPacket;

    if(!m_bUsingLocalTime)
    {
        if(pPacket->pts != AV_NOPTS_VALUE)
        {
            pts = pPacket->pts - m_EncodeInfo.VideoStartPts + m_EncodeInfo.Diff;
            pts = av_rescale_q (pts,
                                m_pPlayerInfo->GetTrackInfo(E_TrackVideo)->time_base,
                                m_outStrm->time_base );

            pPacket->pts = pts;
        }


        if(m_EncodeInfo.Last_Video_pts == pPacket->pts)
            pPacket->pts++; // pts not allow same value
    }
    else
    {
        pPacket->pts = GetPtsByTimeEsc();
        AVRational localTimeBase = {1,(int)m_fVideofps * m_pVideoCodecCtx->ticks_per_frame};
        if(av_cmp_q(localTimeBase,m_outStrm->time_base)!=0)
            av_packet_rescale_ts(pPacket,localTimeBase,m_outStrm->time_base);
    }

    lastpts = pPacket->dts = pPacket->pts;

    if(m_EncodeInfo.Container == E_EncodeContainer_AVI)
    {
        //fill missing frame
        int i32dup = (int)((pPacket->pts - m_EncodeInfo.Last_Video_pts)-1);
        for(int i=0;i32dup>0 && m_prevPacket;i32dup--,i++)
        {
            m_prevPacket->pts++;
            m_prevPacket->dts = m_prevPacket->pts;

            //av_interleaved_write_frame() will free pakcet
            AVPacket *Wtempkt = new AVPacket;
            av_init_packet(Wtempkt);
            av_copy_packet(Wtempkt, m_prevPacket);

            int i32Ret = av_interleaved_write_frame(m_outCtx, Wtempkt);
            if(i32Ret!=0)
            {
                char errstr[256];

                av_strerror(i32Ret,errstr,sizeof(errstr));
                DEBUG_PRINT("av_interleaved_write_frame %d %s\n" ,i32Ret,errstr);
            }

            delete Wtempkt;
        }

        AVPacket *tempkt = new AVPacket;
        av_init_packet(tempkt);

        if(m_bIsRTSP)
        {
            // Remove header which appended by ffmpeg
            av_new_packet(tempkt,pPacket->size - SKIP_SIZE);
            memcpy(&tempkt->data[0x00],&pPacket->data[SKIP_SIZE], pPacket->size - SKIP_SIZE);

            tempkt->dts = tempkt->pts = pPacket->pts;
            tempkt->duration = pPacket->duration;
        }
        else
        {
            av_new_packet(tempkt,pPacket->size );
            memcpy(&tempkt->data[0],&pPacket->data[0], pPacket->size);
            tempkt->dts = tempkt->pts = pPacket->pts;
            tempkt->duration = 1;
        }

        if(m_prevPacket)
        {
            av_free_packet(m_prevPacket);
            delete m_prevPacket;
            m_prevPacket = NULL;
        }

        m_prevPacket = new AVPacket;
        av_init_packet(m_prevPacket);
        av_copy_packet(m_prevPacket, tempkt);

        pWritePkt = tempkt;
        bNeedDelete = true;
    }

    int i32Ret = av_interleaved_write_frame(m_outCtx, pWritePkt);
    if(i32Ret!=0)
    {
        char errstr[256];

        av_strerror(i32Ret,errstr,sizeof(errstr));
        DEBUG_PRINT("av_interleaved_write_frame %d %s\n" ,i32Ret,errstr);
    }

    if(bNeedDelete)
        delete pWritePkt;

    m_EncodeInfo.Last_Video_pts = lastpts;
}
//----------------------------------------------------------------------
int64_t C_VideoEncoder::GetPtsByTimeEsc()
{
    int64_t pts = 0;
    int i32FrameCount = 1;

    int64_t now = GetClock();

    if(m_TimeFixInfo.prevTime!=0)
    {
        //complement the frame
        float frameTime = ((float)1/ (float)m_fVideofps) * AV_TIME_BASE;
        int64_t timeEsc = (now - m_TimeFixInfo.prevTime) + m_TimeFixInfo.prevLeft;

        //timeEsc=fir_run(&g_FIR, timeEsc);

        if(timeEsc < frameTime)
        {
            m_TimeFixInfo.prevLeft = timeEsc - frameTime;
        }
        else
        {
            m_TimeFixInfo.prevLeft =  timeEsc % (int64_t)frameTime ;
            i32FrameCount += (timeEsc / (int64_t)frameTime) - 1;
        }
    }
    m_TimeFixInfo.prevTime = now;
    pts = m_TimeFixInfo.FrameCnt;
    m_TimeFixInfo.FrameCnt+= m_pVideoCodecCtx->ticks_per_frame * i32FrameCount; //next index

    return pts;
}
//----------------------------------------------------------------------
int C_VideoEncoder::pushTranscodeFrame(AVFrame* pFrame)
{
    //add to queue
    AVFrame *pWriteFrame = av_frame_alloc();
    pWriteFrame->format = m_pCodeContext->pix_fmt;
    pWriteFrame->width  = m_pCodeContext->width;
    pWriteFrame->height = m_pCodeContext->height;
    int ret = av_image_alloc(pWriteFrame->data, pWriteFrame->linesize, pWriteFrame->width, pWriteFrame->height,m_pCodeContext->pix_fmt, 1);

    if(m_pCodeContext->pix_fmt != pFrame->format)
    {

        ret = sws_scale(m_outsws_ctx,
                        pFrame->data,
                        pFrame->linesize,
                        0,
                        pWriteFrame->height,
                        pWriteFrame->data,
                        pWriteFrame->linesize);
    }
    else
    {
        av_frame_copy(pWriteFrame, pFrame);
    }

    pWriteFrame->pts = GetPtsByTimeEsc();
    //DEBUG_PRINT("pFrame->pts: %lld\n",pFrame->pts);
    m_ScaledFrameQueue.PushOject(pWriteFrame);

    return FFMPEGPLAYER_NOERROR;
}
//----------------------------------------------------------------------
int  C_VideoEncoder::writeTranscodeFrame()
{
    AVFrame *pOutFrame = m_ScaledFrameQueue.PopObject(true);

    if(pOutFrame == NULL )
        return FFMPEGPLAYER_NOERROR;

    int ret,got_output;
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    ret = avcodec_encode_video2(m_pCodeContext, &pkt, pOutFrame, &got_output);
    if (ret < 0) {
        char msg[1024];
        av_strerror(ret,msg,sizeof(msg));
        DEBUG_PRINT("Error encoding frame %s\n",msg);
        return FFMPEGPLAYER_SAVEVIDEOFAILED;
    }
    if (got_output) {

        writePacket(&pkt);
    }

/*#if defined(__APPLE__)  //Android x264 library not accept null packet
    for (got_output = 1; got_output;){

        ret = avcodec_encode_video2(m_pCodeContext, &pkt, NULL, &got_output);
        if (ret < 0) {
            DEBUG_PRINT("Error encoding frame\n");
            //return FFMPEGPLAYER_SAVEVIDEOFAILED;
        }

        if (got_output) {
            writePacket(&pkt);

        }

    }
#endif*/

    av_freep(&pOutFrame->data[0]);
    av_frame_unref(pOutFrame);
    av_frame_free(&pOutFrame);


    return FFMPEGPLAYER_NOERROR;
}
//----------------------------------------------------------------------
int C_VideoEncoder::writePacket(AVPacket* pPacket)
{
    if(m_pCodeContext->coded_frame->key_frame)
        pPacket->flags |= AV_PKT_FLAG_KEY;

    //DEBUG_PRINT("pkt.pts %lld\n",pkt.pts);
    pPacket->stream_index = m_outStrm->index;
    writePacketDirectly(pPacket);

    return FFMPEGPLAYER_NOERROR;
}
//----------------------------------------------------------------------
int C_VideoEncoder::writePacketDirectly(AVPacket* pPacket)
{
    //DEBUG_PRINT("pPacket->pts: %lld pPacket->dts %lld\n",pPacket->pts,pPacket->dts);
    av_packet_rescale_ts(pPacket,m_outStrm->codec->time_base,m_outStrm->time_base);
#if defined(__APPLE__) //Android x264 library do not set dts
    pPacket->dts = pPacket->pts;
#endif

    int i32Ret = av_interleaved_write_frame(m_outCtx, pPacket);
    if(i32Ret!=0)
    {
        char errstr[256];

        av_strerror(i32Ret,errstr,sizeof(errstr));
        DEBUG_PRINT("av_interleaved_write_frame %d %s\n" ,i32Ret,errstr);
        av_packet_unref(pPacket);

        return i32Ret;
    }

    return FFMPEGPLAYER_NOERROR;
}
//----------------------------------------------------------------------
void C_VideoEncoder::CloseVideo()
{
    DEBUG_PRINT("CloseVideo\n");
    if(m_outFmt!=NULL)
    {
        if(m_outCtx)
        {
            av_write_trailer(m_outCtx);
            if (!(m_outCtx->oformat->flags & AVFMT_NOFILE) && m_outCtx->pb)
                avio_close(m_outCtx->pb);

            avformat_free_context(m_outCtx);
        }

        Close();

        m_outFmt = NULL;
        m_outCtx = NULL;

    }

    if(m_prevPacket)
    {
        av_free_packet(m_prevPacket);
        delete m_prevPacket;
        m_prevPacket = NULL;
    }

    ClearQueue();
    UnLockQueue();
}
