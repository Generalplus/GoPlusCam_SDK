//
//  FFDecodeFrame.m
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2017/9/4.
//  Copyright © 2017年 generalplus_sa1. All rights reserved.
//

#import "FFDecodeFrame.h"
//---------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
    
#include <libavcodec/avcodec.h>
    
#ifdef __cplusplus
}
#endif

//---------------------------------------------------------------------------
@interface FFDecodeFrame()
{

}
@end
//---------------------------------------------------------------------------
@implementation FFDecodeFrame
//---------------------------------------------------------------------------
- (id) init
{
    if ((self = [super init]))
    {
        for(int i=0;i<8;i++)
        {
            data[i] = NULL;
            linesize[i] = 0;
        }
        
        width = 0;
        height = 0;
        format = 0;
    }
    return self;
}
//---------------------------------------------------------------------------
-(void) dealloc
{
    av_freep(&data[0]);
}
//---------------------------------------------------------------------------


@end
