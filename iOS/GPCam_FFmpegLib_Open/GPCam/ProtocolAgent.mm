//
//  ProtocolAgent.m
//  GPCam
//
//  Created by generalplus_sa1 on 8/20/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//
#import <SystemConfiguration/CaptiveNetwork.h>
#import "ProtocolAgent.h"
#import "GPCamCommandAPI.h"
#import "MBProgressHUD.h"
#import "LogDataHelper.h"

#define kVLCSettingNetworkCaching @"network-caching"
#define kVLCSettingNetworkCachingDefaultValue @(100)

#define kVLCSettingMjpegFPS @"mjpeg-fps"
#define kVLCSettingMjpegFPSDefaultValue @(30.0)

#define kVLCSettingLogStats @"--stats"

NSString *Notification_Device_SetModeComplete           = @"Notification_Device_SetModeComplete";
NSString *Notification_Device_ConnectionChange          = @"Notification_Device_ConnectionChange";
NSString *Notification_Device_SettingChange             = @"Notification_Device_SettingChange";
NSString *Notification_Device_CommandFailed             = @"Notification_Device_CommandFailed";
NSString *Notification_Device_PlaybackStopComplete      = @"Notification_Device_PlaybackStopComplete";
NSString *Notification_Device_GetSettingComplete        = @"Notification_Device_GetSettingComplete";
NSString *Notification_Device_ShowPIPButton             = @"Notification_Device_ShowPIPButton";

NSString *ErrorCodeString[] =
{
    NSLocalizedString(@"Server Is Busy!", @""),                     //-1
    NSLocalizedString(@"Invalid Command!", @""),                     //-2
    NSLocalizedString(@"Request TimeOut!", @""),                     //-3
    NSLocalizedString(@"Mode Error!", @""),                     //-4
    NSLocalizedString(@"No Storage!", @""),                     //-5
    NSLocalizedString(@"Write Failed", @""),                     //-6
    NSLocalizedString(@"Get File List Failed!", @""),                     //-7
    NSLocalizedString(@"Get Thumbnail Failed!", @""),                     //-8
    NSLocalizedString(@"Full Storage!", @""),                     //-9
    NSLocalizedString(@"Battery Low!", @""),                    //-10
    NSLocalizedString(@"Mem malloc error!", @""),                    //-11
    NSLocalizedString(@"Checksum error!", @""),                    //-12
    NSLocalizedString(@"No File!", @""),                    //-13
};

typedef enum
{
    E_Protocol_Status_Connect ,
    E_Protocol_Status_Command ,
    E_Protocol_Status_Progress ,
    E_Protocol_Status_MutiCommand ,
    E_Protocol_Status_CommandFinish ,
    E_Protocol_Status_CommandFailed
    
} E_Protocol_Status;

typedef enum
{
    E_Data_Status_initialization ,
    E_Data_Status_FirstGetSetMode ,
    E_Data_Status_AfterGetStatus
    
} E_Data_Status;
//-----------------------------------------------------------------------
int SocketDataCallBack(bool bIsWrite ,int i32DataSize ,BYTE* pbyData)
{
    @autoreleasepool {
        
    NSString *Message;
    
    if(bIsWrite)
        Message = @"Send";
    else
        Message = @"Get";

        [[LogDataHelper GetShareHelper] LogMessage:Message
                                      WithIntArray:pbyData
                                        WithLength:i32DataSize];
    }

    return 0;
}

//-----------------------------------------------------------------------
bool ReadDefaultMenu()
{
    [[ProtocolAgent GetShareAgent] GetSettingFileParser].bIsDefault = true;
    NSString *filePath = [[NSBundle mainBundle] pathForResource: DEFAULT_PAREMETER_FILE_NAME ofType: @"xml"];
    if(![[[ProtocolAgent GetShareAgent] GetSettingFileParser] Parsefile:filePath])
        return false;
    
    printf("Failed to read menu which downloaded from device, Using default menu!\n");
    
    return true;
}
//-----------------------------------------------------------------------
int HandleMode(int i32CMDIndex,int i32Type,int i32Mode,int i32CMDID, int i32DataSize ,BYTE* pbyData)
{
    
    switch(i32Mode)
    {
        case GPSOCK_MODE_General:
        {
            if(i32CMDID == GPSOCK_General_CMD_SetMode)
            {
                if (E_Data_Status_AfterGetStatus == [ProtocolAgent GetShareAgent].getDataStatus) {
                    [[NSNotificationCenter defaultCenter] postNotificationName:Notification_Device_SetModeComplete object:nil];
                    NSLog(@"GPSOCK_General_CMD_SetMode  E_Data_Status_AfterGetStatus");
                }
                else {
                    [ProtocolAgent GetShareAgent].getDataStatus = E_Data_Status_FirstGetSetMode;
                    NSLog(@"GPSOCK_General_CMD_SetMode  E_Data_Status_FirstGetSetMode");
                }

            }
            else if(i32CMDID == GPSOCK_General_CMD_GetDeviceStatus)
            {
                [[ProtocolAgent GetShareAgent] SetDeviceStatus:pbyData];
                if (E_Data_Status_initialization == [ProtocolAgent GetShareAgent].getDataStatus) {
                    NSLog(@"GPSOCK_General_CMD_GetDeviceStatus  E_Data_Status_initialization -> E_Data_Status_AfterGetStatus");
                    [ProtocolAgent GetShareAgent].getDataStatus = E_Data_Status_AfterGetStatus;
                }
                if (E_Data_Status_FirstGetSetMode == [ProtocolAgent GetShareAgent].getDataStatus) {
                    NSLog(@"GPSOCK_General_CMD_GetDeviceStatus  E_Data_Status_FirstGetSetMode -> E_Data_Status_AfterGetStatus");
                    [[NSNotificationCenter defaultCenter] postNotificationName:Notification_Device_SetModeComplete object:nil];
                    [ProtocolAgent GetShareAgent].getDataStatus = E_Data_Status_AfterGetStatus;
                }
                return 0; //No hub, do not nofity ack
            }
            else if(i32CMDID == GPSOCK_General_CMD_GetParameterFile)
            {
                
                NSString *docsPath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) lastObject];
                NSString *filePath = [docsPath stringByAppendingPathComponent:[NSString stringWithCString:PAREMETER_FILE_NAME encoding:NSUTF8StringEncoding]];
                if(![[[ProtocolAgent GetShareAgent] GetSettingFileParser] Parsefile:filePath])
                {
                    if(!ReadDefaultMenu())
                        [[ProtocolAgent GetShareAgent] NotifyCommandAck:true WithErrorCode:Error_InvalidCommand WithCmdIndex:i32CMDIndex];
                    return 0;
                }
            }
            else if(i32CMDID == GPSOCK_General_CMD_RestartStreaming)
            {
                //[[ProtocolAgent GetShareAgent].FFplayer Play];
            }
            else if(i32CMDID == GPSOCK_General_CMD_CheckMapping)
            {
                [[ProtocolAgent GetShareAgent] SetFileMappingInfo:DEFAULT_MAPPING_STR];
            }
            else if(i32CMDID == GPSOCK_General_CMD_GetSetPIP)
            {
                [[NSNotificationCenter defaultCenter] postNotificationName:Notification_Device_ShowPIPButton object:nil];
            }
        }
            break;
        case GPSOCK_MODE_Record:
        {
            
        }
            break;
        case GPSOCK_MODE_Playback:
        {
            if(i32CMDID == GPSOCK_Playback_CMD_GetFileCount)
            {
                int i32FileCount = pbyData[0];
                i32FileCount |= pbyData[1]<<8;
                
                if([ProtocolAgent GetShareAgent].PlaybackDelegate)
                    [[ProtocolAgent GetShareAgent].PlaybackDelegate GetFileCount:i32FileCount];
            }
            else if(i32CMDID == GPSOCK_Playback_CMD_GetNameList)
            {
                int i32FileIndex = pbyData[0]+ (pbyData[1]<<8);
                int i32Count = pbyData[2];
                
                if([ProtocolAgent GetShareAgent].PlaybackDelegate)
                    [[ProtocolAgent GetShareAgent].PlaybackDelegate GetFileName:i32FileIndex
                                                                       withSize:i32Count ];
                
            }
            else if(i32CMDID == GPSOCK_Playback_CMD_GetThumbnail)
            {
                char FilePath[256];
                int i32FileIndex = pbyData[0]+ (pbyData[1]<<8);
                int i32Len = pbyData[2]+ (pbyData[3]<<8);
                FilePath[i32Len] = 0;
                memcpy(FilePath,&pbyData[4],i32Len);
                
                if([ProtocolAgent GetShareAgent].PlaybackDelegate)
                    [[ProtocolAgent GetShareAgent].PlaybackDelegate FileThumbnailIsReady:i32FileIndex
                                                                       withThumbnailName:[NSString stringWithCString:FilePath encoding:NSUTF8StringEncoding]];
                
                return 0; //Thumbnail no hub, do not nofity ack
                
            }
            else if(i32CMDID == GPSOCK_Playback_CMD_GetRawData)
            {
                char FilePath[256];
                int i32Finish = pbyData[0];
                if(i32Finish==0x01)
                {
                    int i32FileIndex = pbyData[1]+ (pbyData[2]<<8);
                    int i32Len = pbyData[3]+ (pbyData[4]<<8);
                    FilePath[i32Len] = 0;
                    memcpy(FilePath,&pbyData[5],i32Len);
                    
                    if([ProtocolAgent GetShareAgent].PlaybackDelegate)
                        [[ProtocolAgent GetShareAgent].PlaybackDelegate FileRawData:i32FileIndex
                                                                       withFileName:[NSString stringWithCString:FilePath encoding:NSUTF8StringEncoding]];
                }
                else
                {
                    float fPercent = pbyData[1];
                    [ProtocolAgent GetShareAgent].progress = fPercent / 100;
                    
                    return 0; // Not finish yet , this just progress .
                }
                
            }
            else if(i32CMDID == GPSOCK_Playback_CMD_Stop)
            {
                [[ProtocolAgent GetShareAgent].FFplayer Stop];
            }
            else if(i32CMDID == GPSOCK_Playback_CMD_DeleteFile)
            {
                if([ProtocolAgent GetShareAgent].PlaybackDelegate)
                    [[ProtocolAgent GetShareAgent].PlaybackDelegate FileDeleted:true];
            }
            
        }
            break;
        case GPSOCK_MODE_Menu:
        {
            if(i32CMDID == GPSOCK_Menu_CMD_GetParameter)
            {
                bool bIsOut = false;
                int i32ID = pbyData[0] + (pbyData[1] <<8) + (pbyData[2] <<16) + (pbyData[3] <<24);
                
                SettingFileParser* FileParser = [[ProtocolAgent GetShareAgent] GetSettingFileParser];
                
                for (CategoriesItem *CategoriesI in FileParser.Categories)
                {
                    for (SettingsItem *SettingsI in CategoriesI.Settings)
                    {
                        if(SettingsI.ID == i32ID)
                        {
//                            if(SettingsI.ID == 0x00000209) {
//                                char StringValus[256];
//                                int i32Len = i32DataSize - sizeof(i32ID);
//                                StringValus[i32Len] = 0;
//                                memcpy(StringValus,&pbyData[4],i32Len);
////                                NSMutableArray* temp = [[NSMutableArray alloc] init];
////                                [temp addObject:@"12314134"];
//                                ValuesItem *NewValueI = [SettingsI.Values objectAtIndex:0];
//                                NewValueI.Name = @"12314134";
//                            }
                                
                            if(SettingsI.Type == SETTING_TYPE_VALUES)
                            {
                                SettingsI.Current = pbyData[4];
                                bIsOut =true;
                            }
                            else if(SettingsI.Type == SETTING_TYPE_INPUTSTRING)
                            {
                                char StringValus[256];
                                int i32Len = i32DataSize - sizeof(i32ID);
                                StringValus[i32Len] = 0;
                                memcpy(StringValus,&pbyData[4],i32Len);
                                SettingsI.ValueString = [NSString stringWithCString:StringValus encoding:NSUTF8StringEncoding];
                            }
     
                            break;
                        }
                    }
                    
                    if(bIsOut)
                        break;
                }
                
                if([ProtocolAgent GetShareAgent].GetParameterDelegate)
                {
                    [[ProtocolAgent GetShareAgent].GetParameterDelegate ID:i32ID
                                                                     Value:pbyData[4]];

                }
                
            }
        }
            break;
            
        case GPSOCK_MODE_Firmware:
        {
            
            if(i32CMDID == GPSOCK_Firmware_CMD_Download)
            {
                if([ProtocolAgent GetShareAgent].FirmwareDownloadDelegate)
                   [[ProtocolAgent GetShareAgent].FirmwareDownloadDelegate DownloadComplete];
            }
            else if(i32CMDID == GPSOCK_Firmware_CMD_SendRawData)
            {
                if([ProtocolAgent GetShareAgent].FirmwareDownloadDelegate)
                   [[ProtocolAgent GetShareAgent].FirmwareDownloadDelegate RawDataComplete];
                
                return 0; //Not complete do not nofity ack
            }
            else if(i32CMDID == GPSOCK_Firmware_CMD_Upgrade)
            {
                if([ProtocolAgent GetShareAgent].FirmwareDownloadDelegate)
                   [[ProtocolAgent GetShareAgent].FirmwareDownloadDelegate UpgradeComplete];
            }
        }
            break;
            
        case GPSOCK_MODE_Vendor:
        {
            if([ProtocolAgent GetShareAgent].VendorDelegate)
                [[ProtocolAgent GetShareAgent].VendorDelegate GetData:pbyData
                                                             withSize:i32DataSize];
        }
            break;
            
    }
    
    [[ProtocolAgent GetShareAgent] NotifyCommandAck:true WithErrorCode:0 WithCmdIndex:i32CMDIndex];

    return 0;
}
//-----------------------------------------------------------------------
int HandleNak(int i32CMDIndex,int i32Type,int i32Mode,int i32CMDID, int i32DataSize ,BYTE* pbyData)
{
    
    switch(i32Mode)
    {
        case GPSOCK_MODE_General:
        {
            if(i32CMDID == GPSOCK_General_CMD_SetMode)
            {
            }
            else if(i32CMDID == GPSOCK_General_CMD_GetDeviceStatus)
            {
            }
            else if(i32CMDID == GPSOCK_General_CMD_GetParameterFile)
            {
                if(ReadDefaultMenu())
                    return 0;
            }
            else if(i32CMDID == GPSOCK_General_CMD_RestartStreaming)
            {

            }
            else if(i32CMDID == GPSOCK_General_CMD_GetSetPIP)
            {
                NSLog(@"Nak GPSOCK_General_CMD_GetSetPIP");
                return 0;
            }
        }
            break;
        case GPSOCK_MODE_Record:
        {
            
        }
            break;
        case GPSOCK_MODE_Playback:
        {
            if(i32CMDID == GPSOCK_Playback_CMD_GetFileCount)
            {
                
            }
            else if(i32CMDID == GPSOCK_Playback_CMD_GetNameList)
            {
                
            }
            else if(i32CMDID == GPSOCK_Playback_CMD_GetThumbnail)
            {
                
            }
            else if(i32CMDID == GPSOCK_Playback_CMD_GetRawData)
            {
                
            }
            else if(i32CMDID == GPSOCK_Playback_CMD_Stop)
            {
                [[ProtocolAgent GetShareAgent].FFplayer Stop];
            }
            else if(i32CMDID == GPSOCK_Playback_CMD_DeleteFile)
            {
                if([ProtocolAgent GetShareAgent].PlaybackDelegate)
                    [[ProtocolAgent GetShareAgent].PlaybackDelegate FileDeleted:false];
            }
            
        }
            break;
        case GPSOCK_MODE_Menu:
        {
            if(i32CMDID == GPSOCK_Playback_CMD_GetFileCount)
            {
                
            }
            else if(i32CMDID == GPSOCK_Playback_CMD_GetNameList)
            {
                
            }
        }
            break;
            
        case GPSOCK_MODE_Firmware:
        {
        }
            break;
            
        case GPSOCK_MODE_Vendor:
        {
        }
            break;
    }
    

    
    short ErrorCode = pbyData[0];
    ErrorCode |= (pbyData[1]<<8);
    
    if([ProtocolAgent GetShareAgent].ignoreIndex == i32CMDIndex && ErrorCode != Error_LostConnection)
    {
        [ProtocolAgent GetShareAgent].ignoreIndex = -1;
        return 0; //ignore this command
    }
    
    NSArray *PassObjs = [[NSArray alloc]initWithObjects:[NSNumber numberWithInt:i32CMDIndex],[NSNumber numberWithInt:ErrorCode],nil];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:Notification_Device_CommandFailed
                                                        object:PassObjs];
    
    [[ProtocolAgent GetShareAgent] NotifyCommandAck:false WithErrorCode:ErrorCode WithCmdIndex:i32CMDIndex];
    
    return 0;
}
//-----------------------------------------------------------------------
int AgentCallBack(int i32CMDIndex,int i32Type,int i32Mode,int i32CMDID, int i32DataSize ,BYTE* pbyData)
{
    @autoreleasepool {
    
        [[LogDataHelper GetShareHelper]LogMessage:@"---------------------------"];
        
        switch(i32Type)
        {
            case GP_SOCK_TYPE_ACK:
            {
                HandleMode(i32CMDIndex,i32Type,i32Mode,i32CMDID,i32DataSize,pbyData);
            }
                break;
            case GP_SOCK_TYPE_NAK:
            {
                HandleNak(i32CMDIndex,i32Type,i32Mode,i32CMDID,i32DataSize,pbyData);
            }
                break;
            default:
                break;
        }
    }
    return 0;
}

//------------------------------------------------------------------------------------------------
@interface ProtocolAgent ()<FFmpegPlayerDelegate,MBProgressHUDDelegate>
{
    SettingFileParser       *_FileParser;
    MBProgressHUD           *HUD;
    E_Protocol_Status       _Status;
    int                     _i32CurCMDIndex;
    int                     _i32MutiCmdCnt;
    int                     _deviceMode;
    short                   _ErrorCode;
    NSString                *_MutiCmdNofify;
    BYTE                    _DeviceStatus[DEVICE_STATUS_SIZE];
    __block bool            _HubFinish;
    
    NSTimeInterval          _progress_prevTime;
    float                   _progress_percent;
}

@end
//------------------------------------------------------------------------------------------------
@implementation ProtocolAgent
//------------------------------------------------------------------------------------------------
-(id)init
{
    if ((self = [super init])) {
        _FileParser = [[SettingFileParser alloc]init];
        
        printf("Library Version: %s\n",GPCam_Version());
        GPCam_SetCmdStatusCallBack(&AgentCallBack);
        GPCam_SetDataCallBack(&SocketDataCallBack);
        HUD = nil;
        _i32MutiCmdCnt = 0;
        _ignoreIndex = -1;
        _MutiCmdNofify = nil;
        _deviceMode = -1;
        _PlayerStatus = FFMPEG_STATUS_STOPPED;

        
        _FFplayer = [[FFmpegPlayer alloc]init];
        
        _FFplayer.delegate = self;
        _PlaybackDelegate = nil;
        _VendorDelegate = nil;
        _FirmwareDownloadDelegate = nil;
        _AlertViewController = nil;
        _HubFinish = false;
        _GetParameterDelegate = nil;
        _m_iRtsp = -1;
        _m_bCanDeleteSDFile = false;
        _m_strCommandIP = @"";
        memset(_DeviceStatus,0x00,sizeof(_DeviceStatus));
        
    }
    return self;
    
}

static id sharedInstance;
//------------------------------------------------------------------------------------------------
+ (void) initialize {
    // subclassing would result in an instance per class, probably not what we want
    NSAssert([ProtocolAgent class] == self, @"Subclassing is not welcome");
    sharedInstance = [[super alloc] init];
}
//------------------------------------------------------------------------------------------------
+(ProtocolAgent*) GetShareAgent;
{
    return sharedInstance;
}
//------------------------------------------------------------------------------------------------
-(SettingFileParser*)   GetSettingFileParser
{
    return _FileParser;
}
//------------------------------------------------------------------------------------
-(void) StatusChange:(int)i32Status
{
    printf("FFmpeg StatusChange: %d\n",i32Status);
    _PlayerStatus = i32Status;
    
    if(i32Status == FFMPEG_STATUS_STOPPED)
    {

    }
    else if(i32Status == FFMPEG_STATUS_PLAYING)
    {
        _Status = E_Protocol_Status_CommandFinish;
    }
    else if(i32Status == FFMPEG_STATUS_BUFFERING)
    {
        [self CreateProgressView:E_Protocol_Status_Command withLabel:NSLocalizedString(@"Buffering", @"")];
    }
}
//------------------------------------------------------------------------------------------------
-(void) NotifyCommandAck:(bool) bIsAck
           WithErrorCode:(short)ErrorCode
            WithCmdIndex:(int)i32Index
{
    bool isFinish = true;
    if(_Status == E_Protocol_Status_MutiCommand)
    {
        _i32MutiCmdCnt--;
        if(_i32MutiCmdCnt>=0)
        {
            if (_i32MutiCmdCnt != 0) {
                isFinish = false;
            }
            if(_MutiCmdNofify)
                [[NSNotificationCenter defaultCenter] postNotificationName:_MutiCmdNofify object:nil];
            
            if(0 == _i32MutiCmdCnt) {
                [[NSNotificationCenter defaultCenter] postNotificationName:Notification_Device_GetSettingComplete object:nil];
            }
        }
        
    }
    
    if(isFinish)
    {
        if(bIsAck)
        {
            if(E_Protocol_Status_Progress == _Status)
            {
                if(i32Index == _i32CurCMDIndex)
                    _Status = E_Protocol_Status_CommandFinish;
            }
            else
                _Status = E_Protocol_Status_CommandFinish;
            
            printf("Command Finish\n");
        }
        else if(!bIsAck)
        {
            _ErrorCode = ErrorCode;
            _Status = E_Protocol_Status_CommandFailed;
            printf("Command Failed\n");
            
            if(_ErrorCode == Error_LostConnection)
            {
                [[NSNotificationCenter defaultCenter] postNotificationName:Notification_Device_ConnectionChange
                                                                    object:@"DisConnected"];
            }
            
        }
    }
}
//------------------------------------------------------------------------------------------------
-(void) SetDeviceStatus:(BYTE*)pbyStatus
{
    memcpy(_DeviceStatus , pbyStatus , sizeof(_DeviceStatus));
}
//------------------------------------------------------------------------------------------------
-(BYTE*) GetDeviceStatus
{
    return _DeviceStatus;
}
//------------------------------------------------------------------------------------------------
-(void)setActiveView:(UIView *)newView
{
    _ActiveView = newView;
    HUD = nil;
}
//------------------------------------------------------------------------------------------------
-(void)setProgress:(float)progress
{
    if(HUD!=nil)
    {
        NSTimeInterval escTime = [NSDate timeIntervalSinceReferenceDate] - _progress_prevTime;
        float fescCent = progress - _progress_percent;
        
        HUD.progress = progress;
        
        if(progress != _progress_percent)
        {

            float fleft = 1.0 - progress;
            float ftimepercent = escTime / fescCent;
            float flefttime = ftimepercent*fleft;
  
            //HUD.labelText = [NSString stringWithFormat:@"Downloading %.f %% Left: %.0fM %.0fS",progress*100,flefttime/60,fmod(flefttime,60)];
            HUD.labelText = [NSString localizedStringWithFormat:NSLocalizedString(@"Downloading %.f%%", @""), progress*100];
            _progress_prevTime = [NSDate timeIntervalSinceReferenceDate];
            _progress_percent = progress;
        }
    }
}
//------------------------------------------------------------------------------------------------
-(float)getProgress
{
    if(HUD!=nil)
    {
        return HUD.progress;
    }
    else
        return 0.0;
}
//------------------------------------------------------------------------------------------------
-(NSString*) GetFileName:(int) i32Index
{
    return [NSString stringWithCString:GPCam_GetFileName(i32Index) encoding:NSUTF8StringEncoding];
}
//------------------------------------------------------------------------------------------------
-(int)  GetFileIndex:(int) i32Index
{
    return GPCam_GetFileIndex(i32Index);
}
//------------------------------------------------------------------------------------------------
-(bool) GetFileTime:(int) i32Index
          withTimeBuffer:(BYTE*)pbyTime
{
    return GPCam_GetFileTime(i32Index, pbyTime);
}
//------------------------------------------------------------------------------------------------
-(unsigned int) GetFileSize:(int) i32Index
{
    return GPCam_GetFileSize(i32Index);
}
//------------------------------------------------------------------------------------------------
-(BYTE*) GetFileExtraInfo:(int) i32Index
         withSizeReadBack:(int*)pi32Size
{
    return GPCam_GetFileExtraInfo(i32Index,pi32Size);
}
//------------------------------------------------------------------------------------------------
-(E_ConnectionStatus) GetConnectionStatus
{
    return GPCam_GetStatus();
}
//-----------------------------------------------------------------
- (NSString *)currentWifiSSID {
    // Does not work on the simulator.
    NSString *ssid = @"device";
    NSArray *ifs = (__bridge_transfer id)CNCopySupportedInterfaces();
    for (NSString *ifnam in ifs) {
        NSDictionary *info = (__bridge_transfer id)CNCopyCurrentNetworkInfo((__bridge CFStringRef)ifnam);
        if (info[@"SSID"]) {
            ssid = info[@"SSID"];
        }
    }
    return ssid;
}
//------------------------------------------------------------------------------------------------
-(void)ShowFailedConnectAlert
{
    dispatch_async(dispatch_get_main_queue(), ^{
    
        if(_AlertViewController)
        {
    
            UIAlertController *alertController = [UIAlertController
                                                  alertControllerWithTitle:[NSString stringWithFormat:@"%@ %@ %@!",
                                                                            NSLocalizedString(@"Connect to", @""),
                                                                            [self currentWifiSSID],
                                                                            NSLocalizedString(@"failed", @"")]
                                                  message:NSLocalizedString(@"Please connect to an authorized Wifi sport Cam device or\n retry to restart the sport Cam first.", @"")
                                                  preferredStyle:UIAlertControllerStyleAlert];
        
            UIAlertAction *okAction = [UIAlertAction
                                       actionWithTitle:NSLocalizedString(@"OK", @"OK action")
                                       style:UIAlertActionStyleDefault
                                       handler:^(UIAlertAction *action)
                                       {
                                           NSLog(@"OK action");
                                       }];
        
            [alertController addAction:okAction];
            [_AlertViewController presentViewController:alertController animated:YES completion:nil];
        }
    });
}
//------------------------------------------------------------------------------------------------
-(void)ConnectCheck
{
    switch(GPCam_GetStatus())
    {
        case E_ConnectionStatus_Idle:
        {
        }
            break;
        case E_ConnectionStatus_Connecting:
        {
        }
            break;
        case E_ConnectionStatus_Connected:
        {
            
            NSString *docsPath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) lastObject];
            
            [[ProtocolAgent GetShareAgent] SetDownloadPath:[docsPath UTF8String]];
            [[ProtocolAgent GetShareAgent] SendGetStatus];
//            usleep(300000); //0.3 Sec
//            printf("SendGetParameterFile!\n");
//            [[ProtocolAgent GetShareAgent] SendGetParameterFile:PAREMETER_FILE_NAME];
//            
//            [[ProtocolAgent GetShareAgent] GetSettingFileParser].bIsDefault = false;
            
            [[NSNotificationCenter defaultCenter] postNotificationName:Notification_Device_ConnectionChange
                                                                object:@"Connected"];
            
            HUD.labelText = NSLocalizedString(@"Connected", @"");
            printf("Connected!\n");
            
        }
            break;
        case E_ConnectionStatus_DisConnected:
        {
            printf("Failed to connect!\n");
            HUD.labelText = NSLocalizedString(@"Failed to connect", @"");
            _HubFinish = true;
            
            //[self ShowFailedConnectAlert];
            _Status = E_Protocol_Status_CommandFinish;
            
            [[NSNotificationCenter defaultCenter] postNotificationName:Notification_Device_ConnectionChange
                                                                object:@"DisConnected"];
        }
            break;
    }
    
}
//------------------------------------------------------------------------------------------------
- (void)ProgressViewTask
{
    _HubFinish = false;
    
    while(!_HubFinish)
    {
        switch(_Status)
        {
            case E_Protocol_Status_Connect:
            {
                [self ConnectCheck];
            }
                break;
            case E_Protocol_Status_MutiCommand:
            {
                HUD.labelText = [NSString stringWithFormat:@"%@: %d",NSLocalizedString(@"Command left", @""),_i32MutiCmdCnt];
            }
            case E_Protocol_Status_Command:
            case E_Protocol_Status_Progress:
            {
                if(GPCam_GetStatus() == E_ConnectionStatus_DisConnected)
                {
                    [[NSNotificationCenter defaultCenter] postNotificationName:Notification_Device_ConnectionChange
                                                                        object:@"DisConnected"];
                    _HubFinish = true;
                }
            }
                break;
                
            case E_Protocol_Status_CommandFinish:
            {
                _HubFinish = true;
            }
                break;
            case E_Protocol_Status_CommandFailed:
            {
                if(_ErrorCode == Error_ModeError) {
                    _HubFinish = true;
                    break;
                }
                
                if(_ErrorCode <= Error_LostConnection)
                    HUD.labelText = [NSString stringWithFormat:@"%@: %d",
                                     NSLocalizedString(@"Lost connection! Command failed", @""),_ErrorCode];
                else
                {
                    if(_ErrorCode < Error_MAXDeviceErrorCode)
                        HUD.labelText = [NSString stringWithFormat:@"%@ %d",NSLocalizedString(@"Failed! Unknown error code", @""),_ErrorCode];
                    else
                        HUD.labelText = [NSString stringWithFormat:@"%@! %@",NSLocalizedString(@"Failed", @""),ErrorCodeString[abs(_ErrorCode)-1]];
                }
                
                if(HUD.labelText != nil) {
                    if (HUD.labelText.length > 0) {
                        [[LogDataHelper GetShareHelper]LogMessage:HUD.labelText];
                    }
                }
                
                _HubFinish = true;
                usleep(2000000); //2.0 Sec
            }
                break;
        }
        
        if(!_HubFinish)
            usleep(250000); //0.25Sec
    }

}
//------------------------------------------------------------------------------------------------
- (void)hudWasHidden:(MBProgressHUD *)hud {
    // Remove HUD from screen when the HUD was hidded
    [HUD removeFromSuperview];
    HUD = nil;
}
//------------------------------------------------------------------------------------------------
-(void) CreateProgressView:(E_Protocol_Status) eStatus
                 withLabel:(NSString*) Label
{
    printf("Command Status:%d\n",eStatus);
    _Status = eStatus;
    
    dispatch_async(dispatch_get_main_queue(), ^{
        
        bool bNewHud = true;
        
        if(HUD!=nil)
            bNewHud = false;
        
        if(bNewHud)
        {
            HUD = [[MBProgressHUD alloc] initWithView:_ActiveView];
            [_ActiveView addSubview:HUD];
            HUD.delegate = self;

        }
        
        if(eStatus == E_Protocol_Status_Progress)
        {
            _progress_prevTime = 0;
            _progress_percent = 0;
            
            HUD.mode = MBProgressHUDModeDeterminate;
            
            if(bNewHud)
            {
                HUD.progress = 0.0;
                HUD.abortBlock =  ^{
                    _HubFinish = true;
                    [self Abort:_i32CurCMDIndex];
                };
                HUD.labelText = Label;
            }
        }
        else
        {
            HUD.mode = MBProgressHUDModeIndeterminate;
            HUD.labelText = Label;
        }
        
        [[LogDataHelper GetShareHelper]LogMessage:Label];
        
        if(bNewHud)
            [HUD showWhileExecuting:@selector(ProgressViewTask) onTarget:self withObject:nil animated:YES];
    });

}
//------------------------------------------------------------------------------------------------
-(void) ResetStreamingURL
{

    NSString *Stringurl = nil;
    id Object  = [[NSUserDefaults standardUserDefaults] objectForKey:@"new_streaming_url"];
    if(Object)
    {
        Stringurl = [[NSUserDefaults standardUserDefaults] stringForKey:@"new_streaming_url"];
    }
    else
    {
        if(1 == _m_iRtsp) {
            Stringurl = [NSString stringWithFormat:RTSP_STREAMING_URL,_m_strCommandIP];
        }
        else if (0 == _m_iRtsp){
            Stringurl = [NSString stringWithFormat:STREAMING_URL,_m_strCommandIP];
        }
    }

    [_FFplayer VideoPath:Stringurl];

}
//------------------------------------------------------------------------------------------------
-(void)SyncFrame
{
    /*static bool s_IsSyncing = false;
    
    if(!s_IsSyncing)
    {
        [[ProtocolAgent GetShareAgent].VLCplayer fastForwardAtRate:2.0];
        double delayInSeconds = 3.0;
        dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, delayInSeconds * NSEC_PER_SEC);
        dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
            [[ProtocolAgent GetShareAgent].VLCplayer fastForwardAtRate:1.0];
            s_IsSyncing = false;
        });
        
        s_IsSyncing = true;
    }*/
    
    
  /*  static bool s_IsSynced = false;
     
     if(!s_IsSynced)
     {
         [[ProtocolAgent GetShareAgent].VLCplayer fastForwardAtRate:2.0];
         double delayInSeconds = 2.0;
         dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, delayInSeconds * NSEC_PER_SEC);
         dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
             [[ProtocolAgent GetShareAgent].VLCplayer fastForwardAtRate:1.0];
         });
     
         s_IsSynced = true;
     }
   */
}
//------------------------------------------------------------------------------------------------
-(int) ConnectToDevice:(LPCTSTR) pszIPAddress
              WithPort:(int)i32PortNum
{
    _deviceMode = -1;
    [ProtocolAgent GetShareAgent].getDataStatus = E_Data_Status_initialization;
    
    [self ResetStreamingURL];
    [[LogDataHelper GetShareHelper]LogMessage:[NSString stringWithFormat:@"Connect to %s",pszIPAddress]];
    
    int i32Status = GPCam_ConnectToDevice(pszIPAddress,i32PortNum);
    [self CreateProgressView:E_Protocol_Status_Connect withLabel:NSLocalizedString(@"Connecting", @"")];
    
    return i32Status;
}
//------------------------------------------------------------------------------------------------
-(void) Disconnect
{
    _HubFinish = true;
    _deviceMode = -1;
    return GPCam_Disconnect();
}
//------------------------------------------------------------------------------------------------
-(void) SetDownloadPath:(const char*) ptszPath
{
    GPCam_SetDownloadPath(ptszPath);
}
//------------------------------------------------------------------------------------------------
-(void) SetFileMappingInfo:(const char*) ptsMappingInfo
{
    GPCam_SetFileNameMapping(ptsMappingInfo);
}
//------------------------------------------------------------------------------------------------
-(void) ClearCmdQueue
{
    GPCam_ClearCmdQueue();
}
//------------------------------------------------------------------------------------------------
-(void) Abort:(int) i32Index
{
    printf("Abort! Cmd: %d\n",i32Index);
    GPCam_Abort(i32Index);
    _ignoreIndex = GPCam_SendGetStatus(); // send any command to abort download, and ignore next NAK
}
//------------------------------------------------------------------------------------------------
-(void) CheckActive
{
    if(_DeviceStatus[1] & 0x01) // Mode is active
    {
        if(_DeviceStatus[0] == E_DeviceMode_Record)
            [[ProtocolAgent GetShareAgent] SendRecordCmd]; //Stop record first
        else if(_DeviceStatus[0] == E_DeviceMode_Capture)
            [[ProtocolAgent GetShareAgent] SendCapturePicture]; //Stop apture Picture first
        
        _DeviceStatus[1] &= ~0x01; ///clear active
    }
}
//------------------------------------------------------------------------------------------------
//General
-(int) SendSetMode:(int) i32Mode
{
    printf("SendSetMode: %d\n",i32Mode);
    
    if(_deviceMode == i32Mode)
    {
        printf("Same mode! Discard command!\n");
        return -1; // not allow switch to same mode
    }
    
    [self CheckActive];
    
    if([ProtocolAgent GetShareAgent].FFplayer.playing)
    {
      [[ProtocolAgent GetShareAgent].FFplayer Stop];
      GPCam_InsertCommandDelay(100); //insert 100 ms delay
    }
    
    _deviceMode = i32Mode;
    
    [self CreateProgressView:E_Protocol_Status_Command withLabel:[NSString stringWithFormat:@"%@ %d",NSLocalizedString(@"Send SetMode", @""),i32Mode]];
    return GPCam_SendSetMode(i32Mode);
}
//------------------------------------------------------------------------------------------------
-(int) SendSetModeDirectly:(int) i32Mode
{
    printf("SendSetModeDirectly: %d\n",i32Mode);
    [self CheckActive];

    [self CreateProgressView:E_Protocol_Status_Command withLabel:[NSString stringWithFormat:@"%@ %d",NSLocalizedString(@"Send SetMode", @""),i32Mode]];
    return GPCam_SendSetMode(i32Mode);
}
//------------------------------------------------------------------------------------------------
-(int) SendGetStatus
{
    return GPCam_SendGetStatus();
}
//------------------------------------------------------------------------------------------------
-(int) SendGetSetPIP:(int) i32Type
{
    return GPCam_SendGetSetPIP(i32Type);
}
//------------------------------------------------------------------------------------------------
-(int) SendGetParameterFile:(const char*) ptszFilaName
{
    [self CreateProgressView:E_Protocol_Status_Command withLabel:NSLocalizedString(@"Getting prameter file", @"")];
    return GPCam_SendGetParameterFile(ptszFilaName);
}
//------------------------------------------------------------------------------------------------
-(int) SendPowerOff
{
    [self CreateProgressView:E_Protocol_Status_Command withLabel:NSLocalizedString(@"Send PowerOff", @"")];
    return GPCam_SendPowerOff();
}
//------------------------------------------------------------------------------------------------
-(int) SendRestartStreaming
{
    double delayInSeconds = 0.3;

    dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, delayInSeconds * NSEC_PER_SEC);
    dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
        [[ProtocolAgent GetShareAgent].FFplayer Play];
    });
 
    [self CreateProgressView:E_Protocol_Status_Command withLabel:NSLocalizedString(@"Send RestartStreaming", @"")];
    return GPCam_SendRestartStreaming();
}
//------------------------------------------------------------------------------------------------
-(void) SendCheckFileMapping
{
    GPCam_CheckFileMapping();
}
//------------------------------------------------------------------------------------------------
//Record
-(int) SendRecordCmd
{
    [self CreateProgressView:E_Protocol_Status_Command withLabel:NSLocalizedString(@"Send record", @"")];
    return GPCam_SendRecordCmd();
}
//------------------------------------------------------------------------------------------------
-(int) SendAudioOnOff:(bool) bOn
{
    [self CreateProgressView:E_Protocol_Status_Command withLabel:NSLocalizedString(@"Send AudioOnOff", @"")];
    return GPCam_SendAudioOnOff(bOn);
}
//------------------------------------------------------------------------------------------------
//Capture picture
-(int) SendCapturePicture
{
    [self CreateProgressView:E_Protocol_Status_Command withLabel:NSLocalizedString(@"Send CapturePicture", @"")];
    return GPCam_SendCapturePicture();
}
//------------------------------------------------------------------------------------------------
//Playback
-(int) SendStartPlayback:(int) i32Index
{
    [self CreateProgressView:E_Protocol_Status_Command withLabel:NSLocalizedString(@"Send StartPlayback", @"")];
    return GPCam_SendStartPlayback(i32Index);
}
//------------------------------------------------------------------------------------------------
-(int) SendPausePlayback
{
    [self CreateProgressView:E_Protocol_Status_Command withLabel:NSLocalizedString(@"Send PausePlayback", @"")];
    return GPCam_SendPausePlayback();
}
//------------------------------------------------------------------------------------------------
-(int) SendGetFullFileList
{
    [self CreateProgressView:E_Protocol_Status_Command withLabel:NSLocalizedString(@"Send GetFullFileList", @"")];
    return GPCam_SendGetFullFileList();
}
//------------------------------------------------------------------------------------------------
-(int) SendGetFileThumbnail:(int)i32Index
{
    [[LogDataHelper GetShareHelper]LogMessage:NSLocalizedString(@"Send GetFileThumbnail", @"")];
    return GPCam_SendGetFileThumbnail(i32Index);
}
//------------------------------------------------------------------------------------------------
-(int) SendGetFileRawData:(int)i32Index
{
    [self CreateProgressView:E_Protocol_Status_Progress withLabel:NSLocalizedString(@"Send GetFileRawData", @"")];
    _i32CurCMDIndex = GPCam_SendGetFileRawdata(i32Index);
    printf("Cmd: %d\n",_i32CurCMDIndex);

    return _i32CurCMDIndex;
}
//------------------------------------------------------------------------------------------------
-(int) SendStopPlayback
{
    [self CreateProgressView:E_Protocol_Status_Command withLabel:NSLocalizedString(@"Send SendStopPlayback", @"")];
    return GPCam_SendStopPlayback();
}
//------------------------------------------------------------------------------------------------
-(int) SetNextPlaybackFileListIndex:(int) i32Index
{
    return GPCam_SetNextPlaybackFileListIndex(i32Index);
}
//------------------------------------------------------------------------------------------------
-(int) SendDeleteFile:(int)i32Index
{
    [self CreateProgressView:E_Protocol_Status_Command withLabel:NSLocalizedString(@"Send SendDeleteFile", @"")];
    return GPCam_SendDeleteFile(i32Index);
}
//------------------------------------------------------------------------------------------------
//Menu
-(int) SendGetParameter:(int) i32ID
{
    return GPCam_SendGetParameter(i32ID);
}
//------------------------------------------------------------------------------------------------
-(int) SendSetParameter:(int) i32ID
               withSize:(int) i32Size
               withData:(BYTE*) pbyData
{
    [self CreateProgressView:E_Protocol_Status_Command withLabel:NSLocalizedString(@"Send SetParameter", @"")];
    return GPCam_SendSetParameter(i32ID,i32Size,pbyData);
}
//------------------------------------------------------------------------------------------------
-(int) SendFirmwareDownload:(unsigned int)ui32FileSize
               withCheckSum:(unsigned int)ui32Checksum
{
    [self CreateProgressView:E_Protocol_Status_Command withLabel:NSLocalizedString(@"Send FirmwareDownload", @"")];
    return GPCam_SendFirmwareDownload(ui32FileSize,ui32Checksum);
}
//------------------------------------------------------------------------------------------------
-(int) SendFirmwareRawData:(unsigned int)ui32Data
                  withData:(BYTE*)pbyData
{
    [self CreateProgressView:E_Protocol_Status_Progress withLabel:NSLocalizedString(@"Send FirmwareRawData", @"")];
    return GPCam_SendFirmwareRawData(ui32Data,pbyData);
}
//------------------------------------------------------------------------------------------------
-(int) SendFirmwareUpgrade
{
    [self CreateProgressView:E_Protocol_Status_Command withLabel:NSLocalizedString(@"Send FirmwareUpgrade", @"")];
    return GPCam_SendFirmwareUpgrade();
}
//------------------------------------------------------------------------------------------------
//Vendor
-(int) SendVendorCmd:(BYTE*)pbyData
            withSize:(int)i32Size
{
    [self CreateProgressView:E_Protocol_Status_Command withLabel:NSLocalizedString(@"Send VendorCmd", @"")];
    return GPCam_SendVendorCmd(pbyData, i32Size);
}
//------------------------------------------------------------------------------------------------
//Muti command
-(int) SendGetDeviceSettings
{
    int i32Ret = 0;
    _i32MutiCmdCnt = 0;
    _MutiCmdNofify = Notification_Device_SettingChange;
    for (CategoriesItem *CategoriesI in _FileParser.Categories)
    {
        for (SettingsItem *SettingsI in CategoriesI.Settings)
        {
            i32Ret = [self SendGetParameter:SettingsI.ID];
            _i32MutiCmdCnt++;
        }
    }
    
    [self CreateProgressView:E_Protocol_Status_MutiCommand withLabel:NSLocalizedString(@"Getting setting from device", @"")];
    
    
    return i32Ret;
}

@end
