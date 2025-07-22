//
//  ProtocolAgent.h
//  GPCam
//
//  Created by generalplus_sa1 on 8/20/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//
#import <UIKit/UIKit.h>
#import <Foundation/Foundation.h>
#import "FFmpegPlayer.h"
#import "SettingFileParser.h"
#import "GPCamTypedef.h"

#define DEFAULT_PAREMETER_FILE_NAME @"Default_Menu"
#define PAREMETER_FILE_NAME         "Menu.xml"
#define DEVICE_STATUS_SIZE          14

#define STREAMING_URL               @"http://%@:8080/?action=stream"
#define RTSP_STREAMING_URL          @"rtsp://%@:8080/?action=stream"
#define COMMAND_IP                  "192.168.25.1"
#define COMMAND_PORT                8081
#define NSUserDefaults_SetTime      @"SetTime"
#define NSUserDefaults_SetBufferingTime      @"SetBufferingTime"
#define NSUserDefaults_SetDiscardcorrupt      @"SetDiscardcorrupt"

//--------------------------------------------------------------------------------
@protocol PlaybackStatusDelegate

-(void) GetFileCount:(int)i32Count;
-(void) GetFileName:(int)i32Index          //Index in command agent
           withSize:(int)i32Size;
-(void) FileThumbnailIsReady:(int)i32Index //Index in command agent
           withThumbnailName:(NSString*)ThumbnailName;
-(void) FileRawData:(int)i32Index          //Index in command agent
       withFileName:(NSString*)FileName;
-(void) FileDeleted:(bool)bsucceed;
@end
//--------------------------------------------------------------------------------
@protocol VendorCommandDelegate

-(void) GetData:(BYTE*)pbyData
       withSize:(int)i32Size;
@end
//--------------------------------------------------------------------------------
@protocol FirmwareDownloadDelegate

-(void) DownloadComplete;
-(void) RawDataComplete;
-(void) UpgradeComplete;

@end
//--------------------------------------------------------------------------------
@protocol GetParameterDelegate

-(void) ID:(int) i32ID
     Value:(int) i32Value;

@end
//--------------------------------------------------------------------------------
extern NSString *Notification_Device_SetModeComplete;
extern NSString *Notification_Device_ConnectionChange;   //Object = (NSSting) @"Connected" , @"DisConnected"
extern NSString *Notification_Device_SettingChange;
extern NSString *Notification_Device_CommandFailed;      //Object = NSArray[(NSNumber),(NSNumber),nil]
extern NSString *Notification_Device_PlaybackStopComplete;
extern NSString *Notification_Device_GetSettingComplete;
extern NSString *Notification_Device_ShowPIPButton;
extern NSString *ErrorCodeString[];
//--------------------------------------------------------------------------------
@interface ProtocolAgent : NSObject

+(ProtocolAgent*)       GetShareAgent;
-(SettingFileParser*)   GetSettingFileParser;


-(int) ConnectToDevice:(LPCTSTR) pszIPAddress
              WithPort:(int)i32PortNum;

-(void) Disconnect;
-(void) SetDownloadPath:(const char*) ptszPath;
-(void) ClearCmdQueue;
-(void) Abort:(int) i32Index;
-(void) SetFileMappingInfo:(const char*) ptsMappingInfo;

//General
-(int) SendSetMode:(int) i32Mode;
-(int) SendGetSetPIP:(int) i32Type;
-(int) SendSetModeDirectly:(int) i32Mode;
-(int) SendGetStatus;
-(int) SendGetParameterFile:(const char*) ptszFilaName;
-(int) SendPowerOff;
-(int) SendRestartStreaming;  // Automatically start VLC player when it's done.
-(void) SendCheckFileMapping;

//Record
-(int) SendRecordCmd;
-(int) SendAudioOnOff:(bool) bOn;

//Capture picture
-(int) SendCapturePicture;

//Playback
-(int) SendStartPlayback:(int) i32Index;
-(int) SendPausePlayback;
-(int) SendGetFullFileList;
-(int) SendGetFileThumbnail:(int)i32Index;
-(int) SendGetFileRawData:(int)i32Index;
-(int) SendStopPlayback;
-(int) SetNextPlaybackFileListIndex:(int)i32Index;
-(int) SendDeleteFile:(int)i32Index;

//Menu
-(int) SendGetParameter:(int) i32ID;
-(int) SendSetParameter:(int) i32ID
               withSize:(int) i32Size
               withData:(BYTE*) pbyData;

//Firmware
-(int) SendFirmwareDownload:(unsigned int)ui32FileSize
               withCheckSum:(unsigned int)ui32Checksum;
-(int) SendFirmwareRawData:(unsigned int)ui32Data
                  withData:(BYTE*)pbyData;
-(int) SendFirmwareUpgrade;

//Vendor
-(int) SendVendorCmd:(BYTE*)pbyData
            withSize:(int)i32Size;

//Muti command
-(int) SendGetDeviceSettings;

//Misc
-(BYTE*) GetDeviceStatus;

-(E_ConnectionStatus) GetConnectionStatus;

-(NSString*) GetFileName:(int) i32Index;
-(int)       GetFileIndex:(int) i32Index; //File index in device
-(bool)      GetFileTime:(int) i32Index
          withTimeBuffer:(BYTE*)pbyTime;
-(unsigned int)       GetFileSize:(int) i32Index;
-(BYTE*)GetFileExtraInfo:(int) i32Index
        withSizeReadBack:(int*)pi32Size;

-(void)      ResetStreamingURL;
-(void)      SyncFrame;

//Internal use
-(void) NotifyCommandAck:(bool) bIsAck
           WithErrorCode:(short)ErrorCode
            WithCmdIndex:(int)i32Index;

-(void) SetDeviceStatus:(BYTE*)pbyStatus;
@property (nonatomic) float progress;  //UI Hub progress
@property (nonatomic) int ignoreIndex; //For abort command
@property (nonatomic) int getDataStatus;

//-------------
@property (nonatomic, strong) UIView            *ActiveView;   //Auto create progress view and insert to activeview
@property (nonatomic, strong) FFmpegPlayer      *FFplayer;
@property (nonatomic, strong) UIViewController  *AlertViewController;

@property (weak) id<PlaybackStatusDelegate>   PlaybackDelegate;
@property (weak) id<VendorCommandDelegate>    VendorDelegate;
@property (weak) id<FirmwareDownloadDelegate> FirmwareDownloadDelegate;
@property (weak) id<GetParameterDelegate>     GetParameterDelegate;

@property (nonatomic) int PlayerStatus;

// -1 = unknow, 0 = http, 1 = rtsp
@property (nonatomic) int m_iRtsp;
@property (nonatomic) bool m_bCanDeleteSDFile;
@property (nonatomic, strong) NSString  *m_strCommandIP;
@end
