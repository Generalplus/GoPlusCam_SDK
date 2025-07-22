//
//  LogDataHelper.h
//  HeartRate
//
//  Created by generalplus_sa1 on 12/8/14.
//  Copyright (c) 2014 generalplus_sa1. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "GPCamTypedef.h"
@interface LogDataHelper : NSObject

+(LogDataHelper*)GetShareHelper;

-(void) ResetLog;

-(void) LogMessage:(NSString*)Msg;

-(void) LogMessage:(NSString*)HeaderMsg
      WithIntArray:(BYTE*)pbyData
        WithLength:(int)i32Length;

-(void) SaveToFile;

@property (readonly) NSMutableString *m_pLogString;
@end
