//
//  CoreAudioPlayer.h
//  ffmpegTest
//
//  Created by generalplus_sa1 on 6/29/16.
//  Copyright © 2016 generalplus_sa1. All rights reserved.
//

#ifndef CoreAudioPlayer_h
#define CoreAudioPlayer_h

#include <AudioToolbox/AudioToolbox.h>
#include <AudioUnit/AudioUnit.h>
#include <CoreAudio/CoreAudioTypes.h>
#include "Defines.h"
#include "Common/AudioPlayerBase.h"
//----------------------------------------------------------------------
class C_CoreAudioPlayer : public C_AudioPlayerBase
{
    
public:
    
    C_CoreAudioPlayer();
    ~C_CoreAudioPlayer();
    
    int     Init(Float64 fSampleRate,int i32CHCnt);
    int     Uninit();
    
    
    int     Play();
    int     Stop();
    int     Pause();
    int     Resume();
    
private:
    
    static OSStatus PlaybackCallback(
                                     void *inRefCon,
                                     AudioUnitRenderActionFlags *ioActionFlags,
                                     const AudioTimeStamp *inTimeStamp,
                                     UInt32 inBusNumber,
                                     UInt32 inNumberFrames, 
                                     AudioBufferList *ioData
                                     );
    
    
    AUGraph                     m_processingGraph;
    AudioUnit                   m_ioUnit;
    AudioUnit                   m_mixerUnit;
    AudioUnit                   m_ConverterUnit;
    
};


#endif /* CoreAudioPlayer_h */
