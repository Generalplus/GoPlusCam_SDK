//
//  SettingTableViewController.m
//  GPCam
//
//  Created by generalplus_sa1 on 8/14/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#import "SettingTableViewController.h"
#import "ProtocolAgent.h"
#import "MBProgressHUD.h"

#import "MZFormSheetController.h"
#import "MZCustomTransition.h"
#import "MZFormSheetSegue.h"
#import "ValueTableViewController.h"
#import "LanguageManager.h"

@interface SettingTableViewController () <MZFormSheetBackgroundWindowDelegate>
{
    NSInteger       _SelValueIndex;
    SettingsItem    *_pSelSettingItem;
    UITableViewCell *_SelCell;
    BOOL            m_bGetSettingComplete;
}

@property (retain, nonatomic) IBOutlet UIBarButtonItem         *FWUpgradeButton;

@end

@implementation SettingTableViewController

//----------------------------------------------------------------------
-(void)selectDownAction:(id)sender
{
    if (m_bGetSettingComplete) {
        [self dismissViewControllerAnimated:YES completion:nil];
    }
}
//----------------------------------------------------------------------
- (void)viewDidLoad {
    [super viewDidLoad];
    
    if([[ProtocolAgent GetShareAgent] GetSettingFileParser].bIsDefault)
    {
        self.navigationItem.title =  [NSString stringWithFormat:@"%@*",self.navigationItem.title];
    }
    
    UIBarButtonItem *DownBtn = [[UIBarButtonItem alloc]initWithTitle:NSLocalizedString(@"Done", @"")
                                                               style:UIBarButtonItemStyleDone
                                                              target:self
                                                              action:@selector(selectDownAction:)];
    
    self.navigationItem.leftBarButtonItem = DownBtn;
    [[MZFormSheetBackgroundWindow appearance] setBackgroundColor:[UIColor clearColor]]; //There has a bug when landscape mode
    
    if ([ProtocolAgent GetShareAgent].m_iRtsp == 0) {
        if([[ProtocolAgent GetShareAgent] GetSettingFileParser].FWVersion <= FW_Old_Number)
            self.navigationItem.rightBarButtonItem = nil;
    }
    m_bGetSettingComplete = NO;
}
//-----------------------------------------------------------------
- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
    [ProtocolAgent GetShareAgent].ActiveView = self.view;
    [[ProtocolAgent GetShareAgent] SendSetMode:E_DeviceMode_Menu];
    [[ProtocolAgent GetShareAgent] SendGetDeviceSettings];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(SettingChange:)
                                                 name:Notification_Device_SettingChange
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(ConnectionChangeNotification:)
                                                 name:Notification_Device_ConnectionChange
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(GetSettingComplete:)
                                                 name:Notification_Device_GetSettingComplete
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                            selector:@selector(CommandFailed:)
                                                  name:Notification_Device_CommandFailed
                                                object:nil];
    
}
//-----------------------------------------------------------------
- (void)viewWillDisappear:(BOOL)animated
{
    [super viewWillDisappear:animated];
 
    
    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:Notification_Device_SettingChange
                                                  object:nil];
    
    
    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:Notification_Device_ConnectionChange
                                                  object:nil];
    
    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:Notification_Device_GetSettingComplete
                                                  object:nil];
    
    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:Notification_Device_CommandFailed
                                                  object:nil];
    
}
//-----------------------------------------------------------------
- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}
//------------------------------------------------------------------------------------------------
- (void) ConnectionChangeNotification:(NSNotification*) notification
{
    NSString *Status = [notification object];
    
    if([Status isEqualToString:@"DisConnected"])
    {
        dispatch_async(dispatch_get_main_queue(), ^{

            [self performSegueWithIdentifier:@"BackToEntry" sender:self];
        });
    }
    
}
//------------------------------------------------------------------------------------------------
- (void) SettingChange:(NSNotification*) notification
{
    dispatch_async(dispatch_get_main_queue(), ^{
        [self.tableView reloadData];
    });
    
}
//------------------------------------------------------------------------------------------------
- (void) GetSettingComplete:(NSNotification*) notification
{
    m_bGetSettingComplete = YES;
}
#pragma mark - Table view Section header
//-----------------------------------------------------------------
- (CGFloat)tableView:(UITableView *)tableView heightForHeaderInSection:(NSInteger)section
{
    return tableView.sectionHeaderHeight * 2;
}
//-----------------------------------------------------------------
- (void)tableView:(UITableView *)tableView willDisplayHeaderView:(UIView *)view forSection:(NSInteger)section
{
    UITableViewHeaderFooterView *tableViewHeaderFooterView = (UITableViewHeaderFooterView *)view;
    tableViewHeaderFooterView.textLabel.font = [UIFont boldSystemFontOfSize:18];
    tableViewHeaderFooterView.textLabel.textColor = [UIColor colorWithRed:0.27f green:0.27f blue:0.27f alpha:1.0f];
    tableViewHeaderFooterView.contentView.backgroundColor = [UIColor colorWithRed:0.85f green:0.86f blue:0.89f alpha:0.3f];
    
    tableViewHeaderFooterView.contentView.layer.masksToBounds = true;
    tableViewHeaderFooterView.contentView.layer.borderColor = [[UIColor colorWithRed:0.85f green:0.86f blue:0.89f alpha:1.0] CGColor];
    tableViewHeaderFooterView.contentView.layer.borderWidth = 0.5;
}
#pragma mark - Table view data source
//-----------------------------------------------------------------
- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {

    // Return the number of sections.
    return [[[ProtocolAgent GetShareAgent] GetSettingFileParser].Categories count];
}
//-----------------------------------------------------------------
- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    
    // Return the number of rows in the section.
    CategoriesItem *Category = [[[ProtocolAgent GetShareAgent] GetSettingFileParser].Categories objectAtIndex:section];
    return [Category.Settings count];
}
//-----------------------------------------------------------------
- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section;
{
    CategoriesItem *Category = [[[ProtocolAgent GetShareAgent] GetSettingFileParser].Categories objectAtIndex:section];
    return SettingLocalizedString(Category.Name, @"");
}
//-----------------------------------------------------------------
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    
    NSInteger section = [indexPath section];
    CategoriesItem *Category = [[[ProtocolAgent GetShareAgent] GetSettingFileParser].Categories objectAtIndex:section];
    NSInteger row = [indexPath row];
    SettingsItem *Setting = [Category.Settings objectAtIndex:row];
    
    // Configure the cell...
    static NSString *CellIdentifier = @"Cell";
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    
    if(cell==nil)
    {
        cell=[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1
                                    reuseIdentifier:CellIdentifier];
    }
    
    cell.detailTextLabel.text = @"";
    cell.textLabel.text = SettingLocalizedString(Setting.Name, @"");
    cell.accessoryType = UITableViewCellAccessoryNone;
    
    if([Setting.Values count]>0)
    {
        if(Setting.Current < [Setting.Values count])
        {
            ValuesItem *Value = [Setting.Values objectAtIndex:Setting.Current];
            if(Value)
                cell.detailTextLabel.text = SettingLocalizedString(Value.Name, @"");
        }
    }
    else if (Setting.Type == SETTING_TYPE_INPUTSTRING)
        cell.detailTextLabel.text = SettingLocalizedString(Setting.ValueString, @"");
    
    if(Setting.Type == SETTING_TYPE_VALUES || Setting.Type == SETTING_TYPE_INPUTSTRING)
        cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
    
    return cell;
}
//-----------------------------------------------------------------
- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath;
{
    NSInteger section = [indexPath section];
    CategoriesItem *Category = [[[ProtocolAgent GetShareAgent] GetSettingFileParser].Categories objectAtIndex:section];
    NSInteger row = [indexPath row];
    SettingsItem *Setting = [Category.Settings objectAtIndex:row];
    
    _SelValueIndex = Setting.Current;
    _pSelSettingItem  = Setting;
    _SelCell = [tableView cellForRowAtIndexPath:indexPath];
    
    
    switch(Setting.Type)
    {
        case SETTING_TYPE_VALUES:
        {

            [self CreatePopOutView:@"ValueNC"
             WithCompletionHandler:^(UIViewController *presentedFSViewController) {
                 // Passing data
                 UINavigationController *navController = (UINavigationController *)presentedFSViewController;
                 navController.topViewController.title = SettingLocalizedString(Setting.Name, @"");
                 
                 [[navController topViewController] setValue:[NSNumber numberWithInt:(int)_SelValueIndex] forKey:@"SelectIndex"];
                 [[navController topViewController] setValue:_pSelSettingItem forKey:@"Setting"];
                 [[navController topViewController] setValue:_SelCell forKey:@"SelCell"];
             }];
            
        }
            break;
        case SETTING_TYPE_ACTION:
        case SETTING_TYPE_SMARTACTION:
        {
            
            UIAlertController *alertController = [UIAlertController
                                                  alertControllerWithTitle:NSLocalizedString(@"Perform action", @"")
                                                  message:[NSString stringWithFormat:@"%@ \n\"%@\"?",NSLocalizedString(@"Are you sure to do", @""),SettingLocalizedString(Setting.Name, @"")]
                                                  preferredStyle:UIAlertControllerStyleAlert];
            
            UIAlertAction *NoAction = [UIAlertAction
                                       actionWithTitle:NSLocalizedString(@"Cancel", @"Cancel action")
                                       style:UIAlertActionStyleDefault
                                       handler:^(UIAlertAction *action)
                                       {
                                           
                                       }];
            
            UIAlertAction *YesAction = [UIAlertAction
                                        actionWithTitle:NSLocalizedString(@"Confirm", @"Confirm action")
                                        style:UIAlertActionStyleDefault
                                        handler:^(UIAlertAction *action)
                                        {
                                            NSLog(@"Confirm Pressed");
                                            if(_pSelSettingItem.Type == SETTING_TYPE_ACTION)
                                            {
                                                [[ProtocolAgent GetShareAgent]SendSetParameter:_pSelSettingItem.ID
                                                                                      withSize:0x00
                                                                                      withData:NULL];
                                                
                                                if(_pSelSettingItem.ReFlash == Reflash_All_Setting)
                                                    [[ProtocolAgent GetShareAgent] SendGetDeviceSettings];
                                            }
                                            else if(_pSelSettingItem.Type == SETTING_TYPE_SMARTACTION)
                                            {
                                                //action on smart device
                                                if( _pSelSettingItem.ID == ClearBuff_ID)
                                                    [self DoClearBuffer];
                                            }
                                            
                                        }];
            
            [alertController addAction:NoAction];
            [alertController addAction:YesAction];
            [self presentViewController:alertController animated:YES completion:nil];
            
            
            
        }
            break;
            
        case SETTING_TYPE_INPUTSTRING:
        {
            [self CreatePopOutView:@"StringValueNC"
             WithCompletionHandler:^(UIViewController *presentedFSViewController) {
                 // Passing data
                 UINavigationController *navController = (UINavigationController *)presentedFSViewController;
                 navController.topViewController.title = SettingLocalizedString(Setting.Name, @"");
                 
                 [[navController topViewController] setValue:[NSNumber numberWithInt:(int)_SelValueIndex] forKey:@"SelectIndex"];
                 [[navController topViewController] setValue:_pSelSettingItem forKey:@"Setting"];
                 [[navController topViewController] setValue:_SelCell forKey:@"SelCell"];
             }];
            
        }
            break;
            
        case SETTING_TYPE_DISPLAY:
        {

        }
            break;
    }
}
//-----------------------------------------------------------------
-(void)CreatePopOutView:(NSString *)StoryboardName
  WithCompletionHandler:(MZFormSheetCompletionHandler) Handler
{
    [[MZFormSheetBackgroundWindow appearance] setBackgroundColor:[UIColor clearColor]]; //There has a bug when landscape mode
    UIViewController *vc = [self.storyboard instantiateViewControllerWithIdentifier:StoryboardName];
    
    MZFormSheetController *formSheet = [[MZFormSheetController alloc] initWithViewController:vc];
    
    formSheet.presentedFormSheetSize = CGSizeMake(300, 298);
    formSheet.shadowRadius = 2.0;
    formSheet.shadowOpacity = 0.3;
    formSheet.shouldDismissOnBackgroundViewTap = YES;
    formSheet.shouldCenterVertically = YES;
    formSheet.movementWhenKeyboardAppears = MZFormSheetWhenKeyboardAppearsCenterVertically;
    // formSheet.keyboardMovementStyle = MZFormSheetKeyboardMovementStyleMoveToTop;
    // formSheet.keyboardMovementStyle = MZFormSheetKeyboardMovementStyleMoveToTopInset;
    // formSheet.landscapeTopInset = 50;
    // formSheet.portraitTopInset = 100;
    
    
    formSheet.willPresentCompletionHandler = Handler;
    
    formSheet.transitionStyle = MZFormSheetTransitionStyleFade;
    
    [MZFormSheetController sharedBackgroundWindow].formSheetBackgroundWindowDelegate = self;
    
    [self mz_presentFormSheetController:formSheet animated:YES completionHandler:^(MZFormSheetController *formSheetController) {

    }];
}
//-----------------------------------------------------------------
-(void) DoClearBuffer
{
    printf("Do Clear buffer \n");
    
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0];
    NSArray *directoryContent = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:documentsDirectory error:NULL];
    NSError *error;
    
    
    for (NSString* item in directoryContent){
        NSString *filePath = [documentsDirectory stringByAppendingPathComponent:item];
        NSURL *fileURL = [[NSURL alloc] initFileURLWithPath:filePath];
        NSLog(@"Delete: %@",filePath);
        [[NSFileManager defaultManager] removeItemAtURL:fileURL error:&error];
        
    }

}

//----------------------------------------------------------------------
- (void) CommandFailed:(NSNotification*) notification
{ 
    NSArray *PassObjs = [notification object];
    NSNumber *FailedIndex = [PassObjs objectAtIndex:0];
    NSNumber *ErrorCode = [PassObjs objectAtIndex:1];
    
    int i32Index = [FailedIndex intValue];
    
    printf("Handle command failed, Index: %d Error: %d\n",i32Index,[ErrorCode intValue]);
}


/*- (BOOL)prefersStatusBarHidden {
    
    return true;
}*/
@end
