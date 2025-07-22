//
//  LogDataHelper.m
//  HeartRate
//
//  Created by generalplus_sa1 on 12/8/14.
//  Copyright (c) 2014 generalplus_sa1. All rights reserved.
//

#import "LogDataHelper.h"

#define MAX_LOG_INDEX   200
//------------------------------------------------------------------------------------
@interface LogDataHelper ()
{
    int i32LogIndex;
    NSDateFormatter* dateFormatter;
}
@end

@implementation LogDataHelper
@synthesize m_pLogString;
//------------------------------------------------------------------------------------
-(id)init
{
    if(self = [super init])
    {
        m_pLogString = [[NSMutableString alloc]initWithFormat:@""];
        i32LogIndex = 0;
    }
    
    return self;
}

static id sharedInstance;
//------------------------------------------------------------------------------------------------
+ (void) initialize {
    // subclassing would result in an instance per class, probably not what we want
    NSAssert([LogDataHelper class] == self, @"Subclassing is not welcome");
    sharedInstance = [[super alloc] init];
}
//------------------------------------------------------------------------------------
+(LogDataHelper*)GetShareHelper
{
    return sharedInstance;
}
//------------------------------------------------------------------------------------
-(void) ResetLog
{
    i32LogIndex = 0;
    [m_pLogString setString:@""];
}
//------------------------------------------------------------------------------------
-(void)AppendTime
{
    i32LogIndex++;
    if(i32LogIndex%MAX_LOG_INDEX==0)
        [m_pLogString setString:@""];

    [m_pLogString appendFormat:@"[%.08d]",i32LogIndex];
    
    if (dateFormatter == nil) {
        dateFormatter = [[NSDateFormatter alloc] init];
        [dateFormatter setDateFormat:@"[hh:mm:ss SSS]"];
    }
    
    NSString *dateString = [dateFormatter stringFromDate:[NSDate date]];
    [m_pLogString appendString:dateString];
}
//------------------------------------------------------------------------------------
-(void) LogMessage:(NSString*)Msg
{
    BOOL bLogEnable = false;
    id LogObject  = [[NSUserDefaults standardUserDefaults] objectForKey:@"log_enable"];
    if(LogObject)
        bLogEnable = [[NSUserDefaults standardUserDefaults] boolForKey:@"log_enable"];
    
    if(!bLogEnable)
        return;
    
    [self AppendTime];
    [m_pLogString appendString:Msg];
    [m_pLogString appendFormat:@"\n"];
}
//------------------------------------------------------------------------------------
-(void) LogMessage:(NSString*)HeaderMsg
      WithIntArray:(BYTE*)pbyData
        WithLength:(int)i32Length
{
    
    BOOL bLogEnable = false;
    id LogObject  = [[NSUserDefaults standardUserDefaults] objectForKey:@"log_enable"];
    if(LogObject)
        bLogEnable = [[NSUserDefaults standardUserDefaults] boolForKey:@"log_enable"];
    
    if(!bLogEnable)
        return;
    
    [self AppendTime];
    [m_pLogString appendString:HeaderMsg];
    [m_pLogString appendFormat:@": %d ,",i32Length];

    BOOL bEnable = false;
    id FullObject  = [[NSUserDefaults standardUserDefaults] objectForKey:@"fulllog_enable"];
    if(FullObject)
        bEnable = [[NSUserDefaults standardUserDefaults] boolForKey:@"fulllog_enable"];
    
    int i32Len = i32Length;
    if(!bEnable)
    {
        i32Len = 20;
        if(i32Length<i32Len)
            i32Len = i32Length;
        
        if(i32Len>0)
            [m_pLogString appendFormat:@"Byte[0] ~ [%d]\n",i32Len-1];
    }
    
    for (int idx = 0; idx < i32Len; ++idx)
    {
        if(idx%16 == 0 && bEnable)
            [m_pLogString appendFormat:@"\n"];
        
       [m_pLogString appendFormat:@"%02X ", pbyData[idx]];
    }
    
    [m_pLogString appendFormat:@"\n"];
}
//------------------------------------------------------------------------------------
-(NSMutableString *) m_pLogString
{
    return m_pLogString;
}
//------------------------------------------------------------------------------------
-(void) SaveToFile
{
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0]; // Get documents directory
    
    NSError *error;
    BOOL succeed = [m_pLogString writeToFile:[documentsDirectory stringByAppendingPathComponent:@"CmdLog.txt"]
                                  atomically:YES encoding:NSUTF8StringEncoding error:&error];
    if (!succeed){
        // Handle error here
    }
}
//------------------------------------------------------------------------------------
@end
