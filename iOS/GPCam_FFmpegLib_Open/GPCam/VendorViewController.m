//
//  VendorViewController.m
//  GPCam
//
//  Created by generalplus_sa1 on 9/8/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#import "VendorViewController.h"
#import "MRHexKeyboard.h"
#import "ProtocolAgent.h"
#import "MZFormSheetController.h"

//------------------------------------------------------------------------------------
@implementation NSString (NSStringHexToBytes)
- (NSData *)dataFromHexString {
    const char *chars = [self UTF8String];
    int i = 0, len = (int)self.length;
    
    NSMutableData *data = [NSMutableData dataWithCapacity:len / 2];
    char byteChars[3] = {'\0','\0','\0'};
    unsigned long wholeByte;
    
    while (i < len)
    {
        if(chars[i] == '0' && (chars[i+1] == 'x' || chars[i+1] == 'X'))
        {
            i+=2;
            continue;
        }
        else if(chars[i] == ' ')
        {
            i+=1;
            continue;
        }
        
        byteChars[0] = chars[i++];
        byteChars[1] = chars[i++];
        wholeByte = strtoul(byteChars, NULL, 16);
        [data appendBytes:&wholeByte length:1];
    }
    
    return data;
}

@end
//------------------------------------------------------------------------------------
@interface VendorViewController ()<VendorCommandDelegate>

@property (retain, nonatomic) IBOutlet UITextField *SendTextField;
@property (retain, nonatomic) IBOutlet UITextField *GetTextField;
@end

@implementation VendorViewController
//----------------------------------------------------------------------
-(void) GetData:(BYTE*)pbyData
       withSize:(int)i32Size
{
    NSMutableString *GetString = [NSMutableString stringWithFormat:@""];
    
    for(int i=0;i<i32Size;i++)
        [GetString appendFormat:@"0x%x ",pbyData[i]];
    
    dispatch_async(dispatch_get_main_queue(), ^{
        
        _GetTextField.text = GetString;
    });
}
//----------------------------------------------------------------------
-(IBAction)PressSend:(id)sender
{
    NSData *pData = [_SendTextField.text dataFromHexString];
    const unsigned char *dataBytes = [pData bytes];
    [[ProtocolAgent GetShareAgent]SendVendorCmd:(unsigned char *)dataBytes
                                       withSize:(int)[pData length]];
    
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
    self.navigationItem.leftBarButtonItem = DownBtn;
    
    _SendTextField.inputView = [[MRHexKeyboard alloc] initWithTextField:_SendTextField];
    
}
//----------------------------------------------------------------------
- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}
//----------------------------------------------------------------------
- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
    [ProtocolAgent GetShareAgent].ActiveView = self.view;
    [ProtocolAgent GetShareAgent].VendorDelegate = self;
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
