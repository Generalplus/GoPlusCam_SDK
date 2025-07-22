
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>

#include "VideoAgent.h"
#include "FFmpegPlayerCore.h"
#include "libavutil/imgutils.h"
//--------------------------------------------------------------------------
C_VideoAgent::C_VideoAgent(): m_bInited(false)
{
    pthread_mutex_init(&m_NewDatalock, NULL);
    m_pData[0] = NULL;
}

C_VideoAgent::~C_VideoAgent()
{

}

//--------------------------------------------------------------------------
bool C_VideoAgent::init()
{
    _render.setGLView(this);
    _render.init();


    return true;
}
//--------------------------------------------------------------------------
void C_VideoAgent::LoadGLBuffer()
{
    DEBUG_PRINT("LoadGLBuffer");
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8_OES, _Viewwidth, _Viewheight);
}

//--------------------------------------------------------------------------
int C_VideoAgent::PlatformDisplay(uint8_t *pData[], int width, int height, int format , int lineSizes[4])
{
    pthread_mutex_lock(&m_NewDatalock);

    if(m_pData[0]==NULL)
    {
        pthread_mutex_unlock(&m_NewDatalock);
        return 0;
    }

    if(width!=_width || height!=_height || format!=_i32Format)
        SetResolution(width,height,format,lineSizes);

    av_image_copy(m_pData,lineSizes,(const uint8_t **)pData,lineSizes,(AVPixelFormat)format, width, height);
    pthread_mutex_unlock(&m_NewDatalock);

    return 0;
}
//--------------------------------------------------------------------------
void C_VideoAgent::SetViewSize(int width, int height)
{
    _Viewwidth  = width;
    _Viewheight = height;

    _render.setViewSize(width,height);
    _render.layoutSubviews();
}
//--------------------------------------------------------------------------
void C_VideoAgent::SetResolution(int width, int height,int format,int linesizes[4])
{
    DEBUG_PRINT("SetResolution");

    _render.setWidth(width);
    _render.setHeight(height);

    if(m_pData[0]!=NULL) {
        av_freep(&m_pData[0]);
    }

    _width = width;
    _height = height;
    _i32Format = format;

    av_image_alloc(m_pData, linesizes, width, height ,(AVPixelFormat)_i32Format, 1);
    ClearPicure();
    _render.Reset();

}
//--------------------------------------------------------------------------
void C_VideoAgent::DrawFrame()
{
    pthread_mutex_lock(&m_NewDatalock);

    if(m_pData[0]==NULL)
    {
        pthread_mutex_unlock(&m_NewDatalock);
        return ;
    }

    if(!m_bInited)
        ClearPicure();

    _render.PlatformDisplay(m_pData,_width,_height,_i32Format);

    pthread_mutex_unlock(&m_NewDatalock);
}
//--------------------------------------------------------------------------
int C_VideoAgent::PlatformAudio(uint8_t **pData , int i32SampleCnt, int64_t i64SampleIndex)
{
    m_AudioPlayer.AddFrame(pData, i32SampleCnt,i64SampleIndex);
    return 0;
}
//--------------------------------------------------------------------------
int C_VideoAgent::PlayedSample(int64_t i64SampleIndex)
{
    m_FFMpegPlayer.PlayedSample(i64SampleIndex);
    return 0;
}
//--------------------------------------------------------------------------
int C_VideoAgent::SetAudioRate(int i32Rate)
{
    m_AudioPlayer.SetAudioRate(i32Rate);
    return 0;
}
//--------------------------------------------------------------------------
int C_VideoAgent::ClearAudioQueue()
{
    m_AudioPlayer.ClearAudioQueue();
    return 0;
}
//--------------------------------------------------------------------------
int C_VideoAgent::DoAudioRampUp()
{
    m_AudioPlayer.DoRampUp();
    return 0;
}
//--------------------------------------------------------------------------
int C_VideoAgent::DoAudioRampDown()
{
    m_AudioPlayer.DoRampDown();
    return 0;
}
//--------------------------------------------------------------------------