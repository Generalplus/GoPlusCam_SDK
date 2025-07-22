//
//  GLView
//
//
//  Created by generalplus_sa1 on 7/11/16.
//  Copyright © 2016 generalplus_sa1. All rights reserved.
//


#import <UIKit/UIKit.h>

@interface GLView : UIView

- (id) initWithFrame:(CGRect)frame
           scaleMode:(int) i32mode;
- (void)freeContext;
-(void) PlatformDisplay:(uint8_t *[])pData
                  width:(int)i32width
                 height:(int)i32height
                 format:(int)i32format;

-(void) LoadBufferStorage;
-(void) Reset;

-(void) SetZoomInRatio:(float)fRatio;

/* Size of video frame */
@property (nonatomic) int sourceWidth, sourceHeight;
@end
