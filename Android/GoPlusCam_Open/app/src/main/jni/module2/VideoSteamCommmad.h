//
//  VideoSteamCommnad.h
//  GPCam
//
//  Created by generalplus_sa1 on 8/12/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#ifndef __GPCam__VideoSteamCommnad__
#define __GPCam__VideoSteamCommnad__

#include <stdio.h>
#include "VideoStreamCommandBase.h"

#define FW_UPGRADE_TIMEOUT    60

/***************************************************************************************
 General mode
 ***************************************************************************************/

class C_SetModeCmd : public C_VideoSteamCommmadBase
{
public:
    
    C_SetModeCmd(I_CommandAgent* pCommandAgnet,I_ViedoFileAgent *pFileAgent,PFN_CmdStatusCallBack StatusCallBack,int i32Mode);
    virtual ~C_SetModeCmd();
    
private:
    
};

class C_GetSetPIPCmd : public C_VideoSteamCommmadBase
{
public:
    
    C_GetSetPIPCmd(I_CommandAgent* pCommandAgnet,I_ViedoFileAgent *pFileAgent,PFN_CmdStatusCallBack StatusCallBack,int i32Type);
    virtual ~C_GetSetPIPCmd();
    
private:
    
};

class C_GetDevuceStatusCmd : public C_VideoSteamCommmadBase
{
public:
    
    C_GetDevuceStatusCmd(I_CommandAgent* pCommandAgnet,I_ViedoFileAgent *pFileAgent,PFN_CmdStatusCallBack StatusCallBack);
    virtual ~C_GetDevuceStatusCmd();
    
    virtual E_HandleAck_Retcode HandleAck(I_PacketParser *packet);
private:
    
    
};

class C_GetParameterFileCmd : public C_VideoSteamCommmadBase
{
public:
    
    C_GetParameterFileCmd(I_CommandAgent* pCommandAgnet,I_ViedoFileAgent *pFileAgent,PFN_CmdStatusCallBack StatusCallBack,const char* pathToSave);
    virtual ~C_GetParameterFileCmd();
    
    virtual E_HandleAck_Retcode HandleAck(I_PacketParser *packet);
private:
    
    char        m_szFilePath[256];
    FILE        *m_fp;
    int         m_i32writeSize;
    
};

class C_SendPowerOffCmd : public C_VideoSteamCommmadBase
{
public:
    
    C_SendPowerOffCmd(I_CommandAgent* pCommandAgnet,I_ViedoFileAgent *pFileAgent,PFN_CmdStatusCallBack StatusCallBack);
    virtual ~C_SendPowerOffCmd();
    
    
};

class C_SendRestartStreamingCmd : public C_VideoSteamCommmadBase
{
public:
    
    C_SendRestartStreamingCmd(I_CommandAgent* pCommandAgnet,I_ViedoFileAgent *pFileAgent,PFN_CmdStatusCallBack StatusCallBack);
    virtual ~C_SendRestartStreamingCmd();
    
    
};

class C_AuthDeviceCmd : public C_VideoSteamCommmadBase
{
public:
    
    C_AuthDeviceCmd(I_CommandAgent* pCommandAgnet,I_ViedoFileAgent *pFileAgent,PFN_CmdStatusCallBack StatusCallBack);
    virtual ~C_AuthDeviceCmd();
    
    virtual E_HandleAck_Retcode HandleAck(I_PacketParser *packet);
    
private:
    
    void   Gen_LFSR_32Bit_Key(BYTE *pbySeed,BYTE *pbyKey,BYTE *pbyPayload);
    BYTE   m_LFSRseed[4];
};

class C_CheckFileMappingCmd : public C_VideoSteamCommmadBase
{
public:
    
    C_CheckFileMappingCmd(I_CommandAgent* pCommandAgnet,I_ViedoFileAgent *pFileAgent,PFN_CmdStatusCallBack StatusCallBack);
    virtual ~C_CheckFileMappingCmd();
    
    virtual E_HandleAck_Retcode HandleAck(I_PacketParser *packet);
private:
    void   Gen_32Bit_key(BYTE *pbyKey);
    BYTE   m_seedMapping[4];
};

/***************************************************************************************
 Record mode
 ***************************************************************************************/
class C_RecordCmd : public C_VideoSteamCommmadBase
{
public:
    
    C_RecordCmd(I_CommandAgent* pCommandAgnet,I_ViedoFileAgent *pFileAgent,PFN_CmdStatusCallBack StatusCallBack);
    virtual ~C_RecordCmd();
    
};

class C_AudioCmd : public C_VideoSteamCommmadBase
{
public:
    
    C_AudioCmd(I_CommandAgent* pCommandAgnet,I_ViedoFileAgent *pFileAgent,PFN_CmdStatusCallBack StatusCallBack, bool bOn);
    virtual ~C_AudioCmd();
    
    
};
/***************************************************************************************
 Capture picture mode
 ***************************************************************************************/
class C_CapturePictureCmd : public C_VideoSteamCommmadBase
{
public:
    
    C_CapturePictureCmd(I_CommandAgent* pCommandAgnet,I_ViedoFileAgent *pFileAgent,PFN_CmdStatusCallBack StatusCallBack);
    virtual ~C_CapturePictureCmd();
    
    
};
/***************************************************************************************
 Playback mode
 ***************************************************************************************/
class C_PlaybackStartCmd : public C_VideoSteamCommmadBase
{
public:
    
    C_PlaybackStartCmd(I_CommandAgent* pCommandAgnet,I_ViedoFileAgent *pFileAgent,PFN_CmdStatusCallBack StatusCallBack,int i32Index);
    virtual ~C_PlaybackStartCmd();
    
};

class C_PlaybackPauseCmd : public C_VideoSteamCommmadBase
{
public:
    
    C_PlaybackPauseCmd(I_CommandAgent* pCommandAgnet,I_ViedoFileAgent *pFileAgent,PFN_CmdStatusCallBack StatusCallBack,int i32Index);
    virtual ~C_PlaybackPauseCmd();
    
};

class C_GetPlaybackFileCountCmd : public C_VideoSteamCommmadBase
{
public:
    
    C_GetPlaybackFileCountCmd(I_CommandAgent* pCommandAgnet,I_ViedoFileAgent *pFileAgent,PFN_CmdStatusCallBack StatusCallBack);
    virtual ~C_GetPlaybackFileCountCmd();
    
    virtual E_HandleAck_Retcode HandleAck(I_PacketParser *packet);
    
};

class C_GetPlaybackNameListCmd : public C_VideoSteamCommmadBase
{
public:
    
    C_GetPlaybackNameListCmd(I_CommandAgent* pCommandAgnet,I_ViedoFileAgent *pFileAgent,PFN_CmdStatusCallBack StatusCallBack,int i32Index,int i32MaxCount,bool bFirst = false,int i32IndexOnDevice = -1 );
    virtual ~C_GetPlaybackNameListCmd();
    
    virtual E_HandleAck_Retcode HandleAck(I_PacketParser *packet);
    
private:
    int m_i32Index;
    int m_i32MaxCount;

};

class C_GetPlaybackThumbnailCmd : public C_VideoSteamCommmadBase
{
public:
    
    C_GetPlaybackThumbnailCmd(I_CommandAgent* pCommandAgnet,I_ViedoFileAgent *pFileAgent,PFN_CmdStatusCallBack StatusCallBack,int i32Index,const char* pathToSave);
    
    virtual ~C_GetPlaybackThumbnailCmd();
    
    virtual E_HandleAck_Retcode HandleAck(I_PacketParser *packet);
    
private:
    int         m_i32Index;
    char        m_szFilePath[256];
    FILE        *m_fp;
    
};

class C_GetPlaybackFileRawDataCmd : public C_VideoSteamCommmadBase
{
public:
    
    C_GetPlaybackFileRawDataCmd(I_CommandAgent* pCommandAgnet,I_ViedoFileAgent *pFileAgent,PFN_CmdStatusCallBack StatusCallBack,int i32Index,const char* pathToSave);
    virtual ~C_GetPlaybackFileRawDataCmd();
    
    virtual E_HandleAck_Retcode HandleAck(I_PacketParser *packet);
    
private:
    int         m_i32Index;
    char        m_szFilePath[256];
    FILE        *m_fp;
    unsigned int         m_i32FileSize;
    unsigned int         m_i32SizeGot;
    
    
};

class C_PlaybackStopCmd : public C_VideoSteamCommmadBase
{
public:
    
    C_PlaybackStopCmd(I_CommandAgent* pCommandAgnet,I_ViedoFileAgent *pFileAgent,PFN_CmdStatusCallBack StatusCallBack,int i32Index);
    virtual ~C_PlaybackStopCmd();
    
};


class C_GetPlaybackSpecificName : public C_VideoSteamCommmadBase
{
public:
    
    C_GetPlaybackSpecificName(I_CommandAgent* pCommandAgnet,I_ViedoFileAgent *pFileAgent,PFN_CmdStatusCallBack StatusCallBack,int i32Inde,int i32MaxCount);
    virtual ~C_GetPlaybackSpecificName();
    
    virtual E_HandleAck_Retcode HandleAck(I_PacketParser *packet);
    
private:
    int         m_i32QueryIndex;
    int         m_i32MaxCount;
};


class C_GetPlaybackDeleteFile : public C_VideoSteamCommmadBase
{
public:
    
    C_GetPlaybackDeleteFile(I_CommandAgent* pCommandAgnet,I_ViedoFileAgent *pFileAgent,PFN_CmdStatusCallBack StatusCallBack,int i32DeviceIndex,int i32Index);
    virtual ~C_GetPlaybackDeleteFile();
    
    virtual E_HandleAck_Retcode HandleAck(I_PacketParser *packet);
    
private:
    int         m_i32DeviceIndex;
    int         m_i32Index;
};


/***************************************************************************************
 Menu mode
 ***************************************************************************************/
class C_GetParameterCmd : public C_VideoSteamCommmadBase
{
public:
    
    C_GetParameterCmd(I_CommandAgent* pCommandAgnet,I_ViedoFileAgent *pFileAgent,PFN_CmdStatusCallBack StatusCallBack,int i32ID);
    virtual ~C_GetParameterCmd();
    
    virtual E_HandleAck_Retcode HandleAck(I_PacketParser *packet);
    
private:
    int    m_i32ID;
    BYTE   m_TempData[64];
    
};

class C_SetParameterCmd : public C_VideoSteamCommmadBase
{
public:
    
    C_SetParameterCmd(I_CommandAgent* pCommandAgnet,I_ViedoFileAgent *pFileAgent,PFN_CmdStatusCallBack StatusCallBack,int i32ID, int i32Size, BYTE*pbyData);
    virtual ~C_SetParameterCmd();
    
};

/***************************************************************************************
 Firmware mode
 ***************************************************************************************/
class C_FirmwareDownloadCmd : public C_VideoSteamCommmadBase
{
public:
    
    C_FirmwareDownloadCmd(I_CommandAgent* pCommandAgnet,I_ViedoFileAgent *pFileAgent,PFN_CmdStatusCallBack StatusCallBack,unsigned int ui32FileSize,unsigned int ui32CheckSum);
    virtual ~C_FirmwareDownloadCmd();

    
private:
    
};

class C_FirmwareSendRawDataCmd : public C_VideoSteamCommmadBase
{
public:
    
    C_FirmwareSendRawDataCmd(I_CommandAgent* pCommandAgnet,I_ViedoFileAgent *pFileAgent,PFN_CmdStatusCallBack StatusCallBack,unsigned int i32DataSize,BYTE *pbyData);
    virtual ~C_FirmwareSendRawDataCmd();
    
private:
    
};

class C_FirmwareUpgradeCmd : public C_VideoSteamCommmadBase
{
public:
    
    C_FirmwareUpgradeCmd(I_CommandAgent* pCommandAgnet,I_ViedoFileAgent *pFileAgent,PFN_CmdStatusCallBack StatusCallBack);
    virtual ~C_FirmwareUpgradeCmd();
    
    virtual unsigned int GetTimeout() { return FW_UPGRADE_TIMEOUT; }
    
private:
    
};

/***************************************************************************************
 CV Firmware mode
 ***************************************************************************************/
class C_CVFirmwareDownloadCmd : public C_VideoSteamCommmadBase
{
public:

	C_CVFirmwareDownloadCmd(I_CommandAgent* pCommandAgnet,I_ViedoFileAgent *pFileAgent,PFN_CmdStatusCallBack StatusCallBack,unsigned int ui32FileSize,unsigned int ui32CheckSum);
    virtual ~C_CVFirmwareDownloadCmd();


private:

};

class C_CVFirmwareSendRawDataCmd : public C_VideoSteamCommmadBase
{
public:

	C_CVFirmwareSendRawDataCmd(I_CommandAgent* pCommandAgnet,I_ViedoFileAgent *pFileAgent,PFN_CmdStatusCallBack StatusCallBack,unsigned int i32DataSize,BYTE *pbyData);
    virtual ~C_CVFirmwareSendRawDataCmd();

private:

};

class C_CVFirmwareUpgradeCmd : public C_VideoSteamCommmadBase
{
public:

	C_CVFirmwareUpgradeCmd(I_CommandAgent* pCommandAgnet,I_ViedoFileAgent *pFileAgent,PFN_CmdStatusCallBack StatusCallBack,unsigned int i32Area);
    virtual ~C_CVFirmwareUpgradeCmd();

    virtual unsigned int GetTimeout() { return FW_UPGRADE_TIMEOUT; }

private:

};

/***************************************************************************************
 Vendor mode
 ***************************************************************************************/

class C_SendVendorCmd : public C_VideoSteamCommmadBase
{
public:
    
    C_SendVendorCmd(I_CommandAgent* pCommandAgnet,I_ViedoFileAgent *pFileAgent,PFN_CmdStatusCallBack StatusCallBack,BYTE *pbyData,int i32Value);
    virtual ~C_SendVendorCmd();
    
};

/***************************************************************************************
 Action
 ***************************************************************************************/

class C_DelayCmd : public C_VideoSteamCommmadBase
{
public:
    
    C_DelayCmd(I_CommandAgent* pCommandAgnet,I_ViedoFileAgent *pFileAgent,PFN_CmdStatusCallBack StatusCallBack,int i32Delayms);
    virtual ~C_DelayCmd();
    
    virtual void DoAction();
    
private:
    
    int m_i32Delayms;
    
};

/***************************************************************************************
 Playback mode for Reverse
 ***************************************************************************************/

class C_GetPlaybackNameListCmdReverse : public C_VideoSteamCommmadBase
{
public:

    C_GetPlaybackNameListCmdReverse(I_CommandAgent* pCommandAgnet,I_ViedoFileAgent *pFileAgent,PFN_CmdStatusCallBack StatusCallBack,int i32Index,int i32MaxCount,int i32IndexOnDevice = -1 );
    virtual ~C_GetPlaybackNameListCmdReverse();

    virtual E_HandleAck_Retcode HandleAck(I_PacketParser *packet);

private:
    int m_i32Index;
    int m_i32MaxCount;

};


#endif /* defined(__GPCam__VideoSteamCommnad__) */
