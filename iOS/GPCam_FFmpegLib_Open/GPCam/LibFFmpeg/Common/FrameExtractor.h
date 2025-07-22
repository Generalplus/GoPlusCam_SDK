//
//  FrameExtractor.h
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2017/11/17.
//  Copyright © 2017年 generalplus_sa1. All rights reserved.
//

#ifndef FrameExtractor_h
#define FrameExtractor_h

#include "CodecDefine.h"
#include "PictureEncoder.h"
//----------------------------------------------------------------------
class C_FrameExtractor
{
public:
    C_FrameExtractor();
    ~C_FrameExtractor();
    
    int ExtractFrame(const char* VideoPath,
                     const char* SavePath,
                     int64_t frameIdx);
    
private:
    
    bool                m_bIsExtracting;
    C_PictureEncoder    m_SnapshotEncoder;
};



#endif /* FrameExtractor_hpp */
