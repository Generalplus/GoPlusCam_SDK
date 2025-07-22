//
//  GPCamCommandAPI.cpp
//  GPCam
//
//  Created by generalplus_sa1 on 8/20/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#include "GPCamCommandAPI.h"
#include "VideoCommandAgent.h"
#include "VideoSteamCommmad.h"

static char str_version[32] = "LibGPCam_V1.0.15";

//----------------------------------------------------
char* GPCam_Version()
{
    return str_version;
}
//----------------------------------------------------
int GPCam_ConnectToDevice(
                          LPCTSTR pszIPAddress,
                          int i32PortNum)
{
    return C_VideoCommandAgent::GetShareAgent()->ConnectToDevice(pszIPAddress, i32PortNum);
}
//----------------------------------------------------
void GPCam_Disconnect()
{
    C_VideoCommandAgent::GetShareAgent()->Disconnect();
}
//----------------------------------------------------
void GPCam_SetCmdStatusCallBack(PFN_CmdStatusCallBack CallBack)
{
    C_VideoCommandAgent::GetShareAgent()->SetCmdStatusCallBack(CallBack);
}
//----------------------------------------------------
void GPCam_SetDownloadPath(const char* ptszPath)
{
    C_VideoCommandAgent::GetShareAgent()->SetDownloadPath(ptszPath);
}
//----------------------------------------------------
void GPCam_ClearCmdQueue()
{
    C_VideoCommandAgent::GetShareAgent()->ClearCmdQueue();
}
//----------------------------------------------------
int GPCam_Abort(int i32Index)
{
    return C_VideoCommandAgent::GetShareAgent()->Abort(i32Index);
}
//----------------------------------------------------
//General
int GPCam_SendSetMode(int i32Mode)
{
    return C_VideoCommandAgent::GetShareAgent()->SendSetMode(i32Mode);
}
//----------------------------------------------------
int GPCam_SendGetSetPIP(int i32Type)
{
    return C_VideoCommandAgent::GetShareAgent()->SendGetSetPIP(i32Type);
}
//----------------------------------------------------
int GPCam_SendGetStatus()
{
    return C_VideoCommandAgent::GetShareAgent()->SendGetStatus();
}
//----------------------------------------------------
int GPCam_SendGetParameterFile(const char* ptszFilaName)
{
    return C_VideoCommandAgent::GetShareAgent()->SendGetParameterFile(ptszFilaName);
}
//----------------------------------------------------
int GPCam_SendPowerOff()
{
    return C_VideoCommandAgent::GetShareAgent()->SendPowerOff();
}
//----------------------------------------------------
int GPCam_SendRestartStreaming()
{
    return C_VideoCommandAgent::GetShareAgent()->SendRestartStreaming();
}
//----------------------------------------------------
//Record
int GPCam_SendRecordCmd()
{
    return C_VideoCommandAgent::GetShareAgent()->SendRecordCmd();
}
//----------------------------------------------------
int GPCam_SendAudioOnOff(bool bOn)
{
    return C_VideoCommandAgent::GetShareAgent()->SendAudioOnOff(bOn);
}
//----------------------------------------------------
//Capture picture
int GPCam_SendCapturePicture()
{
    return C_VideoCommandAgent::GetShareAgent()->SendCapturePicture();
}
//----------------------------------------------------
//Playback
int GPCam_SendStartPlayback(int i32Index)
{
    return C_VideoCommandAgent::GetShareAgent()->SendStartPlayback(i32Index);
}
//----------------------------------------------------
int GPCam_SendPausePlayback()
{
    return C_VideoCommandAgent::GetShareAgent()->SendPausePlayback();
}
//----------------------------------------------------
int GPCam_SendGetFullFileList()
{
    return C_VideoCommandAgent::GetShareAgent()->SendGetFullFileList();
}
//----------------------------------------------------
int GPCam_SendGetFileThumbnail(int i32Index)
{
    return C_VideoCommandAgent::GetShareAgent()->SendGetFileThumbnail(i32Index);
}
//----------------------------------------------------
int GPCam_SendGetFileRawdata(int i32Index)
{
    return C_VideoCommandAgent::GetShareAgent()->SendGetFileRawdata(i32Index);
}
//----------------------------------------------------
int GPCam_SendStopPlayback()
{
    return C_VideoCommandAgent::GetShareAgent()->SendStopPlayback();
}
//----------------------------------------------------
int GPCam_SetNextPlaybackFileListIndex(int i32Index)
{
    return C_VideoCommandAgent::GetShareAgent()->SetNextPreviewFileList(i32Index);
}
//----------------------------------------------------
int GPCam_SendDeleteFile(int i32Index)
{
    return C_VideoCommandAgent::GetShareAgent()->SendDeleteFile(i32Index);
}
//----------------------------------------------------
//Menu
int GPCam_SendGetParameter(int i32ID)
{
    return C_VideoCommandAgent::GetShareAgent()->SendGetParameter(i32ID);
}
//----------------------------------------------------
int GPCam_SendSetParameter(int i32ID, int i32Size, BYTE* pbyData)
{
    return C_VideoCommandAgent::GetShareAgent()->SendSetParameter(i32ID, i32Size, pbyData);
}
//----------------------------------------------------
//Firmware
int GPCam_SendFirmwareDownload(unsigned int ui32FileSize, unsigned int ui32CheckSum)
{
    return C_VideoCommandAgent::GetShareAgent()->SendFirmwareDownload(ui32FileSize, ui32CheckSum);
}
//----------------------------------------------------
int GPCam_SendFirmwareRawData(unsigned int ui32Size, BYTE* pbyData)
{
    return C_VideoCommandAgent::GetShareAgent()->SendFirmwareRawData(ui32Size, pbyData);
}
//----------------------------------------------------
int GPCam_SendFirmwareUpgrade()
{
    return C_VideoCommandAgent::GetShareAgent()->SendFirmwareUpgrade();
}
//----------------------------------------------------
//CV Firmware
int GPCam_SendCVFirmwareDownload(unsigned int ui32FileSize, unsigned int ui32CheckSum)
{
    return C_VideoCommandAgent::GetShareAgent()->SendCVFirmwareDownload(ui32FileSize, ui32CheckSum);
}
//----------------------------------------------------
int GPCam_SendCVFirmwareRawData(unsigned int ui32Size, BYTE* pbyData)
{
    return C_VideoCommandAgent::GetShareAgent()->SendCVFirmwareRawData(ui32Size, pbyData);
}
//----------------------------------------------------
int GPCam_SendCVFirmwareUpgrade(unsigned int ui32Area)
{
    return C_VideoCommandAgent::GetShareAgent()->SendCVFirmwareUpgrade(ui32Area);
}
//----------------------------------------------------
//Vendor
int GPCam_SendVendorCmd(BYTE* pbydata ,int i32Size)
{
    return C_VideoCommandAgent::GetShareAgent()->SendVendorCmd(pbydata, i32Size);
}
//----------------------------------------------------
//Action
int GPCam_InsertCommandDelay(int i32ms)
{
    return C_VideoCommandAgent::GetShareAgent()->InsertCommandDelay(i32ms);
}
//----------------------------------------------------
E_ConnectionStatus  GPCam_GetStatus()
{
    return C_VideoCommandAgent::GetShareAgent()->GetStatus();
}
//----------------------------------------------------
//File
char* GPCam_GetFileName(int i32Index)
{
    return C_VideoCommandAgent::GetShareAgent()->GetFileName(i32Index);
}
//----------------------------------------------------
bool  GPCam_GetFileTime(int i32Index,BYTE *pTime) //Return 6 byte: year(from 2000) , mouth , day , hour , munite , second
{
    return C_VideoCommandAgent::GetShareAgent()->GetFileTime(i32Index, pTime);
}
//----------------------------------------------------
int   GPCam_GetFileIndex(int i32Index)
{
    return C_VideoCommandAgent::GetShareAgent()->GetFileIndex(i32Index);
}
//----------------------------------------------------
unsigned int   GPCam_GetFileSize(int i32Index)
{
    return C_VideoCommandAgent::GetShareAgent()->GetFileSize(i32Index);
}
//----------------------------------------------------
BYTE*  GPCam_GetFileExtraInfo(int i32Index,int *pi32Size)
{
    return C_VideoCommandAgent::GetShareAgent()->GetFileExtraInfo(i32Index, pi32Size);
}
//----------------------------------------------------
bool  GPCam_SetFileNameMapping(const char *pMappingString)
{
    return C_VideoCommandAgent::GetShareAgent()->SetFileNameMapping(pMappingString);
}
//----------------------------------------------------
BYTE  GPCam_GetFileExt(int i32Index)
{
    return C_VideoCommandAgent::GetShareAgent()->GetFileExt(i32Index);
}
//----------------------------------------------------
void  GPCam_SetDataCallBack(PFN_SocketDataCallBack CallBack)
{
    C_VideoCommandAgent::GetShareAgent()->SetDataCallBack(CallBack);
}
//----------------------------------------------------
void  GPCam_CheckFileMapping()
{
    C_VideoCommandAgent::GetShareAgent()->SendCheckFileMapping();
}
//----------------------------------------------------
void GPCam_GetFileByIndex(int i32Index)
{
	C_VideoCommandAgent::GetShareAgent()->SendGetFileByIndex(i32Index);
}
//----------------------------------------------------
int GPCam_DeleteFile(int i32Index)
{
    return C_VideoCommandAgent::GetShareAgent()->DeleteFile(i32Index);
}
