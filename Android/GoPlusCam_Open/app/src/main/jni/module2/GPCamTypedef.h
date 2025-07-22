//
//  Typedef.h
//  GPCam
//
//  Created by generalplus_sa1 on 8/11/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#ifndef GPCam_Typedef_h
#define GPCam_Typedef_h

//--------------------------------------------------------------
//Packet format
//--------------------------------------------------------------
//GP_SOCK_TYPE 2Byte
#include "GPCommondef.h"

//GP_SOCK_MODE_ID 1Byte
#define GPSOCK_MODE_General                     0x00
#define GPSOCK_MODE_Record                      0x01
#define GPSOCK_MODE_CapturePicture              0x02
#define GPSOCK_MODE_Playback                    0x03
#define GPSOCK_MODE_Menu                        0x04
#define GPSOCK_MODE_Firmware                    0x05
#define GPSOCK_MODE_Firmware_CV                 0x06
#define GPSOCK_MODE_Vendor                      0xFF

//GP_SOCK_CMD_ID 1Byte
#define GPSOCK_General_CMD_SetMode              0x00
#define GPSOCK_General_CMD_GetDeviceStatus      0x01
#define GPSOCK_General_CMD_GetParameterFile     0x02
#define GPSOCK_General_CMD_Poweroff             0x03
#define GPSOCK_General_CMD_RestartStreaming     0x04
#define GPSOCK_General_CMD_AuthDevice           0x05
#define GPSOCK_General_CMD_CheckMapping         0x07
#define GPSOCK_General_CMD_GetSetPIP            0x08

#define GPSOCK_Record_CMD_Start                 0x00
#define GPSOCK_Record_CMD_Audio                 0x01

#define GPSOCK_CapturePicture_CMD_Capture       0x00

#define GPSOCK_Playback_CMD_Start               0x00
#define GPSOCK_Playback_CMD_Pause               0x01
#define GPSOCK_Playback_CMD_GetFileCount        0x02
#define GPSOCK_Playback_CMD_GetNameList         0x03
#define GPSOCK_Playback_CMD_GetThumbnail        0x04
#define GPSOCK_Playback_CMD_GetRawData          0x05
#define GPSOCK_Playback_CMD_Stop                0x06
#define GPSOCK_Playback_CMD_GetSpecificName     0x07
#define GPSOCK_Playback_CMD_DeleteFile          0x08

#define GPSOCK_Menu_CMD_GetParameter            0x00
#define GPSOCK_Menu_CMD_SetParameter            0x01

#define GPSOCK_Firmware_CMD_Download            0x00
#define GPSOCK_Firmware_CMD_SendRawData         0x01
#define GPSOCK_Firmware_CMD_Upgrade             0x02

#define GPSOCK_Vendor_CMD_Vendor                0x00

//--------------------------------------------------------------
//Define
//--------------------------------------------------------------
typedef enum
{
    E_DeviceMode_Record = 0,
    E_DeviceMode_Capture ,
    E_DeviceMode_Playback,
    E_DeviceMode_Menu,
    E_DeviceMode_USB
    
}E_DeviceMode;

#define DEFAULT_MAPPING_STR "A=MOVI,avi;J=PICT,jpg;L=LOCK,avi;S=SOS0,avi"

#define GP22_DEFAULT_MAPPING_STR "A=MOVI,mov;J=PICT,jpg;L=LOCK,mov;S=SOS0,mov"

#endif
