//
//  SettingFileParser.h
//  GPCam
//
//  Created by generalplus_sa1 on 8/13/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "Categoriesitem.h"
#import "Settingsitem.h"
#import "Valuesitem.h"

#define  SETTING_TYPE_VALUES          0
#define  SETTING_TYPE_ACTION          1
#define  SETTING_TYPE_INPUTSTRING     2
#define  SETTING_TYPE_DISPLAY         3
#define  SETTING_TYPE_SMARTACTION     4

//Item Index
#define RecordLoopRecording_Setting_ID              0x00000003
#define RecordResolution_Setting_ID                 0x00000000
#define CaptureResolution_Setting_ID                0x00000100
#define Version_Setting_ID                          0x00000209
#define Version_Value_Index                         0
#define Language_Setting_ID                         0x00000203

//Smart device action
#define ClearBuff_ID                  0x00000206
#define Reflash_All_Setting           0x01

#define FRAME_SYNC_TIME               10
#define FRAME_SYNC_RATE               25

//Device FW
#define FW_Old_Number                 0x01000000   // V1.0.0.0
#define FW_SupportPWlength            0x02000000   // V2.0.0.0
//--------------------------------------------------------------
@interface SettingFileParser : NSObject

-(bool) Parsefile:(NSString *)filePath;

-(int)  GetValueFromSettingFile:(unsigned int)i32SettingID;

-(NSString*) GetValueStrFromSettingFile:(unsigned int)i32SettingID
                             ValueIndex:(int)i32Value;


@property (nonatomic, strong) NSString       *Version;
@property (nonatomic, strong) NSMutableArray *Categories;
@property (readonly) unsigned int FWVersion;
@property bool                bIsDefault;
@end
