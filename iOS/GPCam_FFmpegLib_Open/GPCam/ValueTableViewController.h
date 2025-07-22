//
//  ValueTableViewController.h
//  GPCam
//
//  Created by generalplus_sa1 on 8/14/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "SettingFileParser.h"

@interface ValueTableViewController : UITableViewController

@property (nonatomic, strong) SettingsItem      *Setting;
@property (nonatomic, strong) NSNumber          *SelectIndex;
@property (nonatomic, strong) UITableViewCell   *SelCell;

@end  
