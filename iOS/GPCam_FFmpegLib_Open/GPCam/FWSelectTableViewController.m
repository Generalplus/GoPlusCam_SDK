//
//  FWSelectTableViewController.m
//  GoPlus Cam
//
//  Created by generalplus_sa1 on 2/23/16.
//  Copyright © 2016 generalplus_sa1. All rights reserved.
//

#import "FWSelectTableViewController.h"
#import "ProtocolAgent.h"
//-----------------------------------------------------------------
#define DEVICE_FW_HEADER  "SPII"
#define DEVICE_FW_HEADER2 "PGps"

//-----------------------------------------------------------------
@interface FWSelectTableViewController ()<FirmwareDownloadDelegate>
{
    NSMutableArray  *m_DocFolderContents;
    NSString        *pSelectionFile;
    FILE            *_fp;
    int             _i32LastCommandIndex;
    int             _ui32FileSize;
    int             _ui32SendSize;
    bool            _bIsFinish;
}

@end
//-----------------------------------------------------------------
@implementation FWSelectTableViewController

//-----------------------------------------------------------------
- (void)viewDidLoad {
    [super viewDidLoad];
    
    // Uncomment the following line to preserve selection between presentations.
    // self.clearsSelectionOnViewWillAppear = NO;
    
    // Uncomment the following line to display an Edit button in the navigation bar for this view controller.
    // self.navigationItem.rightBarButtonItem = self.editButtonItem;
    
    NSMutableArray *FolderContents = [[[NSFileManager defaultManager] contentsOfDirectoryAtPath:[NSHomeDirectory() stringByAppendingPathComponent:@"Documents"]
                                                                                          error:nil] mutableCopy];
    
    m_DocFolderContents = [[NSMutableArray alloc]init];
    NSString *docsPath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) lastObject];
    
    BOOL isDir = NO;
    for (NSString *FileName in FolderContents)
    {
        isDir = NO;
        NSString *filePath = [docsPath stringByAppendingPathComponent:FileName];
        if ([[NSFileManager defaultManager] fileExistsAtPath:filePath isDirectory:&isDir] && isDir)
            continue;
        
        [m_DocFolderContents addObject:FileName];
    }
    
    _fp = NULL;
    _i32LastCommandIndex = -1;
}

//------------------------- ----------------------------------------
- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
    [ProtocolAgent GetShareAgent].ActiveView = self.view;
    [ProtocolAgent GetShareAgent].FirmwareDownloadDelegate = self;
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(ConnectionChangeNotification:)
                                                 name:Notification_Device_ConnectionChange
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
                                                    name:Notification_Device_ConnectionChange
                                                  object:nil];
    
    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:Notification_Device_CommandFailed
                                                  object:nil];
    if(_fp)
        fclose(_fp);
    
    [[ProtocolAgent GetShareAgent] ClearCmdQueue];
    [ProtocolAgent GetShareAgent].FirmwareDownloadDelegate = nil;
}
//----------------------------------------------------------------------
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
//-----------------------------------------------------------------
- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}
//-----------------------------------------------------------------
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
//-----------------------------------------------------------------

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return 1;
}
//-----------------------------------------------------------------
- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return [m_DocFolderContents count];
}
//-----------------------------------------------------------------
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    static NSString *CellIdentifier = @"Cell";
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    
    // Configure the cell...
    cell=[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault
                                reuseIdentifier:CellIdentifier];
    
    cell.textLabel.text = [m_DocFolderContents objectAtIndex:[indexPath row]];
    return cell;
}
//-----------------------------------------------------------------
- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    UITableViewCell *cell = [tableView cellForRowAtIndexPath:indexPath];
    pSelectionFile = cell.textLabel.text;
    
    UIAlertController *alertController = [UIAlertController
                                          alertControllerWithTitle:NSLocalizedString(@"Upgrade Firmware", @"")
                                          message:[NSString localizedStringWithFormat:NSLocalizedString(@"Are you sure to select \"%@\" to upgrade firmware?",@""),pSelectionFile]
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
                                    [self StartUpgradeFW];
                                }];
    
    [alertController addAction:NoAction];
    [alertController addAction:YesAction];
    [self presentViewController:alertController animated:YES completion:nil];
    
    
}
//-----------------------------------------------------------------
- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section;
{
    return NSLocalizedString(@"Select file to upgrade device FW", @"");
}
//-----------------------------------------------------------------
-(void)ShowAlert:(NSString *)Title
         Message:(NSString *)Message
{
    UIAlertController *alertController = [UIAlertController
                                          alertControllerWithTitle:Title
                                          message:Message
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
//----------------------------------------------------------------
- (void) StartUpgradeFW
{
    NSString *docsPath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) lastObject];
    NSString *filePath = [docsPath stringByAppendingPathComponent:pSelectionFile];

    unsigned int ui32Checksum = 0;
    unsigned int ui32FileSize = 0;

    _bIsFinish = false;
    
    if(_fp)
    {
        fclose(_fp);
        _fp = NULL;
    }
    
    _fp = fopen([filePath UTF8String] ,"rb");
    
    fseek(_fp, 0, SEEK_END);
    ui32FileSize = (unsigned int)ftell(_fp);
    fseek(_fp, 0, SEEK_SET);
    
    //verify header
    BYTE abyFWData[512] = {0};
    int i32Read = (int)fread(abyFWData,1,512,_fp);
    
    if(memcmp(abyFWData,DEVICE_FW_HEADER,strlen(DEVICE_FW_HEADER)) !=0 && memcmp(abyFWData,DEVICE_FW_HEADER2,strlen(DEVICE_FW_HEADER2)) !=0)
    {
        [self ShowAlert:NSLocalizedString(@"Upgrade Firmware failed", @"")
                Message:[NSString localizedStringWithFormat:NSLocalizedString(@"File \"%@\" is incorrect firmware file", @""), pSelectionFile]];
        
        return;
    }
    
    fseek(_fp, 0, SEEK_SET);

    //checksum
    while(!feof(_fp))
    {
        i32Read = (int)fread(abyFWData,1,512,_fp);
        for(int i=0;i<i32Read;i++)
            ui32Checksum+=abyFWData[i];
    }
    
    fseek(_fp, 0, SEEK_SET);
    
    //Start upgrade
    _ui32FileSize = ui32FileSize;
    _ui32SendSize = 0;
    _i32LastCommandIndex = [[ProtocolAgent GetShareAgent] SendFirmwareDownload:ui32FileSize
                                                                  withCheckSum:ui32Checksum];
}
//----------------------------------------------------------------------
- (void) CommandFailed:(NSNotification*) notification
{
    NSArray *PassObjs = [notification object];
    NSNumber *FailedIndex = [PassObjs objectAtIndex:0];
    NSNumber *ErrorCode = [PassObjs objectAtIndex:1];
    
    int i32Index = [FailedIndex intValue];
    
    printf("Handle command failed, Index: %d Error: %d\n",i32Index,[ErrorCode intValue]);
    
    if(i32Index == _i32LastCommandIndex)
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            [self ShowAlert:NSLocalizedString(@"Upgrade Firmware failed", @"")
                    Message:[NSString stringWithFormat:@"%@",ErrorCodeString[abs([ErrorCode intValue])-1]]];
        });
 
    }
}
//----------------------------------------------------------------
-(void) SendRawData
{
    if(!feof(_fp))
    {
        BYTE abyFWData[0x0400] = {0};
        int i32Read = (int)fread(abyFWData,1,0x0400,_fp);
        _i32LastCommandIndex = [[ProtocolAgent GetShareAgent] SendFirmwareRawData:i32Read withData:abyFWData];
        [ProtocolAgent GetShareAgent].progress = (float) _ui32SendSize / (float)_ui32FileSize;
        _ui32SendSize+=i32Read;
    }
    else
    {
        if(!_bIsFinish)
        {
            _i32LastCommandIndex = [[ProtocolAgent GetShareAgent] SendFirmwareRawData:0 withData:NULL];
            _bIsFinish = true;
        }
        else
            _i32LastCommandIndex = [[ProtocolAgent GetShareAgent] SendFirmwareUpgrade];
        
        [ProtocolAgent GetShareAgent].progress = 1.0;
    }
}

//----------------------------------------------------------------
//FirmwareDownloadDelegate
-(void) DownloadComplete
{
    [self SendRawData];
}
//----------------------------------------------------------------
-(void) RawDataComplete
{
    [self SendRawData];
}
//----------------------------------------------------------------
-(void) UpgradeComplete
{
    dispatch_async(dispatch_get_main_queue(), ^{
        UIAlertController *alertController = [UIAlertController
                                              alertControllerWithTitle:NSLocalizedString(@"Upgrade Firmware successfully", @"")
                                              message:NSLocalizedString(@"Please reboot the Wifi sport Cam and APP.", @"")
                                              preferredStyle:UIAlertControllerStyleAlert];
        
        UIAlertAction *okAction = [UIAlertAction
                                   actionWithTitle:NSLocalizedString(@"OK", @"OK action")
                                   style:UIAlertActionStyleDefault
                                   handler:^(UIAlertAction *action)
                                   {
                                       exit(0);
                                   }];
        
        [alertController addAction:okAction];
        [self presentViewController:alertController animated:YES completion:nil];
    });
    
}
//----------------------------------------------------------------
@end
