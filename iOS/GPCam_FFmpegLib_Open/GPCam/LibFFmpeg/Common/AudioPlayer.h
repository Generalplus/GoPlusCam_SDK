#ifndef CoreAudioPlayer_h
#define CoreAudioPlayer_h

#include <stdio.h>
#include <pthread.h>
#include <vector>
#include "Defines.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <pthread.h>
#include <stdlib.h>

#include "AudioPlayerBase.h"

//----------------------------------------------------------------------
class C_AudioPlayer;
typedef struct opensl_stream {

    // engine interfaces
    SLObjectItf engineObject;
    SLEngineItf engineEngine;

    // output mix interfaces
    SLObjectItf outputMixObject;

    // buffer queue player interfaces
    SLObjectItf bqPlayerObject;
    SLPlayItf bqPlayerPlay;
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
    SLEffectSendItf bqPlayerEffectSend;

    // buffer indexes
    int currentOutputIndex;

    // current buffer half (0, 1)
    int currentOutputBuffer;

    // buffers
    short *outputBuffer[2];

    // size of buffers
    int outBufSamples;

    int outchannels;
    int TxSR;

    bool stop;
    C_AudioPlayer *pCore;

} OPENSL_STREAM;

//----------------------------------------------------------------------
class C_AudioPlayer : public C_AudioPlayerBase
{
    
public:
    
    C_AudioPlayer();
    ~C_AudioPlayer();

    int     Init(Float64 fSampleRate,int i32CHCnt);
    int     Uninit();


    int     Play();
    int     Stop();
    int     Pause();
    int     Resume();

private:

    int android_OpenAudioDevice(int TxSR, int outchannels, int Txbufferframes);

    void android_CloseAudioDevice(OPENSL_STREAM *p);
    SLresult openSLCreateEngine(OPENSL_STREAM *p);
    SLresult openSLPlayOpen(OPENSL_STREAM *p);
    void openSLDestroyEngine(OPENSL_STREAM *p);

    static void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context);

    OPENSL_STREAM               m_Stream;
};


#endif /* CoreAudioPlayer_h */
