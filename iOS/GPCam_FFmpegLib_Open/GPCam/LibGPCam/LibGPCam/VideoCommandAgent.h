//
//  CommandAgent.h
//  GPCam
//
//  Created by generalplus_sa1 on 8/11/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#ifndef __GPCam__VideoCommandAgent__
#define __GPCam__VideoCommandAgent__

#include <vector>
#include "VideoSteamCommmad.h"
#include "CommandAgent.h"
#include "CommandAgentDef.h"
#include "VideoCommandAgentDef.h"
#include "VideoPacketParser.h"

typedef struct tag_FileNameMappingInfo
{
    char        szExt;
    char        szPrefixName[16];
    char        szType[16];
    
}S_FileNameMappingInfo;

typedef std::vector<S_FileNameMappingInfo*> FileNameMappingVector;
typedef std::vector<S_FileNameMappingInfo*>::iterator FileNameMapping_iterator;

typedef std::vector<S_FileAttribute*> FileAttributeVector;
typedef std::vector<S_FileAttribute*>::iterator FileAttributeVector_iterator;
//---------------------------------------------------

class C_VideoCommandAgent : public C_CommandAgent ,
                            public I_ViedoFileAgent
{
public:
    C_VideoCommandAgent();
    virtual ~C_VideoCommandAgent();
    
    static C_VideoCommandAgent* GetShareAgent();
    
    void SetDownloadPath(const char* ptszPath);
    
    //General
    int SendSetMode(int i32Mode);
    int SendGetStatus();
    int SendGetParameterFile(const char* ptszFilaName);
    int SendPowerOff();
    int SendRestartStreaming();
    int SendCheckFileMapping();
    int SendGetSetPIP(int i32Type);
    //Record
    int SendRecordCmd();
    int SendAudioOnOff(bool bOn);
    
    //Capture picture
    int SendCapturePicture();
    
    //Playback
    int SendStartPlayback(int i32Index);
    int SendPausePlayback();
    int SendGetFullFileList();
    int SendGetFileByIndex(int i32Index);
    int SendGetFileThumbnail(int i32Index);
    int SendGetFileRawdata(int i32Index);
    int SendStopPlayback();
    int SetNextPreviewFileList(int i32Index);
    int SendDeleteFile(int i32Index);
    int DeleteFile(int i32Index);
    
    //Menu
    int SendGetParameter(int i32ID);
    int SendSetParameter(int i32ID, int i32Size, BYTE* pbyData);
    
    //Firmware
    int SendFirmwareDownload(unsigned int ui32FileSize, unsigned int ui32CheckSum);
    int SendFirmwareRawData(unsigned int ui32Size, BYTE* pbyData);
    int SendFirmwareUpgrade();
    
    //CV Firmware
    int SendCVFirmwareDownload(unsigned int ui32FileSize, unsigned int ui32CheckSum);
    int SendCVFirmwareRawData(unsigned int ui32Size, BYTE* pbyData);
    int SendCVFirmwareUpgrade(unsigned int ui32Area);

    //Vendor
    int SendVendorCmd(BYTE* pbydata ,int i32Size);
    
    //Action
    int InsertCommandDelay(int i32ms);
    
    //File
    virtual char* GetFileName(int i32Index);
    bool GetFileTime(int i32Index,BYTE *pTime); //Return 6 byte: year(from 2000) , mouth , day , hour , munite , second
    bool SetFileNameMapping(const char* ptszFileNameMapping);
    
    //I_ViedoFileAgent interface
    virtual int  GetFileAttrListSize();
    virtual int  SetFileAttr(int i32Index,BYTE* pbyFileAttr,int i32ExtraSize);
    virtual int  AddOneFileAttr(int i32Index,BYTE* pbyFileAttr,int i32ExtraSize);
    virtual int  AllocFileList(int i32Count);
    virtual bool RemoveFileAttr(int i32Index);
    virtual int  GetFileIndex(int i32Index);
    virtual unsigned int  GetFileSize(int i32Index); //KB
    virtual BYTE GetFileExt(int i32Index);
    virtual BYTE *GetFileExtraInfo(int i32Index,int *pi32Size);
    virtual bool GetNextPreview(int *pi32Index);
    
    // C_CommandAgent virtual
    virtual bool CommandBeforeConnect();
    virtual bool CommandAfterConnected();
    virtual I_PacketParser* GetPacketParser();
    
private:
    
    bool IsFileIndexReady(int i32Index);
    
    void                    ClearNameList();
    void                    ClearFileList();
    
    char                    m_szDownloadFileFolder[256];
    
    BYTE*                   m_pbyRealVendorPayload;
    int                     m_pbyRealVendorPayloadSize;
    
    int                     m_i32CurrentPlayIndex;
    FileAttributeVector     m_FileAttributeList;
    FileNameMappingVector   m_FileNameMappingList;
    C_VideoPacketParser     m_PacketParser;
    
    int                     m_i32NextPreviewIndex;

};





#endif /* defined(__GPCam__CommandAgent__) */
