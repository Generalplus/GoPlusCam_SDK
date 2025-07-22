//
//  FileListCollectionViewController.m
//  GPCam
//
//  Created by generalplus_sa1 on 8/24/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#import "FileListCollectionViewController.h"
#import <AssetsLibrary/AssetsLibrary.h>
#import "FilePhotoCollectionViewCell.h"
#import "ProtocolAgent.h"
#import "MZFormSheetController.h"
#import "MZCustomTransition.h"
#import "MZFormSheetSegue.h"
#import "FileThumbnailItem.h"
#import "FileInfoHandler.h"
#import "FileActionTableViewController.h"
#import "ALAssetsLibrary+CustomPhotoAlbum.h"
#import <Photos/Photos.h>

typedef enum
{
    E_ThumbnailAction_Poll,
    E_ThumbnailAction_Recover,
    E_ThumbnailAction_Start,
    E_ThumbnailAction_Idle,
    E_ThumbnailAction_Finish,
    
}E_ThumbnailAction;

#include <pthread.h>
#include <sys/time.h>

#define RELOAD_TIME    0.3
#define kStatusBarHeight [[UIApplication sharedApplication] statusBarFrame].size.height

@interface FileListCollectionViewController ()<UIImagePickerControllerDelegate,UINavigationControllerDelegate , PlaybackStatusDelegate,MZFormSheetBackgroundWindowDelegate,FileActionDelegate>
{
    // Store margins for current setup
    CGFloat _margin, _gutter, _marginL, _gutterL, _columns, _columnsL;
    NSInteger _SelectdRow;
    NSIndexPath *_SelectdPath;

    int       _FileCount;
    E_ThumbnailAction _ThumbnailAction;
    ALAssetsLibrary *_CameraLibrary;
    NSMutableArray *_ThumbnailItemArray;
    NSMutableArray *_ReloadItemArray;
    FileInfoHandler *_FileInfoHandler;
    
    NSThread  *_ThumbnailThread;
    NSTimer   *_ReloadItemTimer;
    
    pthread_mutex_t Thumbnail_mutex;
    pthread_cond_t  Thumbnail_condition;
    bool            _bSupportRandomAccess;
    bool            _bIsDismissed;
    
    int             m_iVisibleCellsIndex;
}
@property (strong, nonatomic) IBOutlet UICollectionView *m_collectionView;

@end

@implementation FileListCollectionViewController

static NSString * const reuseIdentifier = @"FilePhotoCell";
//----------------------------------------------------------------------
-(void)selectDownAction:(id)sender
{
    _FileCount = 0;
    _bIsDismissed = true;
    [self dismissViewControllerAnimated:YES completion:nil];
}
//----------------------------------------------------------------------
- (void)viewDidLoad {
    [super viewDidLoad];
    
    m_iVisibleCellsIndex = 0;
    UIBarButtonItem *DownBtn = [[UIBarButtonItem alloc]initWithTitle:NSLocalizedString(@"Done", @"")
                                                               style:UIBarButtonItemStyleDone
                                                              target:self
                                                              action:@selector(selectDownAction:)];
    self.navigationItem.leftBarButtonItem = DownBtn;
    
    [[MZFormSheetBackgroundWindow appearance] setBackgroundColor:[UIColor clearColor]]; //There has a bug when landscape mode
    //[self.collectionView registerClass:[FilePhotoCollectionViewCell class] forCellWithReuseIdentifier:reuseIdentifier];
    // Do any additional setup after loading the view.
    
    // Defaults
    _columns = 3, _columnsL = 4;
    _margin = 0, _gutter = 1;
    _marginL = 0, _gutterL = 1;
    
    // For pixel perfection...
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad) {
        // iPad
        _columns = 6, _columnsL = 8;
        _margin = 1, _gutter = 2;
        _marginL = 1, _gutterL = 2;
    } else if ([UIScreen mainScreen].bounds.size.height == 480) {
        // iPhone 3.5 inch
        _columns = 3, _columnsL = 4;
        _margin = 0, _gutter = 1;
        _marginL = 1, _gutterL = 2;
    } else {
        // iPhone 4 inch
        _columns = 3, _columnsL = 5;
        _margin = 0, _gutter = 1;
        _marginL = 0, _gutterL = 2;
    }
    
    _bIsDismissed = false;
    _ThumbnailAction = E_ThumbnailAction_Start;
    
    if([[ProtocolAgent GetShareAgent] GetSettingFileParser].FWVersion <= FW_Old_Number)
        _bSupportRandomAccess = false;
    else
        _bSupportRandomAccess = true;
    
    _FileCount = 0;
    
    _ThumbnailItemArray = [[NSMutableArray alloc]init];
    _FileInfoHandler = [[FileInfoHandler alloc]init];
    _ReloadItemArray = [[NSMutableArray alloc]init];
    
    //Read file info
    [_FileInfoHandler ReadFile];
    _CameraLibrary = [[ALAssetsLibrary alloc]init];
    
    //qurey photo library permission
    [_CameraLibrary enumerateGroupsWithTypes:ALAssetsGroupSavedPhotos usingBlock:^(ALAssetsGroup *group, BOOL *stop) {
        
        if (!group) return;
        
        // your code here...
    } failureBlock:^(NSError *error) {
        
        NSAssert(!error, [error description]);
    }];
    
}
//----------------------------------------------------------------------
- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}
//------------------------- ----------------------------------------
- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
    [ProtocolAgent GetShareAgent].ActiveView = self.view;
    [ProtocolAgent GetShareAgent].PlaybackDelegate  = self;
    
    if(_ThumbnailAction == E_ThumbnailAction_Start)
    {
        printf("Get GetFullFileList\n");
        _ThumbnailAction = E_ThumbnailAction_Idle;
        [[ProtocolAgent GetShareAgent]SendSetMode:E_DeviceMode_Playback];
        [[ProtocolAgent GetShareAgent]SendGetFullFileList];
    }
    else
    {
        _ThumbnailAction = E_ThumbnailAction_Poll;
    }
    
    pthread_mutex_init(&Thumbnail_mutex,NULL);
    pthread_cond_init(&Thumbnail_condition, NULL);
    
    _ThumbnailThread = [[NSThread alloc]initWithTarget:self selector:@selector(PollThumbnailFunction) object:nil];
    [_ThumbnailThread start];
    
    
    _ReloadItemTimer = [NSTimer scheduledTimerWithTimeInterval:RELOAD_TIME
                                                        target:self
                                                      selector:@selector(ReloadItemHandler:)
                                                      userInfo:nil
                                                       repeats:YES];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(CommandFailed:)
                                                 name:Notification_Device_CommandFailed
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(ConnectionChangeNotification:)
                                                 name:Notification_Device_ConnectionChange
                                               object:nil];
    
    [[UIApplication sharedApplication]setIdleTimerDisabled:YES];
    if (@available(iOS 11.0, *)) {
        _m_collectionView.frame = self.view.safeAreaLayoutGuide.layoutFrame;
    }
}
//------------------------- ----------------------------------------
- (void)viewWillDisappear:(BOOL)animated
{
    [super viewWillDisappear:animated];

    [self CancelPollThumbnail];
    
    if(_ReloadItemTimer)
    {
        [_ReloadItemTimer invalidate];
        _ReloadItemTimer = nil;
    }
    
    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:Notification_Device_CommandFailed
                                                  object:nil];

    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:Notification_Device_ConnectionChange
                                                  object:nil];
    
    
    [_FileInfoHandler SaveFile];
}
//----------------------------------------------------------------------
- (void)dealloc
{

}
//----------------------------------------------------------------------
- (void)viewDidDisappear:(BOOL)animated
{
    if(_bIsDismissed)
    {
        _ThumbnailItemArray = nil;
        _ReloadItemArray = nil;
        _FileInfoHandler = nil;
        _CameraLibrary = nil;
        
        [ProtocolAgent GetShareAgent].PlaybackDelegate  = nil;
    }
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
//----------------------------------------------------------------------
#pragma mark - Layout

- (CGFloat)getColumns {
    if ((UIInterfaceOrientationIsPortrait([[UIApplication sharedApplication] statusBarOrientation]))) {
        return _columns;
    } else {
        return _columnsL;
    }
}
//----------------------------------------------------------------------
- (CGFloat)getMargin {
    if ((UIInterfaceOrientationIsPortrait([[UIApplication sharedApplication] statusBarOrientation]))) {
        return _margin;
    } else {
        return _marginL;
    }
}
//----------------------------------------------------------------------
- (CGFloat)getGutter {
    if ((UIInterfaceOrientationIsPortrait([[UIApplication sharedApplication] statusBarOrientation]))) {
        return _gutter;
    } else {
        return _gutterL;
    }
}
//----------------------------------------------------------------------
#pragma mark - Collection View
- (NSInteger) collectionView:(UICollectionView *)collectionView numberOfItemsInSection:(NSInteger)section
{
    return _FileCount;
}
//----------------------------------------------------------------------
- (UICollectionViewCell *) collectionView:(UICollectionView *)collectionView cellForItemAtIndexPath:(NSIndexPath *)indexPath
{
    FilePhotoCollectionViewCell *cell = [collectionView dequeueReusableCellWithReuseIdentifier:reuseIdentifier forIndexPath:indexPath];
    /*if (!cell) {
        cell = [[FilePhotoCollectionViewCell alloc] init];
    }*/
    cell.Index = indexPath.row;
   
    FileThumbnailItem *item = [_ThumbnailItemArray objectAtIndex:indexPath.row];
    
    if([[ProtocolAgent GetShareAgent]GetFileIndex:(int)indexPath.row] != -1)
    {
        cell.fileNameLabel.text = [[ProtocolAgent GetShareAgent] GetFileName:(int)indexPath.row];
        item.bIsLoadFileName = true;
    }
    else
        cell.fileNameLabel.text = NSLocalizedString(@"Loading", @"");
    
    bool bHaveImage = false;
    
    if(_ThumbnailItemArray.count > indexPath.row)
    {
        FileThumbnailItem *item = [_ThumbnailItemArray objectAtIndex:indexPath.row];
        if(item.Name)
        {
            bHaveImage = true;
            cell.image = [UIImage imageWithContentsOfFile:item.Name];
            item.bIsLoadImage = true;
        }
        
        if(item.Status == E_ThumbnailStatus_Broken)
            cell.userInteractionEnabled = false;
        else
            cell.userInteractionEnabled = true;
    }

    if(!bHaveImage)
        cell.image = [UIImage imageNamed:@"loading.png"];

    return cell;
}
//----------------------------------------------------------------------
- (CGSize)collectionView:(UICollectionView *)collectionView layout:(UICollectionViewLayout*)collectionViewLayout sizeForItemAtIndexPath:(NSIndexPath *)indexPath {
    CGFloat margin = [self getMargin];
    CGFloat gutter = [self getGutter];
    CGFloat columns = [self getColumns];
    CGFloat value = floorf(((self.view.bounds.size.width - (columns - 1) * gutter - 2 * margin) / columns));
    return CGSizeMake(value, value);
}
//----------------------------------------------------------------------
- (CGFloat)collectionView:(UICollectionView *)collectionView layout:(UICollectionViewLayout*)collectionViewLayout minimumInteritemSpacingForSectionAtIndex:(NSInteger)section {
    return [self getGutter];
}
//----------------------------------------------------------------------
- (CGFloat)collectionView:(UICollectionView *)collectionView layout:(UICollectionViewLayout*)collectionViewLayout minimumLineSpacingForSectionAtIndex:(NSInteger)section {
    return [self getGutter];
}
//----------------------------------------------------------------------
- (UIEdgeInsets)collectionView:(UICollectionView *)collectionView layout:(UICollectionViewLayout*)collectionViewLayout insetForSectionAtIndex:(NSInteger)section {
    CGFloat margin = [self getMargin];
    return UIEdgeInsetsMake(0, margin, 0, margin);
}
//----------------------------------------------------------------------
#pragma mark - collection view delegate

- (void) collectionView:(UICollectionView *)collectionView didSelectItemAtIndexPath:(NSIndexPath *)indexPath
{
    _SelectdRow = indexPath.row;
    _SelectdPath = indexPath;
    
    if([[ProtocolAgent GetShareAgent]GetFileIndex:(int)_SelectdRow] == -1) //Check if device index is ready
    {
        
    }
    else
    {
        FileThumbnailItem *item = [_ThumbnailItemArray objectAtIndex:_SelectdRow];
        if (item.Status == E_ThumbnailStatus_Ready) {
            [self CreatePopOutView:@"FileActionNC"
             WithCompletionHandler:^(UIViewController *presentedFSViewController) {
                 // Passing data
                 UINavigationController *navController = (UINavigationController *)presentedFSViewController;
                 navController.topViewController.title = [[ProtocolAgent GetShareAgent]GetFileName:_SelectdRow] ;
                 [[navController topViewController] setValue:self forKey:@"Delegate"];
                 [[navController topViewController] setValue:[NSNumber numberWithInt:(int)_SelectdRow] forKey:@"SelectIndex"];
                 [[navController topViewController] setValue:[NSNumber numberWithBool:[self VerifyFile:(int)_SelectdRow]] forKey:@"FileDownloaded"];
             }
             ];
        }
    }
}

//----------------------------------------------------------------------
-(void) SelectAction:(int)i32Action
{
    switch(i32Action)
    {
        case FILEACTION_PLAY:
        {
            [self VerifyFile:(int)_SelectdRow];
            [self performSegueWithIdentifier:@"File_Preview" sender:self];
            
            
            
//            NSString *docsPath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) lastObject];
//            NSString *filePath = [docsPath stringByAppendingPathComponent:[[ProtocolAgent GetShareAgent]GetFileName:(int)_SelectdRow]];
//
//            if([[NSFileManager defaultManager] fileExistsAtPath:filePath]) {
//                NSURL *fileURL = [[NSURL alloc] initFileURLWithPath:filePath];
//                AVPlayer *player = [AVPlayer playerWithURL:fileURL];
//
//                // create a player view controller
//                AVPlayerViewController *controller = [[AVPlayerViewController alloc] init];
//                controller.player = player;
//                [self presentViewController:controller animated:YES completion:nil];
//
//                [player play];
//            }
            
            
        }
            break;
        case FILEACTION_DOWNLOAD:
        {
            [self DownloadFile:_SelectdRow];
        }
            break;
        case FILEACTION_DELETE:
        {
            NSString *docsPath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) lastObject];
            NSString *filePath = [docsPath stringByAppendingPathComponent:[[ProtocolAgent GetShareAgent]GetFileName:_SelectdRow] ];
            
            BOOL bExist = [[NSFileManager defaultManager] fileExistsAtPath:filePath];
            if(NO == [ProtocolAgent GetShareAgent].m_bCanDeleteSDFile) {
                UIAlertController *alertController = [UIAlertController
                                                      alertControllerWithTitle:NSLocalizedString(@"Delete File", @"")
                                                      message:[NSString stringWithFormat:@"%@ %@?",
                                                               NSLocalizedString(@"Do you sure to delete", @""),
                                                               [[ProtocolAgent GetShareAgent]GetFileName:_SelectdRow]]
                                                      preferredStyle:UIAlertControllerStyleAlert];
                
                UIAlertAction *NoAction = [UIAlertAction
                                           actionWithTitle:NSLocalizedString(@"No", @"No action")
                                           style:UIAlertActionStyleDefault
                                           handler:^(UIAlertAction *action)
                                           {
                                               
                                           }];
                
                UIAlertAction *YesAction = [UIAlertAction
                                            actionWithTitle:NSLocalizedString(@"Yes", @"Yes action")
                                            style:UIAlertActionStyleDefault
                                            handler:^(UIAlertAction *action)
                                            {
                                                NSLog(@"Yes Pressed");
                                                
                                                if(bExist)
                                                {
                                                    [self deleteLocalFile:filePath];
                                                }
                                                
                                            }];
                
                [alertController addAction:NoAction];
                [alertController addAction:YesAction];
                [self presentViewController:alertController animated:YES completion:nil];
            }
            else {
                [self mz_dismissFormSheetControllerAnimated:YES completionHandler:^(UIViewController *presentedFSViewController) {
                    [self CreatePopOutView:@"DeleteSDNC"
                     WithCompletionHandler:^(UIViewController *presentedFSViewController) {
                         // Passing data
                        UINavigationController *navController = (UINavigationController *)presentedFSViewController;
                        navController.topViewController.title = NSLocalizedString(@"Delete File", nil);
                        [[navController topViewController] setValue:self forKey:@"Delegate"];
                        [[navController topViewController] setValue:[NSNumber numberWithBool:bExist] forKey:@"bFileExist"];
                     }];
                }];
            }
            
    
            
        }
            break;
        default:
            break;
    }
}
//----------------------------------------------------------------------
-(void)deleteLocalFile:(NSString*)filePath
{
    NSURL *fileURL = [[NSURL alloc] initFileURLWithPath:filePath];
    NSError *error;
    [[NSFileManager defaultManager] removeItemAtURL:fileURL error:&error];
    
    NSURL* assertUrl = [_FileInfoHandler GetFileAssertUrl:[[ProtocolAgent GetShareAgent]GetFileName:_SelectdRow]];
    if(assertUrl)
    {
        NSString *currSysVer = [[UIDevice currentDevice] systemVersion];
        if ([currSysVer compare:@"8.0" options:NSNumericSearch] != NSOrderedAscending)
        {
            PHFetchResult *result = [PHAsset fetchAssetsWithALAssetURLs:@[assertUrl] options:nil];
            PHAsset *asset = result.firstObject;
            
            if(asset)
            {
                [[PHPhotoLibrary sharedPhotoLibrary] performChanges:^{
                    [PHAssetChangeRequest deleteAssets:@[asset]];
                } completionHandler:^(BOOL success, NSError *error) {
                    NSLog(@"Finished deleting asset. %@", (success ? @"Success." : error));
                }];
            }
            
        }
        else
        {
            [_CameraLibrary assetForURL:assertUrl resultBlock:^(ALAsset *asset) {
                NSLog(@"Asset: %@", asset);
                [asset setImageData:nil metadata:nil completionBlock:^(NSURL *assetURL, NSError *error) {
                    
                }];
            } failureBlock:^(NSError *error) {
                NSLog(@"Failure, wahhh!");
            }];
        }
    }
}
//----------------------------------------------------------------------
-(void) DeleteAction:(BOOL)bDeleteSDFile
{
    NSString *docsPath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) lastObject];
    NSString *filePath = [docsPath stringByAppendingPathComponent:[[ProtocolAgent GetShareAgent]GetFileName:_SelectdRow] ];
    
    BOOL bExist = [[NSFileManager defaultManager] fileExistsAtPath:filePath];
    if (bExist) {
        [self deleteLocalFile:filePath];
    }
    
    if (bDeleteSDFile) {
        [[ProtocolAgent GetShareAgent] SendDeleteFile:_SelectdRow];
    }
}
//----------------------------------------------------------------------
-(bool)VerifyFile:(int)i32Index
{
    BYTE abyDate[6];
    [[ProtocolAgent GetShareAgent] GetFileTime:i32Index withTimeBuffer:abyDate];
    
    FileInfoItem *NewInfo = [[FileInfoItem alloc]init];
    NewInfo.FileName = [[ProtocolAgent GetShareAgent]GetFileName:i32Index] ;
    NewInfo.lDate = abyDate[0] + (abyDate[1]<<8) + (abyDate[2]<<16);
    NewInfo.lTime = abyDate[3] + (abyDate[4]<<8) + (abyDate[5]<<16);
    NewInfo.lSize = [[ProtocolAgent GetShareAgent]GetFileSize:i32Index];
    NewInfo.assertURL = [_FileInfoHandler GetFileAssertUrl:[[ProtocolAgent GetShareAgent]GetFileName:i32Index]];
    
    int i32Ret = [_FileInfoHandler CheckAndReplaceItem:NewInfo];
    bool keepFile = true;
    
    NSString *docsPath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) lastObject];
    NSString *filePath = [docsPath stringByAppendingPathComponent:NewInfo.FileName];
    
    if(i32Ret == CHECK_RET_MISMATCH)
    {
        keepFile = false;
    }
    else if(i32Ret == CHECK_RET_MATCH)
    {
        if([[NSFileManager defaultManager] fileExistsAtPath:filePath])
        {
            unsigned long long fileSize = [[[NSFileManager defaultManager] attributesOfItemAtPath:filePath error:nil] fileSize];
            if(NewInfo.lSize !=(fileSize/1024))
                keepFile = false;
        }
        else
            keepFile = false;
    }
    else
        keepFile = false;
    
    if(!keepFile)
    {
        if([[NSFileManager defaultManager] fileExistsAtPath:filePath])
        {
            NSURL *fileURL = [[NSURL alloc] initFileURLWithPath:filePath];
            NSError *error;
            [[NSFileManager defaultManager] removeItemAtURL:fileURL error:&error];
        }
    }
    
    return keepFile;
}
//----------------------------------------------------------------------
-(void)CheckFileInfo:(int)i32Index
           withSize:(int)i32Size
{
    for(int i=i32Index;i<i32Index+i32Size && i <_FileCount;i++)
        [self VerifyFile:i];
}
//----------------------------------------------------------------------
-(void) GetFileCount:(int)i32Count
{
    _FileCount = i32Count;
    printf("Total File count: %d\n",i32Count);
    
    for(int i=0;i<_FileCount;i++)
        [_ThumbnailItemArray addObject:[[FileThumbnailItem alloc]init]];
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [self.collectionView reloadData];
    });

}
//----------------------------------------------------------------------
-(void) GetFileName:(int)i32Index
           withSize:(int)i32Size
{
    printf("File Name Ready:  %d ~ %d\n",i32Index,i32Index+i32Size-1);
    
   /* NSArray *cellPathArrary = [self.collectionView indexPathsForVisibleItems];
    NSMutableArray *UpdatePaths = [[NSMutableArray alloc]init];
    
    for (NSIndexPath *path in cellPathArrary)
        if(path.row >= i32Index && path.row < i32Index + i32Size)
            [UpdatePaths addObject:path];
    
    if(UpdatePaths.count > 0)
        [self AddReloadItem:UpdatePaths];*/

    if(i32Index == 0)
    {
        if(i32Size == 0)
            _ThumbnailAction = E_ThumbnailAction_Finish;
        else
            _ThumbnailAction = E_ThumbnailAction_Poll;
    }
    
    [self CheckFileInfo:i32Index
               withSize:i32Size];
}
//----------------------------------------------------------------------
-(void)SignalPollThumbnail
{
    if(_bIsDismissed)
        return;
    
    printf("SignalPollThumbnail\n");
    pthread_mutex_lock(&Thumbnail_mutex);
    pthread_cond_signal(&Thumbnail_condition);
    pthread_mutex_unlock(&Thumbnail_mutex);
}
//----------------------------------------------------------------------
-(void)CancelPollThumbnail
{
    printf("CancelPollThumbnail\n");
    
    _ThumbnailAction = E_ThumbnailAction_Finish;
    [_ThumbnailThread cancel];
    
    if(pthread_mutex_trylock(&Thumbnail_mutex) == 0)
    {
        pthread_cond_signal(&Thumbnail_condition);
        pthread_mutex_unlock(&Thumbnail_mutex);
    }
}
//----------------------------------------------------------------------
-(void)WaitCondition
{
    while(_ThumbnailAction != E_ThumbnailAction_Finish)
    {
        struct timespec timeToWait;
        struct timeval now;

        gettimeofday(&now,NULL);

        timeToWait.tv_sec = now.tv_sec + 1; // 1sec timeout
        timeToWait.tv_nsec = now.tv_usec;

        if(pthread_cond_timedwait(&Thumbnail_condition,&Thumbnail_mutex,&timeToWait)!=ETIMEDOUT)
            break;
        
        if(_bIsDismissed)
            [NSThread exit];
    }
}
//----------------------------------------------------------------------
-(void)PollThumbnailFunction
{
    printf("thread start--------\n");
    
    while(_ThumbnailAction != E_ThumbnailAction_Finish)
    {
        
        if(_ThumbnailAction == E_ThumbnailAction_Idle)
        {
            usleep(2000000);
            continue;
        }
        
        pthread_mutex_lock(&Thumbnail_mutex);
        switch(_ThumbnailAction)
        {
            case E_ThumbnailAction_Poll:
                if([self PollThumbnail])
                    [self WaitCondition];
                else
                    _ThumbnailAction = E_ThumbnailAction_Recover;
                

                break;
            case E_ThumbnailAction_Recover:
                    
                if([self RecoverThumbnail])
                    [self WaitCondition];
                else
                    _ThumbnailAction = E_ThumbnailAction_Finish;
                    
                break;
                
            default:
                break;
        }
        pthread_mutex_unlock(&Thumbnail_mutex);


    }
    
    printf("thread end--------\n");
}
//----------------------------------------------------------------------
-(bool)RecoverThumbnail
{
    bool bIsContinue = true;
    bool bFound = false;
        
    int i32Index = 0;
    for (FileThumbnailItem *item in _ThumbnailItemArray)
    {
        if(item.Status == E_ThumbnailStatus_Failed)
        {
            printf("Recover Thumbnail ");
            [self SendThumbnailCommand:i32Index];
            bFound = true;
            break;
        }
        i32Index++;
    }
        
    if(!bFound)
        bIsContinue = false;

    return bIsContinue;
}

//----------------------------------------------------------------------
-(bool)PollThumbnail
{
    bool bIsContinue = true;
    bool bFound = false;
    
    //Try visible item first
    int i32Index = INT32_MAX;
    
    if(_bSupportRandomAccess)
    {
//        NSArray *cellArrary = [self.collectionView visibleCells];
//        for (FilePhotoCollectionViewCell *cell in cellArrary)
//        {
//            if(_ThumbnailItemArray.count > cell.Index)
//            {
//                FileThumbnailItem *item = [_ThumbnailItemArray objectAtIndex:cell.Index];
//                if(item.Status == E_ThumbnailStatus_Loading)
//                {
//                    if(bFound)
//                    {
//                        if(i32Index > (int)cell.Index)
//                            i32Index = (int)cell.Index;
//                    }
//                    else
//                        i32Index = (int)cell.Index;
//
//                    bFound = true;
//                }
//            }
//        }
        
        for (int Index = m_iVisibleCellsIndex; Index < m_iVisibleCellsIndex + 18 ; Index++) {
            if(_ThumbnailItemArray.count > Index)
            {
                FileThumbnailItem *item = [_ThumbnailItemArray objectAtIndex:Index];
                if(item.Status == E_ThumbnailStatus_Loading)
                {
                    if(bFound)
                    {
                        if(i32Index > Index)
                            i32Index = Index;
                    }
                    else
                        i32Index = Index;

                    bFound = true;
                }
            }
        }
    }

    //Sequentially
    if(!bFound)
    {
        i32Index = 0;
        for (FileThumbnailItem *item in _ThumbnailItemArray)
        {
            if(item.Status == E_ThumbnailStatus_Loading)
            {
                bFound = true;
                break;
            }
            i32Index++;
        }
    }
            
    if(bFound)
        [self SendThumbnailCommand:i32Index];
    else
        bIsContinue = false;

    return bIsContinue;
}
//----------------------------------------------------------------------
-(void)SendThumbnailCommand:(int)i32Index
{
    printf("Get Thumbnail %d\n",i32Index);
    
    int i32DeviceIdx = [[ProtocolAgent GetShareAgent]GetFileIndex:i32Index];
    
    if(_bSupportRandomAccess)
    {
        if(i32DeviceIdx == -1 )
            [[ProtocolAgent GetShareAgent] SetNextPlaybackFileListIndex:i32Index];
    }
    else
    {
        if(i32DeviceIdx== -1 )
        {
            double delayInSeconds = 2.0;
            dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, delayInSeconds * NSEC_PER_SEC);
            dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
                [self SignalPollThumbnail]; // give some time to get file name
            });
            
            return;
        }
    }
    
    int i32CMDIndex = [[ProtocolAgent GetShareAgent] SendGetFileThumbnail:i32Index];
    FileThumbnailItem *item = [_ThumbnailItemArray objectAtIndex:i32Index];
    item.CommandIndex = i32CMDIndex;
    item.Index = i32Index;
    item.Status = E_ThumbnailStatus_WaitingCommnad;
    
    if(i32CMDIndex== -1)
    {
        item.Status = E_ThumbnailStatus_Loading;
        
        double delayInSeconds = 2.0;
        dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, delayInSeconds * NSEC_PER_SEC);
        dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
            [self SignalPollThumbnail]; // give some time to get file name
        });
        
    }
    
}
//----------------------------------------------------------------------
-(void) FileThumbnailIsReady:(int)i32Index
           withThumbnailName:(NSString*)ThumbnailName
{
    if(_ThumbnailItemArray.count > i32Index)
    {
        FileThumbnailItem *item = [_ThumbnailItemArray objectAtIndex:i32Index];
        item.Name = ThumbnailName;
        item.Status = E_ThumbnailStatus_Ready;
    }
    
    [self SignalPollThumbnail];
}
//----------------------------------------------------------------------
-(void) FileRawData:(int)i32Index
       withFileName:(NSString*)FileName
{
    printf("Get FileRawData %d\n",i32Index);
    //Save info to file
    
    BYTE abyDate[6];
    [[ProtocolAgent GetShareAgent] GetFileTime:i32Index withTimeBuffer:abyDate];
    
    FileInfoItem *NewInfo = [[FileInfoItem alloc]init];
    NewInfo.FileName = [[ProtocolAgent GetShareAgent]GetFileName:i32Index] ;
    NewInfo.lDate = abyDate[0] + (abyDate[1]<<8) + (abyDate[2]<<16);
    NewInfo.lTime = abyDate[3] + (abyDate[4]<<8) + (abyDate[5]<<16);
    NewInfo.lSize = [[ProtocolAgent GetShareAgent]GetFileSize:i32Index];
    NewInfo.assertURL = nil;
    
    //Save to camera roll
    if ([FileName rangeOfString:@".jpg"].location != NSNotFound)
    {
        //jpge
        [_CameraLibrary saveImage: [UIImage imageWithContentsOfFile:FileName]
                          toAlbum:@"GoPlus Cam"
                       completion:^(NSURL  *assertURL, NSError *error)
                                    {
                                        NSLog(@"Save to Ablum complete: %@ , error: %@",assertURL,error);
                                        if(error)
                                        {
                                            [self ShowAlert:NSLocalizedString(@"Save to iOS photo library failed!",@"")
                                                withMessage:[NSString stringWithFormat:@"%@",error ]];
                                        }
                                        else
                                        {
                                            NewInfo.assertURL = assertURL;
                                            [_FileInfoHandler AddItem:NewInfo];
                                        }

                                    }
                          failure:^(NSError *error) {
                                    NSLog(@"Failed to save  error: %@",error);
                                    [self ShowAlert:NSLocalizedString(@"Save to iOS photo library failed!",@"")
                                        withMessage:[NSString stringWithFormat:@"%@",error ]];
                                }
         ];
    }
    else
    {
        //avi
        if ([FileName rangeOfString:@".avi"].location != NSNotFound) {
            //check resolution is 720P or 1080P?
            //if resolution is 1080P, do not save to photo library
            const char *pFilePath = [FileName cStringUsingEncoding:NSASCIIStringEncoding];
            FILE *fp = fopen(pFilePath,"r");
            UInt32 u32Size = 0x100/2;
            
            //resolution info locate at 0xB4 & 0xB5 (in byte offset)
            UINT16 resolution = 0;
            UINT16 *pData = malloc(u32Size * sizeof(UINT16));
            if (pData)
            {
                fread(pData, u32Size, sizeof(UINT16), fp);
                resolution = pData[0xB4/2];
            }
            free(pData);
            fclose(fp);
            
            if (resolution > 720) {
                [self ShowAlert:NSLocalizedString(@"Resolution > 720P!",@"")
                    withMessage:NSLocalizedString(@"Video file with resolution > 720P will not save to iOS photo library!",@"")];
            }
            else
            {
                [self copyFile:FileName and:NewInfo];
            }
        }
        else if ([FileName rangeOfString:@".mov"].location != NSNotFound) {
            [self copyFile:FileName and:NewInfo];
        }
        
    }
    
    [_FileInfoHandler AddItem:NewInfo];
}
//----------------------------------------------------------------------
-(void) copyFile:(NSString*)strFileName and:(FileInfoItem *)NewInfo{
    NSURL  *FileURL = [NSURL URLWithString:[strFileName stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]];
    [_CameraLibrary saveVideo:FileURL
                      toAlbum:@"GoPlus Cam"
                   completion:^(NSURL  *assertURL, NSError *error)
     {
         NSLog(@"Save to Album complete: %@ , error: %@",assertURL,error);
         if(error)
         {
             [self ShowAlert:NSLocalizedString(@"Save to iOS photo library failed!",@"")
                 withMessage:[NSString stringWithFormat:@"%@",error ]];
         }
         else
         {
             NewInfo.assertURL = assertURL;
             [_FileInfoHandler AddItem:NewInfo];
         }
     }
                      failure:^(NSError *error) {
                          NSLog(@"Failed to save  error: %@",error);
                          [self ShowAlert:NSLocalizedString(@"Save to iOS photo library failed!",@"")
                              withMessage:[NSString stringWithFormat:@"%@",error ]];
                      }
     ];

}
//----------------------------------------------------------------------
-(void) FileDeleted:(bool)bsucceed
{
    if(bsucceed)
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            
            [self.collectionView performBatchUpdates:^(void){
                [_ThumbnailItemArray removeObjectAtIndex:_SelectdRow];
                [self.collectionView deleteItemsAtIndexPaths:@[[NSIndexPath indexPathForRow:_SelectdRow inSection:0]]];
                _FileCount--;
            } completion:^(BOOL finished) {
                //[self.collectionView reloadData];
            }];
            
        });
    }
    
}
//----------------------------------------------------------------------
-(void) ShowAlert:(NSString*) title
      withMessage:(NSString*) message
{
    UIAlertController *alertController = [UIAlertController
                                          alertControllerWithTitle:title
                                          message:message
                                          preferredStyle:UIAlertControllerStyleAlert];
    
    UIAlertAction *okAction = [UIAlertAction
                               actionWithTitle:NSLocalizedString(@"OK", @"OK action")
                               style:UIAlertActionStyleDefault
                               handler:^(UIAlertAction *action)
                               {
                                   
                               }];
    
    [alertController addAction:okAction];
    dispatch_async(dispatch_get_main_queue(), ^{
        [self presentViewController:alertController animated:YES completion:nil];
    });
}
//----------------------------------------------------------------------
- (void) CommandFailed:(NSNotification*) notification
{
    NSArray *PassObjs = [notification object];
    NSNumber *FailedIndex = [PassObjs objectAtIndex:0];
    NSNumber *ErrorCode = [PassObjs objectAtIndex:1];
    
    int i32Index = [FailedIndex intValue];
    
    printf("Handle command failed, Index: %d Error: %d\n",i32Index,[ErrorCode intValue]);
    
    for (FileThumbnailItem *item in _ThumbnailItemArray)
    {
        if(item.CommandIndex == i32Index)
        {
            if([ErrorCode intValue] == Error_GetThumbnailFail || [ErrorCode intValue] == Error_InvalidCommand) //thumbnail is broken
            {
                item.Name = [[NSBundle mainBundle] pathForResource:@"broken" ofType:@"png"];
                item.Status = E_ThumbnailStatus_Broken;
                break;
            }
        
            item.Status = E_ThumbnailStatus_Failed;
            break;
        }
    }
    
    [self SignalPollThumbnail];

}
//-----------------------------------------------------------------
-(void) prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    if([segue.identifier isEqualToString:@"BackToEntry"])
        return ;
    
    _ThumbnailAction = E_ThumbnailAction_Idle;
    
    [segue.destinationViewController setValue:  [[ProtocolAgent GetShareAgent]GetFileName:(int)_SelectdRow]  forKey:@"Title"];
    [segue.destinationViewController setValue:  [NSNumber numberWithInt:(int)_SelectdRow]  forKey:@"Index"];
    
    FileThumbnailItem *item = [_ThumbnailItemArray objectAtIndex:_SelectdRow];
    [segue.destinationViewController setValue:  item.Name  forKey:@"Thumbnail"];
}
//----------------------------------------------------------------------
-(void) DownloadFile:(NSInteger)Index
{
    if([[ProtocolAgent GetShareAgent]GetFileIndex:(int)Index] != -1) //Check if device index is ready
    {
        bool bRet = [self VerifyFile:(int)Index];
        if(bRet)
        {
            UIAlertController *alertController = [UIAlertController
                                                  alertControllerWithTitle:NSLocalizedString(@"Download file", @"")
                                                  message:[NSString stringWithFormat:@"%@ %@ %@",
                                                           NSLocalizedString(@"File", @""),
                                                           [[ProtocolAgent GetShareAgent]GetFileName:(int)Index],
                                                           NSLocalizedString(@"has been downloaded.", @"")]
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
            [[ProtocolAgent GetShareAgent] SendGetFileRawData:(int)Index];
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

//-----------------------------------------------------------------------
-(void) ReloadItemHandler:(NSTimer*) timer
{
    dispatch_async(dispatch_get_main_queue(), ^{
        NSArray *cellArrary = [self.collectionView visibleCells];
        for (FilePhotoCollectionViewCell *cell in cellArrary)
        {
            if(_ThumbnailItemArray.count > cell.Index)
            {
                FileThumbnailItem *item = [_ThumbnailItemArray objectAtIndex:cell.Index];
                if(([[ProtocolAgent GetShareAgent]GetFileIndex:(int)cell.Index] != -1 &&!item.bIsLoadFileName) ||
                   ((item.Status == E_ThumbnailStatus_Ready || item.Status == E_ThumbnailStatus_Broken)&&!item.bIsLoadImage)
                   )
                {
                    [_ReloadItemArray addObjectsFromArray:@[[self.collectionView indexPathForCell:cell]]];
                }
                
            }
        }
        
        if(_ReloadItemArray.count > 0)
        {
            [self.collectionView reloadItemsAtIndexPaths:_ReloadItemArray];
            [_ReloadItemArray removeAllObjects];
        }
    });
}
//-----------------------------------------------------------------------
- (void)viewWillTransitionToSize:(CGSize)size withTransitionCoordinator:(id<UIViewControllerTransitionCoordinator>)coordinator
{
    [super viewWillTransitionToSize:size withTransitionCoordinator:coordinator];

    [coordinator animateAlongsideTransition:^(id<UIViewControllerTransitionCoordinatorContext> context)
        {
            if (@available(iOS 11.0, *)) {
                _m_collectionView.frame = self.view.safeAreaLayoutGuide.layoutFrame;
            }
            UIInterfaceOrientation orientation = [[UIApplication sharedApplication] statusBarOrientation];
            // do whatever
            [self.collectionView reloadData];
         
        }
                                 completion:^(id<UIViewControllerTransitionCoordinatorContext> context)
        {
         
        }
     ];
}
//-----------------------------------------------------------------------

- (void)scrollViewDidEndDecelerating:(UIScrollView *)scrollView
{
    NSArray *cellArrary = [self.collectionView visibleCells];
    if(cellArrary.count > 0) {
        FilePhotoCollectionViewCell *cell = [cellArrary objectAtIndex:0];
        m_iVisibleCellsIndex = cell.Index;
        for (FilePhotoCollectionViewCell *cell in cellArrary)
        {
            if(cell.Index < m_iVisibleCellsIndex) {
                m_iVisibleCellsIndex = cell.Index;
            }
        }
        NSLog(@"scrollViewDidEndDecelerating ====== %d",m_iVisibleCellsIndex);
    }
}

- (void)scrollViewDidEndDragging:(UIScrollView *)scrollView willDecelerate:(BOOL)decelerate
{
    NSArray *cellArrary = [self.collectionView visibleCells];
    
    if(cellArrary.count > 0) {
        FilePhotoCollectionViewCell *cell = [cellArrary objectAtIndex:0];
        m_iVisibleCellsIndex = cell.Index;
        for (FilePhotoCollectionViewCell *cell in cellArrary)
        {
            if(cell.Index < m_iVisibleCellsIndex) {
                m_iVisibleCellsIndex = cell.Index;
            }
        }
        NSLog(@"scrollViewDidEndDragging ====== %d",m_iVisibleCellsIndex);
    }
}
@end
