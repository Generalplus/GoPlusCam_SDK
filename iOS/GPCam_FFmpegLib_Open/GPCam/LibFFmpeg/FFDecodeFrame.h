//
//  FFDecodeFrame.h
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2017/9/4.
//  Copyright © 2017年 generalplus_sa1. All rights reserved.
//

#import <Foundation/Foundation.h>
//---------------------------------------------------------------------------
#define FFDECODE_FORMAT_YUV420P                 0
#define FFDECODE_FORMAT_YUV422P                 1
#define FFDECODE_FORMAT_YUV444P                 2
#define FFDECODE_FORMAT_YUVJ420P                3
#define FFDECODE_FORMAT_YUVJ422P                4
#define FFDECODE_FORMAT_YUVJ444P                5
//---------------------------------------------------------------------------
@interface FFDecodeFrame : NSObject
{
@public
    uint8_t     *data[8];
    int         linesize[8];
    int         width;
    int         height;
    int         format;
}
//---------------------------------------------------------------------------

@end
