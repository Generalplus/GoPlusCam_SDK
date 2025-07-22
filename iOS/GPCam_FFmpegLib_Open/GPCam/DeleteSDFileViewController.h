//
//  DeleteSDFileViewController.h
//  GoPlus Cam
//
//  Created by Robin on 2017/8/16.
//  Copyright © 2017年 generalplus_sa1. All rights reserved.
//

#import <UIKit/UIKit.h>


@protocol DeleteSDFileDelegate

-(void) DeleteAction:(BOOL)bDeleteSDFile;

@end

@interface DeleteSDFileViewController : UIViewController <UITableViewDelegate,UITableViewDataSource>

@property(strong) id<DeleteSDFileDelegate>  Delegate;
@property (nonatomic, strong) NSNumber    *bFileExist;
@end
