//
//  AppDelegate.h
//  GPCam
//
//  Created by generalplus_sa1 on 8/7/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface AppDelegate : UIResponder <UIApplicationDelegate>
{
    BOOL stdErrRedirected;
}

@property (strong, nonatomic) UIWindow *window;

@property (nonatomic, assign) BOOL stdErrRedirected;

- (void)redirectStdErrToFile;
- (void)restoreStdErr;
@end

