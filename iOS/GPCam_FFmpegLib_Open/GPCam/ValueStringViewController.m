//
//  ValueStringViewController.m
//  GPCam
//
//  Created by generalplus_sa1 on 8/20/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#import "ValueStringViewController.h"
#import "MZFormSheetController.h"
#import "ProtocolAgent.h"

@interface ValueStringViewController ()<UITextFieldDelegate>


@property (nonatomic, strong) IBOutlet UITextField   *InputField;

@end
//--------------------------------------------------------------------
@implementation ValueStringViewController
//--------------------------------------------------------------------
- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    _InputField.text = _Setting.ValueString;
    _InputField.delegate = self;
}
//--------------------------------------------------------------------
- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}
//--------------------------------------------------------------------
-(IBAction)PressCancel:(id)sender
{
    [[[MZFormSheetController formSheetControllersStack] lastObject] dismissAnimated:YES completionHandler:nil];
}
//--------------------------------------------------------------------
-(IBAction)PressConfirm:(id)sender
{
    [[[MZFormSheetController formSheetControllersStack] lastObject] dismissAnimated:YES completionHandler:nil];
     _Setting.ValueString = _InputField.text;
     _SelCell.detailTextLabel.text = _Setting.ValueString;
    
    NSData *data = [_Setting.ValueString dataUsingEncoding:NSUTF8StringEncoding];
    int i32Len = [data length];
    
    BYTE byStringData[i32Len];
    memset(byStringData,0x00,sizeof(byStringData));
    
    memcpy(byStringData,[data bytes],i32Len);
    
    [[ProtocolAgent GetShareAgent]SendSetParameter:_Setting.ID
                                          withSize:i32Len
                                          withData:byStringData];
    
}
//--------------------------------------------------------------------
- (BOOL) textField: (UITextField *)theTextField shouldChangeCharactersInRange:(NSRange)range replacementString: (NSString *)string
{
    // allow backspace
    if (!string.length)
    {
        return YES;
    }
    
    NSString *updatedText = [theTextField.text stringByReplacingCharactersInRange:range withString:string];
    
    if([[ProtocolAgent GetShareAgent] GetSettingFileParser].FWVersion < FW_SupportPWlength) {
        if (updatedText.length > 8) // Only 8 char
        {
            return NO;
        }
    }
    
    NSString* resultString = [theTextField.text stringByReplacingCharactersInRange: range
                                                                     withString: string];
    NSString *regExPattern = @"[a-zA-Z0-9]*";
    BOOL bIsInputValid = [[NSPredicate predicateWithFormat:@"SELF MATCHES %@", regExPattern] evaluateWithObject: resultString];
    return bIsInputValid;
}


/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

@end
