//
//  Settingsitem.m
//  GPCam
//
//  Created by generalplus_sa1 on 8/14/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#import "SettingsItem.h"

@implementation SettingsItem
//------------------------------------------------------------------------------------------------
-(id)init
{
    if ((self = [super init])) {
        self.Values = [[NSMutableArray alloc]init];
        self.ReFlash = 0;
    }
    return self;
}
//------------------------------------------------------------------------------------------------
@end
