//
//  MainViewController
//  GPCam
//
//  Created by generalplus_sa1 on 8/7/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#import "MainViewController.h"
#import "ProtocolAgent.h"

#import "MZFormSheetController.h"
#import "MZCustomTransition.h"
#import "MZFormSheetSegue.h"
#import "VLCDisplayView.h"
#import "LanguageManager.h"
#import "AppDelegate.h"

#define STATUS_CHECKTIME             0.5

typedef enum{
    
    E_Battery_Level0 = 0,
    E_Battery_Level1 = 1,
    E_Battery_Level2 = 2,
    E_Battery_Level3 = 3,
    E_Battery_Level4 = 4,
    E_Battery_Charge = 5,

}E_Battery_Level;

//-----------------------------------------------------------------------
@interface MainViewController () <MZFormSheetBackgroundWindowDelegate,GetParameterDelegate>
{
    UITapGestureRecognizer  *VLCTapReger;
    UITapGestureRecognizer  *TopCtrlTapReger;
    UITapGestureRecognizer  *CtrlTapReger;
    NSTimer                 *m_StatusTimer;
    E_DeviceMode            _CurrentMode;
    
    NSArray                 *_BatteryImageArrary;
    bool                    _CanDisMiss;
    int                     _LoopRecordID;
    int                     _LastSendModeIdx;
    
    NSTimeInterval          _PrevTime;
    NSInteger               _PrevDisplay;
    NSTimeInterval          _MissSyncTime;
    bool                    m_bFirst;
}

@property (retain, nonatomic) IBOutlet VLCDisplayView   *movieView;
@property (retain, nonatomic) IBOutlet UIView           *controllView;
@property (retain, nonatomic) IBOutlet UIView           *controllTopView;

@property (retain, nonatomic) IBOutlet UIImageView      *recordImageView;
@property (retain, nonatomic) IBOutlet UIImageView      *cameraImageView;
@property (retain, nonatomic) IBOutlet UIImageView      *muteImageView;
@property (retain, nonatomic) IBOutlet UIImageView      *batteryImageView;

@property (retain, nonatomic) IBOutlet UIButton         *connectButton;
@property (retain, nonatomic) IBOutlet UIButton         *recordButton;
@property (retain, nonatomic) IBOutlet UIButton         *settingButton;
@property (retain, nonatomic) IBOutlet UIButton         *fileButton;
@property (retain, nonatomic) IBOutlet UIButton         *modeButton;
@property (retain, nonatomic) IBOutlet UIButton         *audioButton;
@property (retain, nonatomic) IBOutlet UIButton         *logButton;
@property (retain, nonatomic) IBOutlet UIButton         *PIPButton;

@property (retain, nonatomic) IBOutlet UILabel          *StatusLabel;
@property (retain, nonatomic) IBOutlet UILabel          *ResLabel;
@property (retain, nonatomic) IBOutlet UILabel          *TimeLabel;
@property (retain, nonatomic) IBOutlet UILabel          *RateLabel;
@end

@implementation MainViewController
//-----------------------------------------------------------------------
- (void)VLCViewTapped:(UIGestureRecognizer *)recognizer
{
    if(_controllView.layer.zPosition == MAXFLOAT)
    {
        _controllView.layer.zPosition = 0;
        _controllTopView.layer.zPosition = 0;
        
        [self.view sendSubviewToBack:_controllTopView];
        [self.view sendSubviewToBack:_controllView];
    }
    else
    {
        _controllView.layer.zPosition = MAXFLOAT;
        _controllTopView.layer.zPosition = MAXFLOAT;
        
        [self.view bringSubviewToFront:_controllTopView];
        [self.view bringSubviewToFront:_controllView];
    }
}
//-----------------------------------------------------------------------
- (void)ControllViewTapped:(UIGestureRecognizer *)recognizer
{

}
//-----------------------------------------------------------------------
- (IBAction)PressSetPIP:(UIButton*)sender
{
    NSLog(@"PressSetPIP");
    [[ProtocolAgent GetShareAgent] SendGetSetPIP:1];
}
//-----------------------------------------------------------------------
- (IBAction)PressRecord:(UIButton*)sender
{
    sender.enabled = NO;
        
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        sender.enabled = YES;
    });

    switch(_CurrentMode)
    {
        case E_DeviceMode_Record:
            NSLog(@"PressRecord");
            [[ProtocolAgent GetShareAgent] SendRecordCmd];
            break;
        case E_DeviceMode_Capture:
            NSLog(@"PressCapture");
            [[ProtocolAgent GetShareAgent] SendCapturePicture];
            break;
        default:

            break;
    }
}
//-----------------------------------------------------------------------
-(void)StatusTimer:(BOOL)bStart
{
    if(m_StatusTimer)
    {
        [m_StatusTimer invalidate];
        m_StatusTimer = nil;
    }
    
    if(bStart &&
       [[ProtocolAgent GetShareAgent] GetConnectionStatus]== E_ConnectionStatus_Connected)
    {
        m_StatusTimer = [NSTimer scheduledTimerWithTimeInterval:STATUS_CHECKTIME
                                                         target:self
                                                       selector:@selector(StatusTimerHandler:)
                                                       userInfo:nil
                                                        repeats:YES];
    }

}
//-----------------------------------------------------------------------
- (IBAction)PressDisconnect:(id)sender
{
    NSLog(@"PressDisconnect");
    if ( [[ProtocolAgent GetShareAgent] GetConnectionStatus] == E_ConnectionStatus_Connected)
    {
        [_connectButton setTitle:@"Connect" forState:UIControlStateNormal];
        [[ProtocolAgent GetShareAgent].FFplayer Stop];
        [[ProtocolAgent GetShareAgent] Disconnect];
    }
    
    [self Connection:false];
    
    if (![self.presentedViewController isBeingDismissed] && _CanDisMiss)
    {
        _CanDisMiss = false;
        [self dismissViewControllerAnimated:YES completion:nil];
    }
}
//-----------------------------------------------------------------------
- (IBAction)PressMode:(id)sender
{
    [self CreatePopOutView:@"ModeNC"
     WithCompletionHandler:^(UIViewController *presentedFSViewController) {
         // Passing data
         UINavigationController *navController = (UINavigationController *)presentedFSViewController;
         navController.topViewController.title = NSLocalizedString(@"Mode", @"");
         
         [[navController topViewController] setValue:[NSNumber numberWithInt:_CurrentMode] forKey:@"SelectIndex"];
         
     }
                FullScreen:FALSE];
}
//-----------------------------------------------------------------------
- (IBAction)PresVendor:(id)sender
{
    [self CreatePopOutView:@"VendorNC"
     WithCompletionHandler:^(UIViewController *presentedFSViewController) {}
                FullScreen:TRUE];
}
//-----------------------------------------------------------------------
- (IBAction)PresLog:(id)sender
{
    [self CreatePopOutView:@"LogNC"
     WithCompletionHandler:^(UIViewController *presentedFSViewController) {}
                FullScreen:TRUE];
}
//-----------------------------------------------------------------------
- (IBAction)PressAudio:(id)sender
{
    NSLog(@"PressAudio");
    static BOOL bIsOn = TRUE;
    bIsOn = !bIsOn;
    [[ProtocolAgent GetShareAgent] SendAudioOnOff:bIsOn];
}
//-----------------------------------------------------------------------
/*- (void) touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    int count = (int)[[touches allObjects] count];
    for(int i=0;i<count;i++)
    {
        CGPoint point = [[[touches allObjects] objectAtIndex:i]locationInView:self.view];
        UIView *hitView = [self.view hitTest:point withEvent:event];
        if(hitView == _controllTopView || hitView == _controllView)
        {
            return;
        }
    }
    
    if([self.view.subviews lastObject] == _controllView)
    {
        [self.view sendSubviewToBack:_controllTopView];
        [self.view sendSubviewToBack:_controllView];
    }
    else
    {
        [self.view bringSubviewToFront:_controllTopView];
        [self.view bringSubviewToFront:_controllView];
    }
}*/
//----------------------------------------------------------------------
- (void) ConnectionChangeNotification:(NSNotification*) notification
{
    NSString *Status = [notification object];

    if([Status isEqualToString:@"DisConnected"])
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            _batteryImageView.alpha = 0.1;
            //[self PressDisconnect:nil];
            
            [[ProtocolAgent GetShareAgent].FFplayer Stop];
            [[ProtocolAgent GetShareAgent] Disconnect];
            [self Connection:false];
            [self performSegueWithIdentifier:@"BackToEntry" sender:self];
        });
    }
    
}
//----------------------------------------------------------------------
- (void) SetModeCompleteNotification:(NSNotification*) notification
{
    
    bool bIsPlaying = [ProtocolAgent GetShareAgent].FFplayer.playing;
    printf("SetModeCompleteNotification is VLC playing %d\n",bIsPlaying);
    
    if(!bIsPlaying)
    {
        
        dispatch_async(dispatch_get_main_queue(), ^{
            [self Connection:true];
            [[ProtocolAgent GetShareAgent] SyncFrame];
            
            _batteryImageView.alpha = 1.0;
        });
        
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
    
    if(i32Index == _LastSendModeIdx && _LastSendModeIdx != -1)
        _LastSendModeIdx = [[ProtocolAgent GetShareAgent]SendSetModeDirectly:_CurrentMode];
        
}
//----------------------------------------------------------------------
- (void) ShowPIPButton:(NSNotification*) notification
{
    dispatch_sync(dispatch_get_main_queue(), ^{

        _PIPButton.hidden = NO;
    });
    
}
//----------------------------------------------------------------------
-(void)CreatePopOutView:(NSString *)StoryboardName
  WithCompletionHandler:(MZFormSheetCompletionHandler) Handler
             FullScreen:(BOOL)bFullScreen
{
    [[MZFormSheetBackgroundWindow appearance] setBackgroundColor:[UIColor clearColor]]; //There has a bug when landscape mode
    
    UIViewController *vc = [self.storyboard instantiateViewControllerWithIdentifier:StoryboardName];
    
    MZFormSheetController *formSheet = [[MZFormSheetController alloc] initWithViewController:vc];
    
    CGFloat screenWidth = 300;
    CGFloat screenHeight = 298;
    
    if(bFullScreen)
    {
        CGRect screenRect = [[UIScreen mainScreen] bounds];
        screenWidth = screenRect.size.width;
        screenHeight = screenRect.size.height;
    }
    
    formSheet.presentedFormSheetSize = CGSizeMake(screenWidth, screenHeight);
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

//----------------------------------------------------------------------
-(void) ButtonsEnable:(bool) bEnable
{
    if(bEnable)
    {
        _recordButton.enabled = true;
        _recordButton.alpha = 1.0;
        
        _settingButton.enabled = true;
        _settingButton.alpha = 1.0;
        
        _fileButton.enabled = true;
        _fileButton.alpha = 1.0;
        
        _modeButton.enabled = true;
        _modeButton.alpha = 1.0;
        
        _audioButton.enabled = true;
        _audioButton.alpha = 1.0;
        
        _connectButton.enabled = true;
        _connectButton.alpha = 1.0;
    }
    else
    {
        _recordButton.enabled = false;
        _recordButton.alpha = 0.2;
        
        _settingButton.enabled = false;
        _settingButton.alpha = 0.2;
        
        _fileButton.enabled = false;
        _fileButton.alpha = 0.2;
        
        _modeButton.enabled = false;
        _modeButton.alpha = 0.2;
        
        _audioButton.enabled = false;
        _audioButton.alpha = 0.2;
        
        _connectButton.enabled = false;
        _connectButton.alpha = 0.2;
    }
}
//-----------------------------------------------------------------------
-(void)ShowMultiShot:(BOOL)bEnable
{
    if(bEnable && _cameraImageView.hidden)
        _cameraImageView.hidden = FALSE;
    else if(!bEnable && !_cameraImageView.hidden)
        _cameraImageView.hidden = TRUE;
}
//-----------------------------------------------------------------------
-(void)AnimateRecord:(BOOL)bEnable
{
    if(bEnable && ![_recordImageView isAnimating])
        [_recordImageView startAnimating];
    else if (!bEnable && [_recordImageView isAnimating])
        [_recordImageView stopAnimating];
}
//-----------------------------------------------------------------------
-(void)ShowMute:(BOOL)bEnable
{
    if(bEnable && _muteImageView.hidden)
        _muteImageView.hidden = FALSE;
    else if(!bEnable && !_muteImageView.hidden)
        _muteImageView.hidden = TRUE;
}
//-----------------------------------------------------------------------
-(void)ShowBattery:(BYTE)byLevel
          IsCharge:(BOOL)bCharge
{
    
    int i32Idx = (E_Battery_Level)byLevel;
    if(bCharge)
        i32Idx = E_Battery_Charge;
    
    if(byLevel>E_Battery_Level4)
        i32Idx = E_Battery_Level4;
    
    if(_batteryImageView.image != [_BatteryImageArrary objectAtIndex:i32Idx])
        _batteryImageView.image = [_BatteryImageArrary objectAtIndex:i32Idx];
}
//-----------------------------------------------------------------------
-(void)CheckFrameStats
{
    //check and try to sync frame
  /*  NSInteger outputFrames = [ProtocolAgent GetShareAgent].VLCplayer.media.numberOfDisplayedPictures;
    
    if(_PrevDisplay!= outputFrames)
    {
        NSTimeInterval now = [NSDate timeIntervalSinceReferenceDate];
        
        int i32FPS = (outputFrames - _PrevDisplay) / (now - _PrevTime);
        
        _RateLabel.text = [NSString stringWithFormat:@"FPS:%d input: %.03f demux:%.03f",
                           i32FPS,
                           [ProtocolAgent GetShareAgent].VLCplayer.media.inputBitrate,
                           [ProtocolAgent GetShareAgent].VLCplayer.media.demuxBitrate];
        
        
        if(i32FPS < FRAME_SYNC_RATE )
        {
            _MissSyncTime += now - _PrevTime;
        }
        else
            _MissSyncTime = 0;
        
        
        if(_MissSyncTime > FRAME_SYNC_TIME)
        {
            //[[ProtocolAgent GetShareAgent] SyncFrame];
            printf("SyncFrame!!! _MissSyncTime = %.02f\n",_MissSyncTime);
            _MissSyncTime = 0;
        }
        
        _PrevTime = now;
        _PrevDisplay = outputFrames;
    }
    
    */
 
}

//-----------------------------------------------------------------------
-(void) StatusTimerHandler:(NSTimer*) timer
{
    
    BYTE *pbyStatus = [[ProtocolAgent GetShareAgent] GetDeviceStatus];
    
    _StatusLabel.text = [NSString stringWithFormat:@"Status:%.2X %.2X %.2x %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X",
                         pbyStatus[0],pbyStatus[1],pbyStatus[2],pbyStatus[3],
                         pbyStatus[4],pbyStatus[5],pbyStatus[6],pbyStatus[7],
                         pbyStatus[8],pbyStatus[9],pbyStatus[10],pbyStatus[11],
                         pbyStatus[12],pbyStatus[13]];
    
    //Need Restart streaming
    if(![ProtocolAgent GetShareAgent].FFplayer.playing)
    {
        if(pbyStatus[1] & 0x80)
        {
            dispatch_async(dispatch_get_main_queue(), ^{
                [[ProtocolAgent GetShareAgent].FFplayer Stop];
            });
            
            double delayInSeconds = 0.3;
            dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, delayInSeconds * NSEC_PER_SEC);
            dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
                [[ProtocolAgent GetShareAgent].FFplayer Play];
            });
        }
    }
    
    //Check mode
    switch((E_DeviceMode)pbyStatus[0])
    {
        case E_DeviceMode_Record:
            if(_CurrentMode != pbyStatus[0])
                [_modeButton setImage:[UIImage imageNamed:@"Mode_DV.png"] forState:UIControlStateNormal];

            if(_audioButton.alpha!=1.0)
            {
                _audioButton.alpha = 1.0;
                _audioButton.enabled = true;
            }
            
            [self ShowMultiShot:FALSE];
            [self AnimateRecord:pbyStatus[1] & 0x01];
            [self ShowMute:!((pbyStatus[1]>>1) & 0x01)];
            
            _ResLabel.text = [[[ProtocolAgent GetShareAgent] GetSettingFileParser] GetValueStrFromSettingFile:RecordResolution_Setting_ID
                                                                                                   ValueIndex:pbyStatus[4]];
            
            
            int i32SecRemain = pbyStatus[5] + (pbyStatus[6]<<8) + (pbyStatus[7] <<16) + (pbyStatus[8] <<24);
            int i32SecLeft = i32SecRemain % 60;
            int i32MinLeft = (i32SecRemain / 60) % 60;
            int i32HourLeft = i32SecRemain / 3600;
            
            _TimeLabel.text  = [NSString stringWithFormat:@"%.02d:%.02d:%.02d",i32HourLeft,i32MinLeft,i32SecLeft];
            
            break;
        case E_DeviceMode_Capture:
            if(_CurrentMode != pbyStatus[0])
                [_modeButton setImage:[UIImage imageNamed:@"Mode_DC.png"] forState:UIControlStateNormal];
            
            if(_audioButton.alpha!=0.2)
            {
                _audioButton.alpha = 0.2;
                _audioButton.enabled = false;
            }
            
            [self ShowMultiShot:pbyStatus[1] & 0x01];
            [self AnimateRecord:FALSE];
            [self ShowMute:FALSE];
            
            _ResLabel.text = [[[ProtocolAgent GetShareAgent] GetSettingFileParser] GetValueStrFromSettingFile:CaptureResolution_Setting_ID
                                                                                                   ValueIndex:pbyStatus[9]];
            
            int i32CaptureRemain = pbyStatus[10] + (pbyStatus[11]<<8) + (pbyStatus[12] <<16) + (pbyStatus[13] <<24);
            _TimeLabel.text  = [NSString stringWithFormat:@"%d",i32CaptureRemain];
            
            break;
        default:
            if(_CurrentMode != pbyStatus[0]) {
                [[ProtocolAgent GetShareAgent] SendSetMode:E_DeviceMode_Record];
            }
            
            [self AnimateRecord:FALSE];
            [self ShowMute:FALSE];
            break;
    }
    
    int byLevel = pbyStatus[2] - ((pbyStatus[2] & 0xff) > 7)*128;
    [self ShowBattery:byLevel IsCharge:pbyStatus[3] & 0x01];
    
    _CurrentMode = pbyStatus[0];
    //renew status
    [[ProtocolAgent GetShareAgent] SendGetStatus];
    
    //[self CheckFrameStats];
}
//-----------------------------------------------------------------------
-(void) ID:(int) i32ID
     Value:(int) i32Value;
{
    if(i32ID == RecordLoopRecording_Setting_ID )
    {
        printf("RecordLoopRecording_Setting_ID %d = %d\n",i32ID,i32Value);
    }
    else if(i32ID == Language_Setting_ID)
    {
        
#ifdef ChangeLanguageDynamically
        printf("Language_Setting_ID %d = %d\n",i32ID,i32Value);
        
        NSString *DeviceLanguage = [[[ProtocolAgent GetShareAgent] GetSettingFileParser] GetValueStrFromSettingFile:i32ID
                                                                                                         ValueIndex:i32Value];
        [LanguageManager saveLanguageByString:DeviceLanguage];
#endif
        
    }
}
//-----------------------------------------------------------------------
- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
    
    m_bFirst = true;
    _CanDisMiss = true;
    m_StatusTimer = nil;
    _CurrentMode = E_DeviceMode_Record;
    
    _PrevTime = 0;
    _PrevDisplay = 0;
    _MissSyncTime = 0;
    _LastSendModeIdx = -1;
    _recordImageView.animationImages = [NSArray arrayWithObjects:
                                        [UIImage imageNamed:@"record_1.png"],
                                        [UIImage imageNamed:@"record_2.png"],
                                        nil];
    
    [_recordImageView setAnimationDuration:1.0f];
    [_recordImageView setAnimationRepeatCount:0];
    [_recordImageView stopAnimating];
    
    
    [[ProtocolAgent GetShareAgent] SetFileMappingInfo:"A=MOVI,avi;J=PICT,jpg;L=LOCK,avi;S=SOS0,avi;W=WAVE,wav"];
    
    _BatteryImageArrary = [NSArray arrayWithObjects:
                           [UIImage imageNamed:@"battery_level0.png"],
                           [UIImage imageNamed:@"battery_level1.png"],
                           [UIImage imageNamed:@"battery_level2.png"],
                           [UIImage imageNamed:@"battery_level3.png"],
                           [UIImage imageNamed:@"battery_level4.png"],
                           [UIImage imageNamed:@"battery_charge.png"],
                           nil];
    
    //pin to top
    _StatusLabel.layer.zPosition = MAXFLOAT;
    _cameraImageView.layer.zPosition = MAXFLOAT;
    _recordImageView.layer.zPosition = MAXFLOAT;
    _muteImageView.layer.zPosition = MAXFLOAT;
    _batteryImageView.layer.zPosition = MAXFLOAT;
    _ResLabel.layer.zPosition = MAXFLOAT;
    _TimeLabel.layer.zPosition = MAXFLOAT;
    _RateLabel.layer.zPosition = MAXFLOAT;
    
    _controllView.layer.zPosition = MAXFLOAT;
    _controllTopView.layer.zPosition = MAXFLOAT;
    
    if (!UIAccessibilityIsReduceTransparencyEnabled())
    {
        _controllView.backgroundColor = [UIColor clearColor];
        _controllTopView.backgroundColor = [UIColor clearColor];
        
        UIBlurEffect *blurEffect = [UIBlurEffect effectWithStyle:UIBlurEffectStyleDark];
        
        UIVisualEffectView *blurEffectView = [[UIVisualEffectView alloc] initWithEffect:blurEffect];
        blurEffectView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
        blurEffectView.frame =  _controllView.bounds;
        [_controllView addSubview:blurEffectView];
        [_controllView sendSubviewToBack:blurEffectView];
        
        UIVisualEffectView *blurEffectView2 = [[UIVisualEffectView alloc] initWithEffect:blurEffect];
        blurEffectView2.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
        blurEffectView2.frame =  _controllTopView.bounds;
        [_controllTopView addSubview:blurEffectView2];
        [_controllTopView sendSubviewToBack:blurEffectView2];
    }
    
    
    VLCTapReger = [[UITapGestureRecognizer alloc]initWithTarget:self action:@selector(VLCViewTapped:)];
    [self.movieView addGestureRecognizer:VLCTapReger];
    
    TopCtrlTapReger = [[UITapGestureRecognizer alloc]initWithTarget:self action:@selector(ControllViewTapped:)];
    CtrlTapReger = [[UITapGestureRecognizer alloc]initWithTarget:self action:@selector(ControllViewTapped:)];
    
    [self.controllTopView addGestureRecognizer:TopCtrlTapReger];
    [self.controllView addGestureRecognizer:CtrlTapReger];
    
    
    _movieView.PassThroughViews = [[NSMutableArray alloc] initWithObjects: _controllView, _controllTopView, nil];
    
    NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
    if(1 == [userDefaults integerForKey:NSUserDefaults_SetTime]) {
        [self setVendorTime];
    }
    
}

-(void)setVendorTime {
    BYTE byStringData[17] = {0};
    byStringData[0] = 0x47;
    byStringData[1] = 0x50;
    byStringData[2] = 0x56;
    byStringData[3] = 0x45;
    byStringData[4] = 0x4E;
    byStringData[5] = 0x44;
    byStringData[6] = 0x4F;
    byStringData[7] = 0x52;
    byStringData[8] = 0x0;
    byStringData[9] = 0x0;
    
    NSDateFormatter* dateFormatter = [[NSDateFormatter alloc] init];
    [dateFormatter setDateFormat:@"yyyyMMddHHmmss"];
    NSString *dateString = [dateFormatter stringFromDate:[NSDate date]];
    
    int iYear = [[dateString substringWithRange:NSMakeRange(0, 4)] intValue];
    
    byStringData[10] = (Byte) iYear;
    byStringData[11] = (Byte) (iYear >> 8);
    byStringData[12] = (Byte) [[dateString substringWithRange:NSMakeRange(4, 2)] intValue];
    byStringData[13] = (Byte) [[dateString substringWithRange:NSMakeRange(6, 2)] intValue];
    byStringData[14] = (Byte) [[dateString substringWithRange:NSMakeRange(8, 2)] intValue];
    byStringData[15] = (Byte) [[dateString substringWithRange:NSMakeRange(10, 2)] intValue];
    byStringData[16] = (Byte) [[dateString substringWithRange:NSMakeRange(12, 2)] intValue];;
    [[ProtocolAgent GetShareAgent]SendVendorCmd:(unsigned char *)byStringData
                                       withSize:17];
}

//------------------------------------------------------------------
-(void) Connection:(BOOL)bIsConnected
{
    if(bIsConnected)
    {
        BYTE *pbyStatus = [[ProtocolAgent GetShareAgent] GetDeviceStatus];
        if (m_bFirst) {
            [[ProtocolAgent GetShareAgent] SendGetParameterFile:PAREMETER_FILE_NAME];
            printf("SendGetParameterFile!\n");
            [[ProtocolAgent GetShareAgent] GetSettingFileParser].bIsDefault = false;
            m_bFirst = false;
            if( 1 == (pbyStatus[2] & 0xff) >> 7) {
                [ProtocolAgent GetShareAgent].m_iRtsp = 1;
                [[ProtocolAgent GetShareAgent] SetFileMappingInfo:GP22_DEFAULT_MAPPING_STR];
                [[ProtocolAgent GetShareAgent] SendCheckFileMapping];
            }
            else {
                [ProtocolAgent GetShareAgent].m_iRtsp = 0;
                [[ProtocolAgent GetShareAgent] SetFileMappingInfo:DEFAULT_MAPPING_STR];
            }
        }
        
        if( 1 == (pbyStatus[3] & 0xff) >> 7) {
            [ProtocolAgent GetShareAgent].m_bCanDeleteSDFile = true;
        }
        else {
            [ProtocolAgent GetShareAgent].m_bCanDeleteSDFile = false;
        }
        [[ProtocolAgent GetShareAgent] ResetStreamingURL];
        [[ProtocolAgent GetShareAgent] SendRestartStreaming];
        
        double delayInSeconds = 2.0;
        dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, delayInSeconds * NSEC_PER_SEC);
        dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
            [self ButtonsEnable:true];
            [self StatusTimer:true];
        });
    }
    else
    {
        [self StatusTimer:false];
        [self ButtonsEnable:false];
    }
}
//------------------------- ----------------------------------------
- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
    _PrevTime = [NSDate timeIntervalSinceReferenceDate];
    _PrevDisplay = 0;
    _LoopRecordID = -1;
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(ConnectionChangeNotification:)
                                                 name:Notification_Device_ConnectionChange
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(SetModeCompleteNotification:)
                                                 name:Notification_Device_SetModeComplete
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(CommandFailed:)
                                                 name:Notification_Device_CommandFailed
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(ShowPIPButton:)
                                                 name:Notification_Device_ShowPIPButton
                                               object:nil];
    
    
    [[ProtocolAgent GetShareAgent] ClearCmdQueue];
    [ProtocolAgent GetShareAgent].ActiveView = self.view;
    [ProtocolAgent GetShareAgent].GetParameterDelegate = self;
    [ProtocolAgent GetShareAgent].FFplayer.DrawView = _movieView;
    [[ProtocolAgent GetShareAgent].FFplayer SetStreaming:true];
    
    NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
    BOOL bSetDiscardcorrupt = (BOOL)[userDefaults boolForKey:NSUserDefaults_SetDiscardcorrupt];
    NSString* strTemp = @"";
    if (bSetDiscardcorrupt) {
        strTemp = @"fflags=discardcorrupt;";
    }
    [[ProtocolAgent GetShareAgent].FFplayer UserOptions:strTemp]; // Keep connection after end of streaming.
    
    if([[ProtocolAgent GetShareAgent] GetConnectionStatus]== E_ConnectionStatus_Connected)
    {
        if(_CurrentMode == E_DeviceMode_Menu || _CurrentMode ==E_DeviceMode_Playback)
            _CurrentMode = E_DeviceMode_Record;
        
        _LastSendModeIdx = [[ProtocolAgent GetShareAgent]SendSetMode:_CurrentMode];
        [[ProtocolAgent GetShareAgent] SendGetStatus];
    }
    else if([[ProtocolAgent GetShareAgent] GetConnectionStatus]== E_ConnectionStatus_DisConnected)
    {
        [self PressDisconnect:nil];
    }
    
    [self Connection:false];
    
    BOOL bLogEnable = false;
    id LogObject  = [[NSUserDefaults standardUserDefaults] objectForKey:@"log_enable"];
    if(LogObject)
        bLogEnable = [[NSUserDefaults standardUserDefaults] boolForKey:@"log_enable"];
    
    _logButton.hidden = !bLogEnable;
    
    double delayInSeconds = 10.0;
    dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, delayInSeconds * NSEC_PER_SEC);
    dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
        
        //Make sure disconnect is available no matter what happens.
        _connectButton.enabled = true;
        _connectButton.alpha = 1.0;
    });

    _LoopRecordID = [[ProtocolAgent GetShareAgent] SendGetParameter:RecordLoopRecording_Setting_ID];
    
    [[ProtocolAgent GetShareAgent] SendGetSetPIP:0];
#ifdef ChangeLanguageDynamically
    [[ProtocolAgent GetShareAgent] SendGetParameter:Language_Setting_ID];
#endif
}
//------------------------- ----------------------------------------
- (void)viewWillDisappear:(BOOL)animated
{
    [super viewWillDisappear:animated];
    [ProtocolAgent GetShareAgent].GetParameterDelegate = false;
    if([[ProtocolAgent GetShareAgent] GetConnectionStatus]== E_ConnectionStatus_Connected)
        [self Connection:false];
    
    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:Notification_Device_ConnectionChange
                                                  object:nil];
    
    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:Notification_Device_SetModeComplete
                                                  object:nil];
    
    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:Notification_Device_CommandFailed
                                                  object:nil];
    
    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:Notification_Device_ShowPIPButton
                                                  object:nil];
    
    [self AnimateRecord:FALSE];
}
//-----------------------------------------------------------------------
- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}
//-----------------------------------------------------------------
-(UIStatusBarStyle)preferredStatusBarStyle {
    return UIStatusBarStyleLightContent; 
}
//-----------------------------------------------------------------
- (BOOL)prefersStatusBarHidden {
    
    return true;
}
//-----------------------------------------------------------------------
@end
