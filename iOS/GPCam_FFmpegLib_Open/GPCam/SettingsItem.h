//
//  Settingsitem.h
//  GPCam
//
//  Created by generalplus_sa1 on 8/14/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface SettingsItem : NSObject

@property (nonatomic, strong) NSString       *Name;
@property unsigned int ID;
@property int Type;
@property int Current;
@property int ReFlash;
@property (nonatomic, strong) NSString       *ValueString;
@property (nonatomic, strong) NSMutableArray *Values;

@end
