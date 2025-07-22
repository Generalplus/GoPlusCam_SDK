//
//  FilePreviewViewController.m
//  GPCam
//
//  Created by generalplus_sa1 on 8/24/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#import "FilePreviewViewController.h"
#import "ProtocolAgent.h"

@interface FilePreviewViewController ()
{
    UITapGestureRecognizer  *VLCTapReger;
    UITapGestureRecognizer  *CtrlTapReger;
    bool                    bIsPause;
    bool                    bPlayLocal;
    NSTimer                 *m_StatusTimer;
    long                    m_iFileTime;
    bool                    bPlayEnd;
    int                     m_iPlayEndIndex;
}

@property (retain, nonatomic) IBOutlet UIView   *movieView;
@property (retain, nonatomic) IBOutlet UIView   *controllView;
@property (retain, nonatomic) IBOutlet UIButton *playpauseButton;
@property (retain, nonatomic) IBOutlet UIImageView *photoImageView;
@property (retain, nonatomic) IBOutlet UILabel *m_lbTime;

@end
//-----------------------------------------------------------------------
@implementation FilePreviewViewController
//-----------------------------------------------------------------------
- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    self.navigationItem.title = _Title;
    
    
    VLCTapReger = [[UITapGestureRecognizer alloc]initWithTarget:self action:@selector(VLCViewTapped:)];
    [self.movieView addGestureRecognizer:VLCTapReger];

    CtrlTapReger = [[UITapGestureRecognizer alloc]initWithTarget:self action:@selector(ControllViewTapped:)];
    [self.controllView addGestureRecognizer:CtrlTapReger];
    bIsPause = false;
    bPlayLocal = false;
    bIsPause = false;
    m_iPlayEndIndex = 0;
    
}
//-----------------------------------------------------------------------
- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}
//-----------------------------------------------------------------------
- (IBAction)PressPlayPause:(id)sender
{
    
    if(bPlayLocal)
    {
        if(bIsPause)
            [[ProtocolAgent GetShareAgent].FFplayer Resume];
        else
            [[ProtocolAgent GetShareAgent].FFplayer Pause];
    }
    else
    {
        if(bPlayEnd || [ProtocolAgent GetShareAgent].PlayerStatus == FFMPEG_STATUS_STOPPED)
        {
            [[ProtocolAgent GetShareAgent].FFplayer Stop];
            [[ProtocolAgent GetShareAgent] SendStopPlayback];
            bPlayEnd = false;
            bIsPause = true;
            m_iPlayEndIndex = 0;
            [[ProtocolAgent GetShareAgent] SendRestartStreaming];
            [[ProtocolAgent GetShareAgent] SendStartPlayback:[_Index intValue]];
            
        }
        else
        {
            if(bIsPause)
                [[ProtocolAgent GetShareAgent].FFplayer Resume];
            else
                [[ProtocolAgent GetShareAgent].FFplayer Pause];
            
            [[ProtocolAgent GetShareAgent] SendPausePlayback];
        }
    }
    
    bIsPause = !bIsPause;
}
//-----------------------------------------------------------------------
- (void)VLCViewTapped:(UIGestureRecognizer *)recognizer
{
    if([self.view.subviews lastObject] == _controllView)
    {
        [[self navigationController] setNavigationBarHidden:YES animated:YES];
        [self.view sendSubviewToBack:_controllView];
    }
    else
    {
        [[self navigationController] setNavigationBarHidden:NO animated:YES];
        [self.view bringSubviewToFront:_controllView];
    }
}
//-----------------------------------------------------------------------
- (void)ControllViewTapped:(UIGestureRecognizer *)recognizer
{
    
}
//------------------------- ----------------------------------------
- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
    
    NSString *docsPath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) lastObject];
    NSString *filePath = [docsPath stringByAppendingPathComponent:_Title];
    [ProtocolAgent GetShareAgent].FFplayer.DrawView = _movieView;
    [[self navigationController] setNavigationBarHidden:YES animated:YES];
    
    [[ProtocolAgent GetShareAgent].FFplayer SetAutoReconnect:false];
    
    if([[NSFileManager defaultManager] fileExistsAtPath:filePath])
    {
        if ([_Title rangeOfString:@".jpg"].location != NSNotFound)
        {
            _photoImageView.image = [UIImage imageWithContentsOfFile:filePath];
            _controllView.hidden = true;
        }
        else
        {
            [[ProtocolAgent GetShareAgent].FFplayer SetStreaming:false];
            [[ProtocolAgent GetShareAgent].FFplayer VideoPath:filePath];
            [[ProtocolAgent GetShareAgent].FFplayer Play];
        }
        
        bPlayLocal = true;
    }
    else
    {

        [ProtocolAgent GetShareAgent].ActiveView = self.parentViewController.view;
        
        NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
        int myInt = (int)[userDefaults integerForKey:NSUserDefaults_SetBufferingTime];
        [[ProtocolAgent GetShareAgent].FFplayer SetBufferingTime:myInt];
        [[ProtocolAgent GetShareAgent].FFplayer SetStreaming:true];
        
        [[ProtocolAgent GetShareAgent] SendRestartStreaming];
        [[ProtocolAgent GetShareAgent] SendStartPlayback:[_Index intValue]];
        
        
        BOOL bSetDiscardcorrupt = (BOOL)[userDefaults boolForKey:NSUserDefaults_SetDiscardcorrupt];
        NSString* strTemp = @"timeout=-1;stimeout=-1;probesize=4096;";
        if (bSetDiscardcorrupt) {
            strTemp = @"timeout=-1;stimeout=-1;probesize=4096;fflags=discardcorrupt;";
        }
        [[ProtocolAgent GetShareAgent].FFplayer UserOptions:strTemp]; // Keep connection after end of streaming.

        if ([_Title rangeOfString:@".jpg"].location != NSNotFound)
        {
            // Make sure VLC will display jpeg
            [[ProtocolAgent GetShareAgent] SendStartPlayback:[_Index intValue]];
//            [[ProtocolAgent GetShareAgent] SendStartPlayback:[_Index intValue]];
//            [[ProtocolAgent GetShareAgent] SendStartPlayback:[_Index intValue]];
        }
        bPlayLocal = false;
    }
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(ConnectionChangeNotification:)
                                                 name:Notification_Device_ConnectionChange
                                               object:nil];
    
    m_iFileTime = -1;
    if ([_Title rangeOfString:@".jpg"].location == NSNotFound) {
        NSURL *fileUrl = [NSURL fileURLWithPath:_Thumbnail];
        NSData *fileData = [NSData dataWithContentsOfURL:fileUrl];
        
        if (fileData.length > 0x21) {
            unsigned char *cData;
            cData = malloc([fileData length]);
            [fileData getBytes:cData];
            
            if (cData[0x19] == 'T' && cData[0x1A] == 'I'&& cData[0x1B] == 'M'&& cData[0x1C] == 'E') {
                long l = 0;
                l |= cData[0x20] & 0xFF;
                l <<= 8;
                l |= cData[0x1F] & 0xFF;
                l <<= 8;
                l |= cData[0x1E] & 0xFF;
                l <<= 8;
                l |= cData[0x1D] & 0xFF;
                
                m_iFileTime = l;
                
                m_StatusTimer = [NSTimer scheduledTimerWithTimeInterval:0.5
                  target:self
                selector:@selector(StatusTimerHandler:)
                userInfo:nil
                 repeats:YES];
                _m_lbTime.hidden = NO;
            }
            
        }
    }
        
}
//------------------------- ----------------------------------------
-(void) StatusTimerHandler:(NSTimer*) timer
{
    long iTime = [[ProtocolAgent GetShareAgent].FFplayer GetPosition] / 1000000;
    
    [_m_lbTime setText:[NSString stringWithFormat:@"%.2lds / %.2lds",iTime, m_iFileTime]];
    //NSLog(@"GetPosition = %ld",iTime/1000000);
    if(iTime == m_iFileTime) {
        bPlayEnd = true;
    }
    
    if(iTime == m_iFileTime - 1 && bIsPause == false) {
        m_iPlayEndIndex++;
    }
    
    if(m_iPlayEndIndex >= 2) {
        bPlayEnd = true;
    }
    
}
//------------------------- ----------------------------------------
- (void)viewWillDisappear:(BOOL)animated
{
    [super viewWillDisappear:animated];
    
    [[ProtocolAgent GetShareAgent].FFplayer SetAutoReconnect:true];
    [[ProtocolAgent GetShareAgent].FFplayer SetBufferingTime:0];
    //[[ProtocolAgent GetShareAgent].VLCplayer stop];
    
    if(bPlayLocal)
    {
        [[ProtocolAgent GetShareAgent] ResetStreamingURL];
        [[ProtocolAgent GetShareAgent].FFplayer Stop];
    }
    else
    {
	    [[ProtocolAgent GetShareAgent].FFplayer Stop];
        [ProtocolAgent GetShareAgent].FFplayer.DrawView = nil;
        [[ProtocolAgent GetShareAgent] SendStopPlayback];
    }
    
    NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
    BOOL bSetDiscardcorrupt = (BOOL)[userDefaults boolForKey:NSUserDefaults_SetDiscardcorrupt];
    NSString* strTemp = @"";
    if (bSetDiscardcorrupt) {
        strTemp = @"fflags=discardcorrupt;";
    }
    [[ProtocolAgent GetShareAgent].FFplayer UserOptions:strTemp]; // Keep connection after end of streaming.
    
    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:Notification_Device_ConnectionChange
                                                  object:nil];
    
    if(m_StatusTimer)
    {
        [m_StatusTimer invalidate];
        m_StatusTimer = nil;
    }
}
//----------------------------------------------------------------------
- (void) ConnectionChangeNotification:(NSNotification*) notification
{
    NSString *Status = [notification object];
    
    if([Status isEqualToString:@"DisConnected"])
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            [[ProtocolAgent GetShareAgent].FFplayer Stop];
            [self performSegueWithIdentifier:@"BackToEntry" sender:self];
        });
    }
    
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
