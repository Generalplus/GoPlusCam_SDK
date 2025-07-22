//
//  FileInfoHandler.h
//  GPCam
//
//  Created by generalplus_sa1 on 9/14/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "FileInfoItem.h"

#define CHECK_RET_NOFOUND    0
#define CHECK_RET_MATCH      1
#define CHECK_RET_MISMATCH   2

@interface FileInfoHandler : NSObject

-(bool) ReadFile;
-(bool) SaveFile;

-(bool) AddItem:(FileInfoItem*)item;
-(int ) CheckAndReplaceItem:(FileInfoItem*)item;
-(NSURL*) GetFileAssertUrl:(NSString*) FileName;

@end
