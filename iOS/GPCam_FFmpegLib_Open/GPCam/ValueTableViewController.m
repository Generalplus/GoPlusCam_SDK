//
//  ValueTableViewController.m
//  GPCam
//
//  Created by generalplus_sa1 on 8/14/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#import "ValueTableViewController.h"
#import "ProtocolAgent.h"
#import "MZFormSheetController.h"
#import "LanguageManager.h"

@interface ValueTableViewController ()

@end

@implementation ValueTableViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    // Uncomment the following line to preserve selection between presentations.
    // self.clearsSelectionOnViewWillAppear = NO;
    
    // Uncomment the following line to display an Edit button in the navigation bar for this view controller.
    // self.navigationItem.rightBarButtonItem = self.editButtonItem;
}
//------------------------- ----------------------------------------
- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
}
//------------------------- ----------------------------------------
- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark - Table view data source
//-----------------------------------------------------------------
- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    // Return the number of sections.
    return 1;
}
//-----------------------------------------------------------------
- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    // Return the number of rows in the section.

    return [_Setting.Values count];
}

//-----------------------------------------------------------------
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    
    NSInteger row = [indexPath row];
    ValuesItem *Value = [_Setting.Values  objectAtIndex:row];
    
    // Configure the cell...
    static NSString *CellIdentifier = @"Cell";
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    
    if(cell==nil)
    {
        cell=[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1
                                    reuseIdentifier:CellIdentifier];
    }
    
    cell.textLabel.text = SettingLocalizedString(Value.Name, @"");
    cell.accessoryType = UITableViewCellAccessoryNone;
    
    if(row == [_SelectIndex integerValue])
        cell.accessoryType = UITableViewCellAccessoryCheckmark;
    
    return cell;
}
//-----------------------------------------------------------------
- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath;
{
    NSArray *cellArrary = [tableView visibleCells];
    for (UITableViewCell *object in cellArrary)
        object.accessoryType = UITableViewCellAccessoryNone;

    UITableViewCell *cell = [tableView cellForRowAtIndexPath:indexPath];
    cell.accessoryType = UITableViewCellAccessoryCheckmark;
    
    NSInteger row = [indexPath row];
    ValuesItem *Value = [_Setting.Values  objectAtIndex:row];
    _Setting.Current = row;
    _SelCell.detailTextLabel.text = SettingLocalizedString(Value.Name, @"");
    
    BYTE byCurrent = _Setting.Current;
    [[ProtocolAgent GetShareAgent]SendSetParameter:_Setting.ID
                                          withSize:1
                                          withData:&byCurrent];
    
    [self CheckID:_Setting.ID Value:(int)byCurrent];
    
    [[[MZFormSheetController formSheetControllersStack] lastObject] dismissAnimated:YES completionHandler:nil];
    
}
//-----------------------------------------------------------------
-(void) CheckID:(int) i32ID
          Value:(int) i32Value;
{
    
    if(i32ID == Language_Setting_ID)
    {
        printf("Language_Setting_ID %d = %d\n",i32ID,i32Value);
        
    }
}
//-----------------------------------------------------------------
/*- (BOOL)prefersStatusBarHidden {
    
    return true;
}*/
@end
