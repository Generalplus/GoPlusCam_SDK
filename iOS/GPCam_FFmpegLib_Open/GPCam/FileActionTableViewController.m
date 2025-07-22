//
//  FileActionTableViewController.m
//  GPCam
//
//  Created by generalplus_sa1 on 9/15/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#import "FileActionTableViewController.h"
#import "MZFormSheetController.h"
#import "ProtocolAgent.h"

@interface FileActionTableViewController ()
{
    NSArray           *_ModeArrary;
}

@end


@implementation FileActionTableViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    // Uncomment the following line to preserve selection between presentations.
    // self.clearsSelectionOnViewWillAppear = NO;
    
    // Uncomment the following line to display an Edit button in the navigation bar for this view controller.
    // self.navigationItem.rightBarButtonItem = self.editButtonItem;
    
    _ModeArrary = [[NSArray alloc]initWithObjects:
                   NSLocalizedString(@"Play", @""),
                   NSLocalizedString(@"Download", @""),
                   NSLocalizedString(@"Info", @""),
                   NSLocalizedString(@"Delete", @""),
                   nil];
    
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
    
    bool bCanSelect = true;
    
    if([_FileDownloaded boolValue] && row == FILEACTION_DOWNLOAD)
        bCanSelect = false;
    else if(![_FileDownloaded boolValue] && row == FILEACTION_DELETE)
        bCanSelect = false;
    
//    if ([ProtocolAgent GetShareAgent].m_iRtsp == 1 && row == FILEACTION_PLAY && ![_FileDownloaded boolValue]) {
//        bCanSelect = false;
//    }
    
    if ([ProtocolAgent GetShareAgent].m_bCanDeleteSDFile && row == FILEACTION_DELETE) {
        bCanSelect = true;
    }
    
    if(!bCanSelect)
    {
        cell.selectionStyle = UITableViewCellSelectionStyleNone;
        cell.userInteractionEnabled = false;
    }
    
    return cell;
}
//-----------------------------------------------------------------
- (void)tableView:(UITableView *)tableView willDisplayCell:(UITableViewCell *)cell forRowAtIndexPath:(NSIndexPath *)indexPath
{
    if(!cell.userInteractionEnabled)
        cell.alpha = 0.3;
}
//-----------------------------------------------------------------
- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath;
{
    NSInteger row = [indexPath row];
    
    if(_Delegate)
        [self.Delegate SelectAction:(int)row];
    
    if((int)row == FILEACTION_INFO)
    {
        // XCode 7.0 will hang on here.... Maybe is MZFormSheetController bug?
        //[self performSegueWithIdentifier:@"Show_FileDetail" sender:self];
        BYTE abyDate[6];
        unsigned int i32FileSize = [[ProtocolAgent GetShareAgent]GetFileSize: [_SelectIndex intValue]];
        [[ProtocolAgent GetShareAgent] GetFileTime: [_SelectIndex intValue] withTimeBuffer:abyDate];
        unsigned int i32ExtraSize=0;
        BYTE *pbyExtra = [[ProtocolAgent GetShareAgent] GetFileExtraInfo: [_SelectIndex intValue] withSizeReadBack:&i32ExtraSize];
        
        unsigned long lExtraInfo = 0;
        
        for(int i=0;i<i32ExtraSize && i<4;i++)
            lExtraInfo += (pbyExtra[i]>>i);

        
        NSMutableString *ExtraString = [NSMutableString stringWithFormat:@""];
//        if(i32ExtraSize>0)
//            ExtraString = [NSMutableString stringWithFormat:@"\nExtra Info: 0x%lx",lExtraInfo];
        
    
        NSString *DetailString = [NSString stringWithFormat:@"%@: %.02d/%.02d/%.02d %.02d:%.02d:%.02d\n%@: %u KB%@",NSLocalizedString(@"Time", @""),
                                  abyDate[0] + 2000 ,abyDate[1],abyDate[2],abyDate[3],abyDate[4],abyDate[5],NSLocalizedString(@"Size", @""),i32FileSize,ExtraString];
        
        UIAlertController *alertController = [UIAlertController
                                              alertControllerWithTitle:[[ProtocolAgent GetShareAgent]GetFileName:[_SelectIndex intValue]]
                                              message:DetailString
                                              preferredStyle:UIAlertControllerStyleAlert];
        
        UIAlertAction *okAction = [UIAlertAction
                                   actionWithTitle:NSLocalizedString(@"OK", @"OK action")
                                   style:UIAlertActionStyleDefault
                                   handler:^(UIAlertAction *action)
                                   {
                                       
                                   }];
        
        [alertController addAction:okAction];
        [self presentViewController:alertController animated:YES completion:nil];
        
        
    }
    else
        [[[MZFormSheetController formSheetControllersStack] lastObject] dismissAnimated:YES completionHandler:nil];

}
//-----------------------------------------------------------------
-(void) prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    
    BYTE abyDate[6];
    unsigned int i32FileSize = [[ProtocolAgent GetShareAgent]GetFileSize: [_SelectIndex intValue]];
    [[ProtocolAgent GetShareAgent] GetFileTime: [_SelectIndex intValue] withTimeBuffer:abyDate];
    
    NSString *DetailString = [NSString stringWithFormat:@"%@: %.02d/%.02d/%.02d %.02d:%.02d:%.02d\n%@: %d KB",NSLocalizedString(@"Time", @""),
                              abyDate[0] + 2000 ,abyDate[1],abyDate[2],abyDate[3],abyDate[4],abyDate[5],NSLocalizedString(@"Size", @""),i32FileSize];
    
    [segue.destinationViewController setValue:  [[ProtocolAgent GetShareAgent]GetFileName:[_SelectIndex intValue]]  forKey:@"Title"];
    [segue.destinationViewController setValue:  DetailString forKey:@"content"];
}

@end

