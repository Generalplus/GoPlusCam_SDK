//
//  LogViewController.m
//  GPCam
//
//  Created by generalplus_sa1 on 8/21/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#import "LogViewController.h"
#import "LogDataHelper.h"
#import "MZFormSheetController.h"

@interface LogViewController ()


@property (retain, nonatomic) IBOutlet UITextView *LogView;
@end

@implementation LogViewController

//----------------------------------------------------------------------
-(void)SaveLog:(id)sender
{
    [[LogDataHelper GetShareHelper] SaveToFile];
}
//----------------------------------------------------------------------
-(void)selectDownAction:(id)sender
{
    //[self dismissViewControllerAnimated:YES completion:nil];
    [[[MZFormSheetController formSheetControllersStack] lastObject] dismissAnimated:YES completionHandler:nil];
}
//----------------------------------------------------------------------
- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    UIBarButtonItem *DownBtn = [[UIBarButtonItem alloc]initWithTitle:NSLocalizedString(@"Done", @"")
                                                               style:UIBarButtonItemStyleDone
                                                              target:self
                                                              action:@selector(selectDownAction:)];
    
    UIBarButtonItem *SaveBtn = [[UIBarButtonItem alloc]initWithTitle:NSLocalizedString(@"Save", @"")
                                                               style:UIBarButtonItemStyleDone
                                                              target:self
                                                              action:@selector(SaveLog:)];
    
    self.navigationItem.leftBarButtonItem = DownBtn;
    self.navigationItem.rightBarButtonItem = SaveBtn;
    
    _LogView.text = [LogDataHelper GetShareHelper].m_pLogString;
}
//----------------------------------------------------------------------
- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}
//----------------------------------------------------------------------
/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

@end
