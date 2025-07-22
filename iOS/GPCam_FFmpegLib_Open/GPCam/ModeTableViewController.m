//
//  ModeTableViewController.m
//  GPCam
//
//  Created by generalplus_sa1 on 8/21/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#import "ModeTableViewController.h"
#import "ProtocolAgent.h"
#import "MZFormSheetController.h"

@interface ModeTableViewController ()
{
    NSArray           *_ModeArrary;
}

@end


@implementation ModeTableViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    // Uncomment the following line to preserve selection between presentations.
    // self.clearsSelectionOnViewWillAppear = NO;
    
    // Uncomment the following line to display an Edit button in the navigation bar for this view controller.
    // self.navigationItem.rightBarButtonItem = self.editButtonItem;
    
    _ModeArrary = [[NSArray alloc]initWithObjects:
                   NSLocalizedString(@"Record", @""),
                   NSLocalizedString(@"Capture", @""), nil];
    
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    // Return the number of sections.
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    // Return the number of rows in the section.
    return [_ModeArrary count];
}

//-----------------------------------------------------------------
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    
    NSInteger row = [indexPath row];
    
    // Configure the cell...
    static NSString *CellIdentifier = @"Cell";
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    
    if(cell==nil)
    {
        cell=[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1
                                    reuseIdentifier:CellIdentifier];
    }
    
    cell.textLabel.text = [_ModeArrary objectAtIndex:row];
    cell.accessoryType = UITableViewCellAccessoryNone;
    
    if(row == [_SelectIndex integerValue])
        cell.accessoryType = UITableViewCellAccessoryCheckmark;
    
    return cell;
}
//-----------------------------------------------------------------
- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath;
{
    NSInteger row = [indexPath row];
    BYTE *pbyStatus = [[ProtocolAgent GetShareAgent] GetDeviceStatus];
    if(pbyStatus[0] != row)
    {
        NSArray *cellArrary = [tableView visibleCells];
        for (UITableViewCell *object in cellArrary)
            object.accessoryType = UITableViewCellAccessoryNone;
        
        UITableViewCell *cell = [tableView cellForRowAtIndexPath:indexPath];
        cell.accessoryType = UITableViewCellAccessoryCheckmark;
        
        [[ProtocolAgent GetShareAgent]SendSetModeDirectly:row];
    }

    [[[MZFormSheetController formSheetControllersStack] lastObject] dismissAnimated:YES completionHandler:nil];
}

@end
