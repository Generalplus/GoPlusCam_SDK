//
//  main.m
//  GPCam
//
//  Created by generalplus_sa1 on 8/7/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "AppDelegate.h"
#import "LanguageManager.h"

int main(int argc, char * argv[]) {
    @autoreleasepool {
        
#ifdef ChangeLanguageDynamically
        [LanguageManager setupCurrentLanguage];
#endif
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}
