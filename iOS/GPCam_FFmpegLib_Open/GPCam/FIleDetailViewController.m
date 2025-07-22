//
//  FIleDetailViewController.m
//  GPCam
//
//  Created by generalplus_sa1 on 8/28/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#import "FIleDetailViewController.h"
#import "MZFormSheetController.h"

@interface FIleDetailViewController ()

@property (retain, nonatomic) IBOutlet UITextView *DetailView;

@end

@implementation FIleDetailViewController



//----------------------------------------------------------------------
-(void)selectDownAction:(id)sender
{
    [[[MZFormSheetController formSheetControllersStack] lastObject] dismissAnimated:YES completionHandler:nil];
}
//----------------------------------------------------------------------
- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    
   // UIBarButtonItem *DownBtn = [[UIBarButtonItem alloc]initWithBarButtonSystemItem:UIBarButtonSystemItemDone
   //                                                                         target:self
   //                                                                         action:@selector(selectDownAction:)];
    //self.navigationItem.leftBarButtonItem = DownBtn;
}
//----------------------------------------------------------------------
- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

//----------------------------------------------------------------------
- (void) setContent:(NSString *)content
{
    _content = content;
}
//-----------------------------------------------------------------
- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
    _DetailView.text = _content;
    [_DetailView setFont:[UIFont boldSystemFontOfSize:17]];
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
