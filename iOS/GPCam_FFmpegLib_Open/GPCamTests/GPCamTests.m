//
//  GPCamTests.m
//  GPCamTests
//
//  Created by generalplus_sa1 on 8/7/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <XCTest/XCTest.h>
#import "MainViewController.h"

@interface GPCamTests : XCTestCase

@property (nonatomic, strong) MainViewController *vc;

@end

@implementation GPCamTests

- (void)setUp {
    [super setUp];
    // Put setup code here. This method is called before the invocation of each test method in the class.
    UIStoryboard *storyboard = [UIStoryboard storyboardWithName:@"Main" bundle:nil];
    self.vc = [storyboard instantiateViewControllerWithIdentifier:@"VLCMainView"];
    [self.vc performSelectorOnMainThread:@selector(loadView) withObject:nil waitUntilDone:YES];
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void)testExample {
    // This is an example of a functional test case.
    
    //[self.vc performSelectorOnMainThread:@selector(PressConnect:) withObject:self.vc  waitUntilDone:YES];
    XCTAssert(YES, @"Pass");
}

- (void)testPerformanceExample {
    // This is an example of a performance test case.
    [self measureBlock:^{
        // Put the code you want to measure the time of here.
    }];
}

@end
