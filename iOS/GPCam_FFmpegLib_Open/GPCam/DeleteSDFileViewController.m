    //
//  DeleteSDFileViewController.m
//  GoPlus Cam
//
//  Created by Robin on 2017/8/16.
//  Copyright © 2017年 generalplus_sa1. All rights reserved.
//

#import "DeleteSDFileViewController.h"
#import "MZFormSheetController.h"
@interface DeleteSDFileViewController ()
{
    NSMutableArray          *_ModeArrary;
}

@property (weak, nonatomic) IBOutlet UITableView *m_tableView;

@end

@implementation DeleteSDFileViewController

static bool g_bDefaultDeleteSDFile = false;
- (void)viewDidLoad {
    [super viewDidLoad];
    
    _ModeArrary = [[NSMutableArray alloc]initWithObjects:
                   NSLocalizedString(@"Delete Wifi sport Cam file", @""),
                   nil];
    
    if ([_bFileExist boolValue]) {
        [_ModeArrary addObject:NSLocalizedString(@"Delete phone file", @"")];
    }
    else {
        g_bDefaultDeleteSDFile = true;
    }
    
    _m_tableView.delegate = self;
    _m_tableView.dataSource = self;
}


-(IBAction)PressCancel:(id)sender
{
    [[[MZFormSheetController formSheetControllersStack] lastObject] dismissAnimated:YES completionHandler:nil];
}
//--------------------------------------------------------------------
-(IBAction)PressConfirm:(id)sender
{
    [[[MZFormSheetController formSheetControllersStack] lastObject] dismissAnimated:YES completionHandler:nil];
    if(_Delegate)
        [self.Delegate DeleteAction:g_bDefaultDeleteSDFile];
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    // Return the number of sections.
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    // Return the number of rows in the section.
    if (nil == _ModeArrary) {
        return 0;
    }
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
    
    if(1 == row)
    {
        cell.accessoryType = UITableViewCellAccessoryCheckmark;
        cell.userInteractionEnabled = false;
    }
    else if(0 == row){
        if (false == [_bFileExist boolValue]) {
            cell.accessoryType = UITableViewCellAccessoryCheckmark;
            cell.userInteractionEnabled = false;
        }
        else {
            if (g_bDefaultDeleteSDFile) {
                cell.accessoryType = UITableViewCellAccessoryCheckmark;
            }
            else {
                cell.accessoryType = UITableViewCellAccessoryNone;
            }
        }
    }
    
    return cell;
}
//-----------------------------------------------------------------
- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath;
{
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
    UITableViewCell *cell = [tableView cellForRowAtIndexPath:indexPath];
    if (cell.selectionStyle != UITableViewCellSelectionStyleNone) {
        if (UITableViewCellAccessoryCheckmark == cell.accessoryType) {
            cell.accessoryType = UITableViewCellAccessoryNone;
            g_bDefaultDeleteSDFile = false;
        }
        else {
            cell.accessoryType = UITableViewCellAccessoryCheckmark;
            g_bDefaultDeleteSDFile = true;
        }
    }
}



@end
