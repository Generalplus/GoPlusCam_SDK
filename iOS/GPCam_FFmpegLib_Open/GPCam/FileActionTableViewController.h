//
//  FileActionTableViewController.h
//  GPCam
//
//  Created by generalplus_sa1 on 9/15/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#import <UIKit/UIKit.h>

#define FILEACTION_PLAY     0
#define FILEACTION_DOWNLOAD 1
#define FILEACTION_INFO     2
#define FILEACTION_DELETE   3

@protocol FileActionDelegate

-(void) SelectAction:(int)i32Action;

@end

@interface FileActionTableViewController : UITableViewController

@property (nonatomic, strong) NSNumber    *SelectIndex;
@property (nonatomic, strong) NSNumber    *FileDownloaded;
@property(strong) id<FileActionDelegate>  Delegate;
@end
