//
//  FileInfoItem.h
//  GPCam
//
//  Created by generalplus_sa1 on 9/14/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface FileInfoItem : NSObject

@property (nonatomic, strong) NSString *FileName;
@property (nonatomic, strong) NSURL    *assertURL;

@property unsigned long lDate;
@property unsigned long lTime;
@property unsigned long lSize;

@end
