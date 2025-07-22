//
// Created by generalplus_sa1 on 4/29/16.
//

#ifndef FFMPEGTEST_DEFINES_H
#define FFMPEGTEST_DEFINES_H

#if defined(ANDROID) || defined(__ANDROID__)
    #include <android/log.h>
    #define DEVICE_PRINT(...) __android_log_print(ANDROID_LOG_INFO  , "ffmpegJNI",__VA_ARGS__)
#else
    #define DEVICE_PRINT  printf
#endif

#if defined(DEBUG)

    #if defined(ANDROID) || defined(__ANDROID__)
        #include <android/log.h>
        #define DEBUG_PRINT(...) __android_log_print(ANDROID_LOG_ERROR  , "ffmpegJNI",__VA_ARGS__)
    #else
        #define DEBUG_PRINT(format, ...)  printf( \
        "%*.*s(%.5d): " \
        format, 40, 40 , __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__ \
        )
    #endif

#else
    #define DEBUG_PRINT(format, ...)  ((void)0)
#endif

//----------------------------------------------------------------------
#define FFMPEGPLAYER_NOERROR                     0
#define FFMPEGPLAYER_INITMEDIAFAILED             1
#define FFMPEGPLAYER_MEDIAISPLAYING              2
#define FFMPEGPLAYER_CREATESAVESTREAMFAILED      3
#define FFMPEGPLAYER_SAVESNAPSHOTFAILED          4
#define FFMPEGPLAYER_SAVEVIDEOFAILED             5

//----------------------------------------------------------------
class I_FFmpegAgnet
{
public:
    
    //Init complete
    virtual int InitComplete(int i32Result) = 0;
    
    //Core to GLView
    virtual int PlatformDisplay(uint8_t *pData[], int width, int height , int format , int lineSizes[4]) = 0;
    
    //Core to Audio player
    virtual int PlatformAudio(uint8_t **pData , int i32SampleCnt, int64_t i64SampleIndex) = 0;
    virtual int SetAudioRate(int i32Rate) = 0;
    virtual int ClearAudioQueue() = 0;
    virtual int DoAudioRampUp() = 0;
    virtual int DoAudioRampDown() = 0;
    
    //Save video and snapshot
    virtual int SaveSanpshotComplete() = 0;
    virtual int SaveVideoComplete() = 0;
    
    virtual int StartBuffering() = 0;
    virtual int ContinuePlaying() = 0;
};
//----------------------------------------------------------------
class I_FFmpegAudioAgnet
{
public:
    //Audio player to Core
    virtual int PlayedSample(int64_t i64SampleIndex) = 0;
};

//----------------------------------------------------------------
class I_GLViewAgent
{
public:
    virtual void LoadGLBuffer()=0;
};
//----------------------------------------------------------------

#endif //FFMPEGTEST_DEFINES_H
