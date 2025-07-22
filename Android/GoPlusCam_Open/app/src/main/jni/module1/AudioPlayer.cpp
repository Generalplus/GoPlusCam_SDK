#include "AudioPlayer.h"
#include <unistd.h>

#define TX_BUFFERTIME   0.025
//--------------------------------------------------------------------------
C_AudioPlayer::C_AudioPlayer():
        C_AudioPlayerBase()

{
    m_Stream.stop = true;
}
C_AudioPlayer::~C_AudioPlayer()
{

}
//--------------------------------------------------------------------------
int C_AudioPlayer::Init(Float64 fSampleRate,int i32CHCnt)
{

    if(m_IsInited)
        Uninit();

    C_AudioPlayerBase::Init(fSampleRate,i32CHCnt);

    int i32Frame =  TX_BUFFERTIME *  m_fSampleRate;

    int i32Ret = android_OpenAudioDevice(
            fSampleRate,
            i32CHCnt,
            i32Frame);

    if(i32Ret==0)
         m_IsInited = true;
    
    return 0;
}
//--------------------------------------------------------------------------
int C_AudioPlayer::android_OpenAudioDevice(int TxSR, int outchannels, int Txbufferframes)
{

    OPENSL_STREAM *p = &m_Stream;

    p->stop = false;
    p->outchannels = outchannels;
    p->TxSR = TxSR;
    p->pCore = this;

    if((p->outBufSamples  =  Txbufferframes * outchannels * m_i32SampleBit/8 ) != 0) {
        if((p->outputBuffer[0] = (short *) calloc(p->outBufSamples, sizeof(short))) == NULL ||
           (p->outputBuffer[1] = (short *) calloc(p->outBufSamples, sizeof(short))) == NULL) {
            android_CloseAudioDevice(p);
            return 1;
        }
    }

    memset(p->outputBuffer[0],0x00,p->outBufSamples);
    memset(p->outputBuffer[1],0x00,p->outBufSamples);

    p->currentOutputIndex = 0;
    p->currentOutputBuffer  = 0;

    if(openSLCreateEngine(p) != SL_RESULT_SUCCESS) {
        android_CloseAudioDevice(p);
        return 1;
    }

    if(openSLPlayOpen(p) != SL_RESULT_SUCCESS) {
        android_CloseAudioDevice(p);
        return 1;
    }


    return 0;
}
//--------------------------------------------------------------------------
// close the android audio device
void C_AudioPlayer::android_CloseAudioDevice(OPENSL_STREAM *p){

    if (p == NULL)
        return;

    openSLDestroyEngine(p);

    if (p->outputBuffer[0] != NULL) {
        free(p->outputBuffer[0]);
        p->outputBuffer[0] = NULL;
    }

    if (p->outputBuffer[1] != NULL) {
        free(p->outputBuffer[1]);
        p->outputBuffer[1] = NULL;
    }
}
//--------------------------------------------------------------------------
// creates the OpenSL ES audio engine
SLresult C_AudioPlayer::openSLCreateEngine(OPENSL_STREAM *p)
{
    SLresult result;
    // create engine
    result = slCreateEngine(&(p->engineObject), 0, NULL, 0, NULL, NULL);
    if(result != SL_RESULT_SUCCESS) goto  engine_end;

    // realize the engine
    result = (*p->engineObject)->Realize(p->engineObject, SL_BOOLEAN_FALSE);
    if(result != SL_RESULT_SUCCESS) goto engine_end;

    // get the engine interface, which is needed in order to create other objects
    result = (*p->engineObject)->GetInterface(p->engineObject, SL_IID_ENGINE, &(p->engineEngine));
    if(result != SL_RESULT_SUCCESS) goto  engine_end;

    engine_end:
    return result;
}
//--------------------------------------------------------------------------
// opens the OpenSL ES device for output
SLresult C_AudioPlayer::openSLPlayOpen(OPENSL_STREAM *p)
{
    SLresult result;
    SLuint32 sr = p->TxSR;
    SLuint32  channels = p->outchannels;

    if(channels){
        // configure audio source
        SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};

        switch(sr){

            case 8000:
                sr = SL_SAMPLINGRATE_8;
                break;
            case 11025:
                sr = SL_SAMPLINGRATE_11_025;
                break;
            case 16000:
                sr = SL_SAMPLINGRATE_16;
                break;
            case 22050:
                sr = SL_SAMPLINGRATE_22_05;
                break;
            case 24000:
                sr = SL_SAMPLINGRATE_24;
                break;
            case 32000:
                sr = SL_SAMPLINGRATE_32;
                break;
            case 44100:
                sr = SL_SAMPLINGRATE_44_1;
                break;
            case 48000:
                sr = SL_SAMPLINGRATE_48;
                break;
            case 64000:
                sr = SL_SAMPLINGRATE_64;
                break;
            case 88200:
                sr = SL_SAMPLINGRATE_88_2;
                break;
            case 96000:
                sr = SL_SAMPLINGRATE_96;
                break;
            case 192000:
                sr = SL_SAMPLINGRATE_192;
                break;
            default:
                return -1;
        }

        const SLInterfaceID ids[] = {SL_IID_VOLUME};
        const SLboolean req[] = {SL_BOOLEAN_FALSE};
        result = (*p->engineEngine)->CreateOutputMix(p->engineEngine, &(p->outputMixObject), 1, ids, req);
        if(result != SL_RESULT_SUCCESS) return result;

        // realize the output mix
        result = (*p->outputMixObject)->Realize(p->outputMixObject, SL_BOOLEAN_FALSE);

        SLuint32 speakers;
        if(channels > 1)
            speakers = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
        else speakers = SL_SPEAKER_FRONT_CENTER;
        SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM,channels, sr,
                                       SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                       speakers, SL_BYTEORDER_LITTLEENDIAN};

        SLDataSource audioSrc = {&loc_bufq, &format_pcm};

        // configure audio sink
        SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, p->outputMixObject};
        SLDataSink audioSnk = {&loc_outmix, NULL};

        // create audio player
        const SLInterfaceID ids1[] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
        const SLboolean req1[] = {SL_BOOLEAN_TRUE};
        result = (*p->engineEngine)->CreateAudioPlayer(p->engineEngine, &(p->bqPlayerObject), &audioSrc, &audioSnk,
                                                       1, ids1, req1);
        if(result != SL_RESULT_SUCCESS) return result;

        // realize the player
        result = (*p->bqPlayerObject)->Realize(p->bqPlayerObject, SL_BOOLEAN_FALSE);
        if(result != SL_RESULT_SUCCESS) return result;

        // get the play interface
        result = (*p->bqPlayerObject)->GetInterface(p->bqPlayerObject, SL_IID_PLAY, &(p->bqPlayerPlay));
        if(result != SL_RESULT_SUCCESS) return result;

        // get the buffer queue interface
        result = (*p->bqPlayerObject)->GetInterface(p->bqPlayerObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                                                    &(p->bqPlayerBufferQueue));
        if(result != SL_RESULT_SUCCESS) return result;

        // register callback on the buffer queue
        result = (*p->bqPlayerBufferQueue)->RegisterCallback(p->bqPlayerBufferQueue, C_AudioPlayer::bqPlayerCallback, p);
        if(result != SL_RESULT_SUCCESS) return result;

        // set the player's state to playing
        result = (*p->bqPlayerPlay)->SetPlayState(p->bqPlayerPlay, SL_PLAYSTATE_PLAYING);

        return result;
    }
    return SL_RESULT_SUCCESS;
}
//--------------------------------------------------------------------------
// close the OpenSL IO and destroy the audio engine
void C_AudioPlayer::openSLDestroyEngine(OPENSL_STREAM *p){

    SLresult result;
    SLuint32 state;

    while(1)
    {
        if(p->bqPlayerPlay)
        {
            result = (*p->bqPlayerPlay)->GetPlayState(p->bqPlayerPlay, &state);
            if(state == SL_PLAYSTATE_STOPPED)
                break;

            sleep(1);
        }
        else
            break;
    }


    // destroy buffer queue audio player object, and invalidate all associated interfaces
    if (p->bqPlayerObject != NULL) {
        (*p->bqPlayerObject)->Destroy(p->bqPlayerObject);
        p->bqPlayerObject = NULL;
        p->bqPlayerPlay = NULL;
        p->bqPlayerBufferQueue = NULL;
        p->bqPlayerEffectSend = NULL;
    }

    // destroy output mix object, and invalidate all associated interfaces
    if (p->outputMixObject != NULL) {
        (*p->outputMixObject)->Destroy(p->outputMixObject);
        p->outputMixObject = NULL;
    }

    // destroy engine object, and invalidate all associated interfaces
    if (p->engineObject != NULL) {
        (*p->engineObject)->Destroy(p->engineObject);
        p->engineObject = NULL;
        p->engineEngine = NULL;
    }

}
//--------------------------------------------------------------------------
int C_AudioPlayer::Uninit()
{
    if(!m_IsInited)
        return 1;

    Stop();
    android_CloseAudioDevice(&m_Stream);
    m_IsInited = false;

    return 0;
}
//--------------------------------------------------------------------------

int C_AudioPlayer::Play()
{
    if(!m_IsInited)
        return 1;

    OPENSL_STREAM  *p = &m_Stream;

    if(p->stop)
    {
        SLresult result;
        result = (*p->bqPlayerPlay)->SetPlayState(p->bqPlayerPlay, SL_PLAYSTATE_PLAYING);
        p->stop = false;
    }

    //Send first sample
    short *outBuffer = p->outputBuffer[p->currentOutputBuffer];

    (*p->bqPlayerBufferQueue)->Enqueue(p->bqPlayerBufferQueue,
                                       outBuffer,p->outBufSamples);
    
    return 0;
}
//--------------------------------------------------------------------------
int C_AudioPlayer::Stop()
{
    if(!m_IsInited)
        return 1;

    OPENSL_STREAM  *p = &m_Stream;
    SLresult result;
    if(p->bqPlayerPlay)
        result = (*p->bqPlayerPlay)->SetPlayState(p->bqPlayerPlay, SL_PLAYSTATE_STOPPED);

    m_Stream.stop = true;
    m_AudioQueue.ClearQueue();

    FreeCoreAudioFrame(m_pCurrentFrame);
    m_pCurrentFrame = NULL;

    return 0;
}
//--------------------------------------------------------------------------
int C_AudioPlayer::Pause()
{
    if(!m_IsInited)
        return 1;

    OPENSL_STREAM  *p = &m_Stream;

    SLresult result;
    result = (*p->bqPlayerPlay)->SetPlayState(p->bqPlayerPlay, SL_PLAYSTATE_PAUSED);

    return 0;
}
//--------------------------------------------------------------------------
int C_AudioPlayer::Resume()
{
    if(!m_IsInited)
        return 1;

    OPENSL_STREAM  *p = &m_Stream;

    SLresult result;
    result = (*p->bqPlayerPlay)->SetPlayState(p->bqPlayerPlay, SL_PLAYSTATE_PLAYING);

    return 0;
}
//--------------------------------------------------------------------------
// this callback handler is called every time a buffer finishes playing
void C_AudioPlayer::bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
    OPENSL_STREAM *p = (OPENSL_STREAM *) context;
    if(!p->stop)
    {
        short *outBuffer;
        if(p == NULL)
            return ;

        int bufsamps = p->outBufSamples / ((p->pCore->m_i32SampleBit/8) * p->outchannels) ;
        outBuffer = p->outputBuffer[p->currentOutputBuffer];
        memset(outBuffer, 0x00, p->outBufSamples);

        p->pCore->FillInFrame(outBuffer ,bufsamps);

        (*p->bqPlayerBufferQueue)->Enqueue(p->bqPlayerBufferQueue,
                                           outBuffer,p->outBufSamples);
        p->currentOutputIndex = 0;
    }
}
//--------------------------------------------------------------------------

