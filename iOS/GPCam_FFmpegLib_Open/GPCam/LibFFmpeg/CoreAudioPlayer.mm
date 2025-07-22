//
//  CoreAudioPlayer.cpp
//  ffmpegTest
//
//  Created by generalplus_sa1 on 6/29/16.
//  Copyright © 2016 generalplus_sa1. All rights reserved.
//

#include "CoreAudioPlayer.h"
#include "Defines.h"
#include <math.h>

#define kOutputBus	0
#define kInputBus	1


#define kIOUnitOutputBus	0
#define kIOUnitInputBus	    1
#define kMixerUnitInputBus	0
//--------------------------------------------------------------------------
C_CoreAudioPlayer::C_CoreAudioPlayer():
C_AudioPlayerBase()
{
}
C_CoreAudioPlayer::~C_CoreAudioPlayer()
{

}
//--------------------------------------------------------------------------
int C_CoreAudioPlayer::Init(Float64 fSampleRate,int i32CHCnt)
{
    if(m_IsInited)
        Uninit();

    C_AudioPlayerBase::Init(fSampleRate,i32CHCnt);
    
    //------------------------------
    //  Instantiate an audio processing graph
    //------------------------------
    OSStatus result = NewAUGraph (&m_processingGraph);
    if(result != noErr)
    {
        NSLog(@"Unable to create an AUGraph object. Error code: %d '%.4s'", (int) result, (const char *)&result);
        return -1;
    }
    
    //------------------------------
    // Create AU Nodes
    //------------------------------
    AUNode ioNode, mixerNode, converterNode;
    
    AudioComponentDescription cd = {};
    cd.componentManufacturer     = kAudioUnitManufacturer_Apple;
    
    //Output Node
    cd.componentType = kAudioUnitType_Output;
    cd.componentSubType = kAudioUnitSubType_RemoteIO;            // Output to speakers
    
    result = AUGraphAddNode (m_processingGraph, &cd, &ioNode);
    if(result != noErr)
    {
        NSLog(@"Unable to add the Output Node to the audio processing graph. Error code: %d '%.4s'", (int) result, (const char *)&result);
        return -1;
    }
    
    //Mixer Node
    cd.componentType = kAudioUnitType_Mixer;                     // Mixer
    cd.componentSubType = kAudioUnitSubType_MultiChannelMixer;   // sub type - MultiChannel Mixer
    
    result = AUGraphAddNode (m_processingGraph, &cd, &mixerNode);
    if(result != noErr)
    {
        NSLog(@"Unable to add the Mixer Node to the audio processing graph. Error code: %d '%.4s'", (int) result, (const char *)&result);
        return -1;
    }
    
    //Converter Node
    cd.componentType = kAudioUnitType_FormatConverter;           // Converter
    cd.componentSubType = kAudioUnitSubType_AUConverter;         // pcm converter, converter unit can change sample rate
    
    result = AUGraphAddNode (m_processingGraph, &cd, &converterNode);
    if(result != noErr)
    {
        NSLog( @"Unable to add the Mixer Node to the audio processing graph. Error code: %d '%.4s'", (int) result, (const char *)&result);
        return -1;
    }
    
    //------------------------------
    // Open the graph
    //------------------------------
    result = AUGraphOpen (m_processingGraph);
    if(result != noErr)
    {
        NSLog( @"Unable to open the audio processing graph. Error code: %d '%.4s'", (int) result, (const char *)&result);
        return -1;
    }
    
    //------------------------------
    // Set Mixer Unit Input number and Obtain mixer Unit
    //------------------------------
    result = AUGraphNodeInfo (m_processingGraph, mixerNode, 0, &m_mixerUnit);
    if(result != noErr)
    {
        NSLog( @"Unable to obtain a reference to the Mixer unit. Error code: %d '%.4s'", (int) result, (const char *)&result);
        return -1;
    }
    
    UInt32 numbuses = 1;
    AudioUnitSetProperty(m_mixerUnit,kAudioUnitProperty_ElementCount, kAudioUnitScope_Input, 0, &numbuses,sizeof(numbuses));
    
    float volume = 1.0f;
    AudioUnitSetParameter(m_mixerUnit, kMultiChannelMixerParam_Volume, kAudioUnitScope_Input, kMixerUnitInputBus, volume, 0 ) ;
    
    
    //------------------------------
    // Set Converter Unit Input number and Obtain Converter Unit
    //------------------------------
    result = AUGraphNodeInfo (m_processingGraph, converterNode, 0, &m_ConverterUnit);
    if(result != noErr)
    {
        NSLog( @"Unable to obtain a reference to the Converter unit. Error code: %d '%.4s'", (int) result, (const char *)&result);
        return -1;
    }
    
    //------------------------------
    // Connect input call back to the Converter Node
    //------------------------------
    
    AudioStreamBasicDescription StreamFormat;
    
    StreamFormat.mSampleRate		= m_fSampleRate;
    StreamFormat.mFormatID			= kAudioFormatLinearPCM;
    StreamFormat.mFormatFlags		= kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
    StreamFormat.mFramesPerPacket	= 1;
    StreamFormat.mChannelsPerFrame	= i32CHCnt;
    StreamFormat.mBitsPerChannel	= m_i32SampleBit;
    StreamFormat.mBytesPerPacket	= (m_i32SampleBit/8) * i32CHCnt;
    StreamFormat.mBytesPerFrame		= (m_i32SampleBit/8) * i32CHCnt;
    
    result = AudioUnitSetProperty (m_ConverterUnit,kAudioUnitProperty_StreamFormat,kAudioUnitScope_Input,kMixerUnitInputBus,&StreamFormat,sizeof (StreamFormat));
    
    // Setup the struture that contains the input render callback
    AURenderCallbackStruct inputCallbackStruct;
    inputCallbackStruct.inputProc        = &PlaybackCallback;
    inputCallbackStruct.inputProcRefCon  = this;
    
    result = AUGraphSetNodeInputCallback ( m_processingGraph, converterNode, 0 ,&inputCallbackStruct);
    
    //------------------------------
    // Connect the Converter Node to the Mixer Node
    //------------------------------
    result = AUGraphConnectNodeInput (m_processingGraph, converterNode, 0, mixerNode, 0);
    if(result != noErr)
    {
        NSLog(@"Unable to interconnect the Converter nodes in the audio processing graph. Error code: %d '%.4s'", (int) result, (const char *)&result);
        return -1;
    }
    
    //------------------------------
    // Connect the Mixer Node to the output Node
    //------------------------------
    
    result = AUGraphConnectNodeInput (m_processingGraph, mixerNode, 0, ioNode, 0);
    if(result != noErr)
    {
        NSLog(@"Unable to interconnect the Mixer nodes in the audio processing graph. Error code: %d '%.4s'", (int) result, (const char *)&result);
        return -1;
    }
    
    //------------------------------
    // AUGraph Initialize 
    //------------------------------
    result = AUGraphInitialize (m_processingGraph);
    if(result != noErr)
    {
        NSLog( @"Unable to initialze AUGraph object. Error code: %d '%.4s'", (int) result, (const char *)&result);
        return -1;
    }
    
#if defined(DEBUG)
    CAShow(m_processingGraph);
#endif
    
    m_IsInited = true;
    
    return 0;
}
//--------------------------------------------------------------------------
int C_CoreAudioPlayer::Uninit()
{
    Stop();
    AUGraphUninitialize(m_processingGraph);
    m_IsInited = false;
    
    return 0;
}
//--------------------------------------------------------------------------

int C_CoreAudioPlayer::Play()
{
    
    Init(m_fSampleRate,m_i32CHCnt);
    m_i32PlayRate = 1;
  	OSStatus status =  AUGraphStart (m_processingGraph);
    return 0;
}
//--------------------------------------------------------------------------
int C_CoreAudioPlayer::Stop()
{
    if(!m_IsInited)
        return 1;

	OSStatus status = AUGraphStop (m_processingGraph);
    m_AudioQueue.ClearQueue();
    
    FreeCoreAudioFrame(m_pCurrentFrame);
    m_pCurrentFrame = NULL;

    return 0;
}
//--------------------------------------------------------------------------
int C_CoreAudioPlayer::Pause()
{
    float volume = 0.0f;
    AudioUnitSetParameter(m_mixerUnit, kMultiChannelMixerParam_Volume, kAudioUnitScope_Input, kMixerUnitInputBus, volume, 0 ) ;
    
    return 0;
}
//--------------------------------------------------------------------------
int C_CoreAudioPlayer::Resume()
{
    float volume = 1.0f;
    AudioUnitSetParameter(m_mixerUnit, kMultiChannelMixerParam_Volume, kAudioUnitScope_Input, kMixerUnitInputBus, volume, 0 ) ;
    
    return 0;
}
//--------------------------------------------------------------------------
OSStatus C_CoreAudioPlayer::PlaybackCallback(
                                 void *inRefCon,
                                 AudioUnitRenderActionFlags *ioActionFlags,
                                 const AudioTimeStamp *inTimeStamp,
                                 UInt32 inBusNumber,
                                 UInt32 inNumberFrames,
                                 AudioBufferList *ioData
                                 )
{
    // Notes: ioData contains buffers (may be more than one!)
    // Fill them up as much as you can. Remember to set the size value in each buffer to match how
    // much data is in the buffer
    C_CoreAudioPlayer *pAudioAPI = (C_CoreAudioPlayer*)inRefCon;
    
    for (int i=0; i < ioData->mNumberBuffers; i++)
    {
        AudioBuffer buffer = ioData->mBuffers[i];
        memset(buffer.mData, 0x00, inNumberFrames * 2);
    }
    
    
    AudioBuffer buffer = ioData->mBuffers[0];
    short* pbuffer= (short*)buffer.mData;
    pAudioAPI->FillInFrame(pbuffer,inNumberFrames);

    return noErr;

}
//--------------------------------------------------------------------------
