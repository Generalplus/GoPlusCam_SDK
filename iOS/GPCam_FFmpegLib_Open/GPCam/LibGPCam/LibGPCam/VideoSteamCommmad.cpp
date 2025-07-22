//
//  VideoSteamCommnad.cpp
//  GPCam
//
//  Created by generalplus_sa1 on 8/12/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#include "VideoSteamCommmad.h"
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

/***************************************************************************************
 General mode
 ***************************************************************************************/

//----------------------------------------------------------
//Set mode
//----------------------------------------------------------
C_SetModeCmd::C_SetModeCmd(I_CommandAgent* pCommandAgnet,
		I_ViedoFileAgent *pFileAgent, PFN_CmdStatusCallBack StatusCallBack,
		int i32Mode) :
		C_VideoSteamCommmadBase(pCommandAgnet, pFileAgent, StatusCallBack,
				E_CommandType_Set) {
	BYTE byMode = i32Mode & 0xFF;

	SetPacketload(GP_SOCK_TYPE_CMD, GPSOCK_MODE_General,
			GPSOCK_General_CMD_SetMode, &byMode, 1);

}

C_SetModeCmd::~C_SetModeCmd() {

}

//----------------------------------------------------------
//Get Set PIP
//----------------------------------------------------------

C_GetSetPIPCmd::C_GetSetPIPCmd(I_CommandAgent* pCommandAgnet,
        I_ViedoFileAgent *pFileAgent, PFN_CmdStatusCallBack StatusCallBack,
        int i32Type) :
        C_VideoSteamCommmadBase(pCommandAgnet, pFileAgent, StatusCallBack,
                E_CommandType_Set) {
    BYTE byType = i32Type & 0xFF;

    SetPacketload(GP_SOCK_TYPE_CMD, GPSOCK_MODE_General,
                  GPSOCK_General_CMD_GetSetPIP, &byType, 1);

}

C_GetSetPIPCmd::~C_GetSetPIPCmd() {

}

//----------------------------------------------------------
//Get device status
//----------------------------------------------------------
C_GetDevuceStatusCmd::C_GetDevuceStatusCmd(I_CommandAgent* pCommandAgnet,
		I_ViedoFileAgent *pFileAgent, PFN_CmdStatusCallBack StatusCallBack) :
		C_VideoSteamCommmadBase(pCommandAgnet, pFileAgent, StatusCallBack,
				E_CommandType_Get) {

	SetPacketload(GP_SOCK_TYPE_CMD, GPSOCK_MODE_General,
			GPSOCK_General_CMD_GetDeviceStatus, NULL, 0);

}

C_GetDevuceStatusCmd::~C_GetDevuceStatusCmd() {

}

//----------------------------------------------------------
E_HandleAck_Retcode C_GetDevuceStatusCmd::HandleAck(I_PacketParser *packet) {
	E_HandleAck_Retcode RetCode = C_VideoSteamCommmadBase::HandleAck(packet);
	if (RetCode != E_HandleAck_Retcode_NoError) {
		return RetCode;
	}

	if (m_CmdStatuCallBack)
		m_CmdStatuCallBack(GetCMDIdex(), packet->GetType(), GetMode(), GetCMD(),
				packet->GetPayloadSize(), packet->GetPayload());

	return RetCode;
}

//----------------------------------------------------------
//Get parameter file
//----------------------------------------------------------
C_GetParameterFileCmd::C_GetParameterFileCmd(I_CommandAgent* pCommandAgnet,
		I_ViedoFileAgent *pFileAgent, PFN_CmdStatusCallBack StatusCallBack,
		const char* pathToSave) :
		C_VideoSteamCommmadBase(pCommandAgnet, pFileAgent, StatusCallBack,
				E_CommandType_Get), m_fp(NULL), m_i32writeSize(0) {
	strncpy(m_szFilePath, pathToSave, 256);
	SetPacketload(GP_SOCK_TYPE_CMD, GPSOCK_MODE_General,
			GPSOCK_General_CMD_GetParameterFile, NULL, 0);
	m_fp = fopen(m_szFilePath, "wb+");
}

C_GetParameterFileCmd::~C_GetParameterFileCmd() {
	if (m_fp)
		fclose(m_fp);
}
//----------------------------------------------------------
E_HandleAck_Retcode C_GetParameterFileCmd::HandleAck(I_PacketParser *packet) {
	E_HandleAck_Retcode RetCode = C_VideoSteamCommmadBase::HandleAck(packet);
	if (RetCode != E_HandleAck_Retcode_NoError) {
		return RetCode;
	}

	if (m_fp == NULL) {
		short ErrorCode = Error_WriteFail;

		if (m_CmdStatuCallBack)
			m_CmdStatuCallBack(GetCMDIdex(), GP_SOCK_TYPE_NAK, GetMode(),
					GetCMD(), sizeof(ErrorCode), (BYTE*) &ErrorCode);

		return E_HandleAck_Retcode_Failed;
	}

	if (packet->GetPayloadSize() == 0) {
		DEBUG_PRINT("remove 0xFF\n");
		int i = 1;
		BYTE byReadData = 0x00;
		BYTE byWriteData = 0x00;

		while (m_i32writeSize > 0) {
			fseek(m_fp, -i, SEEK_END);
			byReadData = fgetc(m_fp);
			if (byReadData != 0xFF)
				break;

			fseek(m_fp, -i, SEEK_END);
			fputc(byWriteData, m_fp);
			i++;
			m_i32writeSize--;
		}

		DEBUG_PRINT("close File\n");
		fclose(m_fp);
		m_fp = NULL;

		if (m_CmdStatuCallBack)
			m_CmdStatuCallBack(GetCMDIdex(), packet->GetType(), GetMode(),
					GetCMD(), 0, 0);

	} else {
		m_i32writeSize += packet->GetPayloadSize();
		fwrite(packet->GetPayload(), 1, packet->GetPayloadSize(), m_fp);
		return E_HandleAck_Retcode_RequestMore;
	}

	return RetCode;
}
//----------------------------------------------------------
//Send Power off
//----------------------------------------------------------
C_SendPowerOffCmd::C_SendPowerOffCmd(I_CommandAgent* pCommandAgnet,
		I_ViedoFileAgent *pFileAgent, PFN_CmdStatusCallBack StatusCallBack) :
		C_VideoSteamCommmadBase(pCommandAgnet, pFileAgent, StatusCallBack,
				E_CommandType_Set) {

	SetPacketload(GP_SOCK_TYPE_CMD, GPSOCK_MODE_General,
			GPSOCK_General_CMD_Poweroff, NULL, 0);

}

C_SendPowerOffCmd::~C_SendPowerOffCmd() {

}

//----------------------------------------------------------
//Send Restart Streaming
//----------------------------------------------------------
C_SendRestartStreamingCmd::C_SendRestartStreamingCmd(
		I_CommandAgent* pCommandAgnet, I_ViedoFileAgent *pFileAgent,
		PFN_CmdStatusCallBack StatusCallBack) :
		C_VideoSteamCommmadBase(pCommandAgnet, pFileAgent, StatusCallBack,
				E_CommandType_Set) {

	SetPacketload(GP_SOCK_TYPE_CMD, GPSOCK_MODE_General,
			GPSOCK_General_CMD_RestartStreaming, NULL, 0);

}

C_SendRestartStreamingCmd::~C_SendRestartStreamingCmd() {

}

//----------------------------------------------------------
//Auth Device
//----------------------------------------------------------
C_AuthDeviceCmd::C_AuthDeviceCmd(I_CommandAgent* pCommandAgnet,
		I_ViedoFileAgent *pFileAgent, PFN_CmdStatusCallBack StatusCallBack) :
		C_VideoSteamCommmadBase(pCommandAgnet, pFileAgent, StatusCallBack,
				E_CommandType_Get) {
	int i32Ramdom;

	srand (time(NULL));i32Ramdom
	= rand();
	m_LFSRseed[0] = i32Ramdom & 0xFF;
	m_LFSRseed[1] = (i32Ramdom >> 8) & 0xFF;
	m_LFSRseed[2] = (i32Ramdom >> 16) & 0xFF;
	m_LFSRseed[3] = (i32Ramdom >> 24) & 0xFF;

	SetPacketload(GP_SOCK_TYPE_CMD, GPSOCK_MODE_General,
			GPSOCK_General_CMD_AuthDevice, m_LFSRseed, sizeof(m_LFSRseed));

}

C_AuthDeviceCmd::~C_AuthDeviceCmd() {

}

void C_AuthDeviceCmd::Gen_LFSR_32Bit_Key(BYTE *pbySeed, BYTE *pbyKey,
		BYTE *pbyPayload) {
#define code
#define xdata
#define data

#define U8		unsigned char
#define U16		unsigned short
#define bit		U8

	static U8 LFSR_LUT[16] = { 0x0E, 0x47, 0xDB, 0x46, 0x46, 0x8D, 0x38, 0xE5,
			0xFC, 0x52, 0x7A, 0xDE, 0x6F, 0xC5, 0x05, 0xE6 };

	U8 tc_period, q;
	U16 tw_LFSR_Key;
	U16 tw_LFSR_MagicNum;

	((U8*) (&tw_LFSR_MagicNum))[0] = pbySeed[0] ^ pbyPayload[0];
	((U8*) (&tw_LFSR_MagicNum))[1] = pbySeed[1] ^ pbyPayload[1];
	((U8*) (&tw_LFSR_Key))[0] = pbySeed[2] ^ pbyPayload[2];
	((U8*) (&tw_LFSR_Key))[1] = pbySeed[3] ^ pbyPayload[3];

	tc_period = (((U8) tw_LFSR_MagicNum) ^ ((U8) (tw_LFSR_MagicNum >> 8)))
			& 0x0F;
	tc_period = ((LFSR_LUT[tc_period] ^ pbySeed[0])
			+ (LFSR_LUT[15 - tc_period] ^ pbySeed[1]));
	for (q = 0; q < tc_period; q++) {
		bit tb_bit = (bit) ((tw_LFSR_Key ^ (tw_LFSR_Key >> 2)
				^ (tw_LFSR_Key >> 3) ^ (tw_LFSR_Key >> 5)) & 0x01);
		tw_LFSR_Key = (((U16) tb_bit) << 15) | (tw_LFSR_Key >> 1);
	}
	pbyKey[0] = ((U8*) (&tw_LFSR_Key))[0];
	pbyKey[1] = ((U8*) (&tw_LFSR_Key))[1];

	//DEBUG_PRINT("\nGen_LFSR_32Bit_Key:\n");

	//DEBUG_PRINT("in: 0x%.2x 0x%.2x 0x%.2x 0x%.2x\n",pbySeed[0],pbySeed[1],pbySeed[2],pbySeed[3]);
	//DEBUG_PRINT("out:0x%.2x 0x%.2x\n",pbyKey[0],pbyKey[1]);
	//DEBUG_PRINT("tc_period: 0x%.2x\n",tc_period);

}

E_HandleAck_Retcode C_AuthDeviceCmd::HandleAck(I_PacketParser *packet) {
	E_HandleAck_Retcode RetCode = C_VideoSteamCommmadBase::HandleAck(packet);
	if (RetCode != E_HandleAck_Retcode_NoError) {
		RetCode = E_HandleAck_Retcode_StopConnection;
		return RetCode;
	}

	BYTE* pbyPlayload = packet->GetPayload();
	BYTE byLFSRKeyDevice[2], byLFSRKey[2];
	byLFSRKeyDevice[0] = pbyPlayload[4];
	byLFSRKeyDevice[1] = pbyPlayload[5];

	Gen_LFSR_32Bit_Key(m_LFSRseed, byLFSRKey, pbyPlayload);

	if (byLFSRKeyDevice[0] != byLFSRKey[0]
			|| byLFSRKeyDevice[1] != byLFSRKey[1]) {
		short ErrorCode = Error_InvalidCommand;

		if (m_CmdStatuCallBack)
			m_CmdStatuCallBack(GetCMDIdex(), GP_SOCK_TYPE_NAK, GetMode(),
					GetCMD(), sizeof(ErrorCode), (BYTE*) &ErrorCode);

		RetCode = E_HandleAck_Retcode_StopConnection;
	} else {
		// No need to ack , Auth command send by agent
		//if(m_CmdStatuCallBack)
		//    m_CmdStatuCallBack(GetCMDIdex(),packet->GetType(),GetMode(),GetCMD(),packet->GetPayloadSize(),packet->GetPayload());

	}

	DEBUG_PRINT("\nC_AuthDeviceCmd OK\n");
	return RetCode;
}

//----------------------------------------------------------
//Auth Device for Mapping
//----------------------------------------------------------
C_CheckFileMappingCmd::C_CheckFileMappingCmd(I_CommandAgent* pCommandAgnet,
		I_ViedoFileAgent *pFileAgent, PFN_CmdStatusCallBack StatusCallBack) :
		C_VideoSteamCommmadBase(pCommandAgnet, pFileAgent, StatusCallBack,
				E_CommandType_Get) {
	int i32Ramdom;

	srand (time(NULL));i32Ramdom
	= rand();
	m_seedMapping[0] = i32Ramdom & 0xFF;
	m_seedMapping[1] = (i32Ramdom >> 8) & 0xFF;
	m_seedMapping[2] = (i32Ramdom >> 16) & 0xFF;
	m_seedMapping[3] = (i32Ramdom >> 24) & 0xFF;

	SetPacketload(GP_SOCK_TYPE_CMD, GPSOCK_MODE_General,
			GPSOCK_General_CMD_CheckMapping, m_seedMapping,
			sizeof(m_seedMapping));

}

C_CheckFileMappingCmd::~C_CheckFileMappingCmd() {

}

void C_CheckFileMappingCmd::Gen_32Bit_key(BYTE *pbyKey) {
	static U8 LFSR_LUT[16] = { 0x0E, 0x47, 0xDB, 0x46, 0x46, 0x8D, 0x38, 0xE5,
			0xFC, 0x52, 0x7A, 0xDE, 0x6F, 0xC5, 0x05, 0xE6 };

	DEBUG_PRINT("\nm_seedMapping, %x  %x %x %x\n", m_seedMapping[0],
			m_seedMapping[1], m_seedMapping[2], m_seedMapping[3]);

	int idx = 0;
	for (int i = 0; i < 4; i++) {
		idx = (m_seedMapping[i] + (m_seedMapping[i] >> 4)) & 0xf;

		pbyKey[i] = m_seedMapping[i] ^ LFSR_LUT[idx];

		DEBUG_PRINT("\npbyKey, Index: %d pbyKey:%x  , idx = %d\n", i, pbyKey[i],
				idx);
	}

}

E_HandleAck_Retcode C_CheckFileMappingCmd::HandleAck(I_PacketParser *packet) {
	E_HandleAck_Retcode RetCode = C_VideoSteamCommmadBase::HandleAck(packet);
	if (RetCode != E_HandleAck_Retcode_NoError) {
		RetCode = E_HandleAck_Retcode_NoError;
		return RetCode;
	}

	BYTE* pbyPlayload = packet->GetPayload();
	BYTE byKeyDevice[4], byKey[4];
	byKeyDevice[0] = pbyPlayload[0];
	byKeyDevice[1] = pbyPlayload[1];
	byKeyDevice[2] = pbyPlayload[2];
	byKeyDevice[3] = pbyPlayload[3];

	for (int i = 0; i < 8; i++) {

		DEBUG_PRINT("\npbyPlayload, Index: %d pbyPlayload:%x ", i,
				pbyPlayload[i]);
	}

	Gen_32Bit_key(byKey);

	if (byKeyDevice[0] != byKey[0] || byKeyDevice[1] != byKey[1]
			|| byKeyDevice[2] != byKey[2] || byKeyDevice[3] != byKey[3]) {
		short ErrorCode = Error_InvalidCommand;

		if (m_CmdStatuCallBack)
			m_CmdStatuCallBack(GetCMDIdex(), GP_SOCK_TYPE_NAK, GetMode(),
					GetCMD(), sizeof(ErrorCode), (BYTE*) &ErrorCode);

		RetCode = E_HandleAck_Retcode_Failed;
	} else {
		// No need to ack , Auth command send by agent
		if (m_CmdStatuCallBack)
			m_CmdStatuCallBack(GetCMDIdex(), packet->GetType(), GetMode(),
					GetCMD(), packet->GetPayloadSize(), packet->GetPayload());

	}

	DEBUG_PRINT("\nC_CheckFileMappingCmd OK\n");
	return RetCode;
}

/***************************************************************************************
 Record mode
 ***************************************************************************************/
//----------------------------------------------------------
//Send Record
//----------------------------------------------------------
C_RecordCmd::C_RecordCmd(I_CommandAgent* pCommandAgnet,
		I_ViedoFileAgent *pFileAgent, PFN_CmdStatusCallBack StatusCallBack) :
		C_VideoSteamCommmadBase(pCommandAgnet, pFileAgent, StatusCallBack,
				E_CommandType_Set) {
	SetPacketload(GP_SOCK_TYPE_CMD, GPSOCK_MODE_Record, GPSOCK_Record_CMD_Start,
			NULL, 0);
}

C_RecordCmd::~C_RecordCmd() {

}

//----------------------------------------------------------
//Audio On / Off
//----------------------------------------------------------
C_AudioCmd::C_AudioCmd(I_CommandAgent* pCommandAgnet,
		I_ViedoFileAgent *pFileAgent, PFN_CmdStatusCallBack StatusCallBack,
		bool bOn) :
		C_VideoSteamCommmadBase(pCommandAgnet, pFileAgent, StatusCallBack,
				E_CommandType_Set) {
	BYTE byIsOn = bOn;
	SetPacketload(GP_SOCK_TYPE_CMD, GPSOCK_MODE_Record, GPSOCK_Record_CMD_Audio,
			&byIsOn, 1);

}

C_AudioCmd::~C_AudioCmd() {

}
/***************************************************************************************
 Capture picture mode
 ***************************************************************************************/

//----------------------------------------------------------
//Capture PictureCmd
//----------------------------------------------------------
C_CapturePictureCmd::C_CapturePictureCmd(I_CommandAgent* pCommandAgnet,
		I_ViedoFileAgent *pFileAgent, PFN_CmdStatusCallBack StatusCallBack) :
		C_VideoSteamCommmadBase(pCommandAgnet, pFileAgent, StatusCallBack,
				E_CommandType_Set) {
	SetPacketload(GP_SOCK_TYPE_CMD, GPSOCK_MODE_CapturePicture,
			GPSOCK_CapturePicture_CMD_Capture, NULL, 0);
}

C_CapturePictureCmd::~C_CapturePictureCmd() {

}

/***************************************************************************************
 Playback mode
 ***************************************************************************************/
//----------------------------------------------------------
//Playback Start
//----------------------------------------------------------
C_PlaybackStartCmd::C_PlaybackStartCmd(I_CommandAgent* pCommandAgnet,
		I_ViedoFileAgent *pFileAgent, PFN_CmdStatusCallBack StatusCallBack,
		int i32Index) :
		C_VideoSteamCommmadBase(pCommandAgnet, pFileAgent, StatusCallBack,
				E_CommandType_Set) {
	BYTE byIndex[2];
	int i32DeviceIdx = m_pFileAgent->GetFileIndex(i32Index);

	byIndex[0] = i32DeviceIdx & 0xFF;
	byIndex[1] = (i32DeviceIdx >> 8) & 0xFF;

	SetPacketload(GP_SOCK_TYPE_CMD, GPSOCK_MODE_Playback,
			GPSOCK_Playback_CMD_Start, byIndex, 2);
}

C_PlaybackStartCmd::~C_PlaybackStartCmd() {

}

//----------------------------------------------------------
//Playback Pause
//----------------------------------------------------------
C_PlaybackPauseCmd::C_PlaybackPauseCmd(I_CommandAgent* pCommandAgnet,
		I_ViedoFileAgent *pFileAgent, PFN_CmdStatusCallBack StatusCallBack,
		int i32Index) :
		C_VideoSteamCommmadBase(pCommandAgnet, pFileAgent, StatusCallBack,
				E_CommandType_Set) {
	BYTE byType;
	byType = pFileAgent->GetFileExt(i32Index);
	SetPacketload(GP_SOCK_TYPE_CMD, GPSOCK_MODE_Playback,
			GPSOCK_Playback_CMD_Pause, &byType, 1);
}

C_PlaybackPauseCmd::~C_PlaybackPauseCmd() {

}

//----------------------------------------------------------
//Playback get file count
//----------------------------------------------------------
C_GetPlaybackFileCountCmd::C_GetPlaybackFileCountCmd(
		I_CommandAgent* pCommandAgnet, I_ViedoFileAgent *pFileAgent,
		PFN_CmdStatusCallBack StatusCallBack) :
		C_VideoSteamCommmadBase(pCommandAgnet, pFileAgent, StatusCallBack,
				E_CommandType_Get) {
	SetPacketload(GP_SOCK_TYPE_CMD, GPSOCK_MODE_Playback,
			GPSOCK_Playback_CMD_GetFileCount, NULL, 0);
}

C_GetPlaybackFileCountCmd::~C_GetPlaybackFileCountCmd() {

}
//----------------------------------------------------------
E_HandleAck_Retcode C_GetPlaybackFileCountCmd::HandleAck(
		I_PacketParser *packet) {
	E_HandleAck_Retcode RetCode = C_VideoSteamCommmadBase::HandleAck(packet);
	if (RetCode != E_HandleAck_Retcode_NoError) {
		return RetCode;
	}

	if (m_CmdStatuCallBack) {
		//DEBUG_PRINT("FileCountCmd Index:%d Device index: %d\n", m_i32Index,
		//		i32DeviceIdx);
		m_CmdStatuCallBack(GetCMDIdex(), packet->GetType(), GetMode(), GetCMD(),
				packet->GetPayloadSize(), packet->GetPayload());
	}

	BYTE* pbyPlayload = packet->GetPayload();
	int i32FileCount = pbyPlayload[0];
	i32FileCount |= pbyPlayload[1] << 8;

	DEBUG_PRINT("FileCountCmd i32FileCount:%d  \n", i32FileCount);
	if (i32FileCount > 0) {
		m_pFileAgent->AllocFileList(i32FileCount);

		C_GetPlaybackNameListCmd *pNameList = new C_GetPlaybackNameListCmd(
				m_pCommnadAgent, m_pFileAgent, m_CmdStatuCallBack, 0,
				i32FileCount,true);
		m_pCommnadAgent->QueueCmd(pNameList);
	}

	return RetCode;
}

//----------------------------------------------------------
//Playback get file name list
//----------------------------------------------------------
C_GetPlaybackNameListCmd::C_GetPlaybackNameListCmd(
		I_CommandAgent* pCommandAgnet, I_ViedoFileAgent *pFileAgent,
		PFN_CmdStatusCallBack StatusCallBack, int i32Index, int i32MaxCount,bool bFirst,
		int i32IndexOnDevice) :
		C_VideoSteamCommmadBase(pCommandAgnet, pFileAgent, StatusCallBack,
				E_CommandType_Get), m_i32Index(i32Index), m_i32MaxCount(
				i32MaxCount) {
	BYTE byData[3];
	int i32DeviceIdx = 0x00;

	if (bFirst)
		byData[0] = 0x01; // first file
	else {
		byData[0] = 0x00;

		if (i32IndexOnDevice == -1)
			i32DeviceIdx = pFileAgent->GetFileIndex(i32Index);
		else
			i32DeviceIdx = i32IndexOnDevice;

		DEBUG_PRINT("Index:%d Device index: %d\n", m_i32Index, i32DeviceIdx);

		m_i32Index++;
	}

	byData[1] = i32DeviceIdx & 0xFF;
	byData[2] = (i32DeviceIdx >> 8) & 0xFF;

	SetPacketload(GP_SOCK_TYPE_CMD, GPSOCK_MODE_Playback,
			GPSOCK_Playback_CMD_GetNameList, byData, sizeof(byData));
}

C_GetPlaybackNameListCmd::~C_GetPlaybackNameListCmd() {

}

//----------------------------------------------------------
E_HandleAck_Retcode C_GetPlaybackNameListCmd::HandleAck(
		I_PacketParser *packet) {
	E_HandleAck_Retcode RetCode = C_VideoSteamCommmadBase::HandleAck(packet);
	if (RetCode != E_HandleAck_Retcode_NoError) {
		return RetCode;
	}

	if (packet->GetPayloadSize() == 0) {
		if (m_CmdStatuCallBack)
			m_CmdStatuCallBack(GetCMDIdex(), packet->GetType(), GetMode(),
					GetCMD(), 0, 0);
	} else {

		BYTE* pbyPlayload = packet->GetPayload();
		BYTE byCount = pbyPlayload[0];

		int i32FileAttrSize = Get_FileAttribute_RealSize;
		int i32TotalSize = Get_FileAttribute_RealSize * byCount;
		int i32ExtraSize = (((packet->GetPayloadSize() - 1) - i32TotalSize)
				/ byCount);
		i32FileAttrSize += i32ExtraSize;

		for (int i = 0; i < byCount; i++)
			m_pFileAgent->SetFileAttr(m_i32Index + i,
					&pbyPlayload[1 + (i32FileAttrSize * i)], i32ExtraSize);

		BYTE byData[3];
		byData[0] = m_i32Index & 0xFF;
		byData[1] = (m_i32Index >> 8) & 0xFF;
		byData[2] = byCount;

		if (m_CmdStatuCallBack)
			m_CmdStatuCallBack(GetCMDIdex(), packet->GetType(), GetMode(),
					GetCMD(), sizeof(byData), byData);

		DEBUG_PRINT("\nFileName, Index: %d Size:%d\n", m_i32Index, byCount);

		int i32NextPreview = m_i32Index + byCount - 1;
		if (byCount > 0 && m_pFileAgent->GetNextPreview(&i32NextPreview)) {
			DEBUG_PRINT("\nNextPreview, Index: %d\n", i32NextPreview);

			if (m_pFileAgent->GetFileIndex(i32NextPreview) == -1) {
				DEBUG_PRINT("\nSpecificName Index: %d\n", i32NextPreview);
				C_GetPlaybackSpecificName *pNameList =
						new C_GetPlaybackSpecificName(m_pCommnadAgent,
								m_pFileAgent, m_CmdStatuCallBack,
								i32NextPreview, m_i32MaxCount);
				m_pCommnadAgent->QueueCmd(pNameList);
			} else {
				C_GetPlaybackNameListCmd *pNameList =
						new C_GetPlaybackNameListCmd(m_pCommnadAgent,
								m_pFileAgent, m_CmdStatuCallBack,
								i32NextPreview, m_i32MaxCount,false);
				m_pCommnadAgent->QueueCmd(pNameList);
			}
		}
	}

	return RetCode;
}

//----------------------------------------------------------
//Playback get file Thumbnail
//----------------------------------------------------------
C_GetPlaybackThumbnailCmd::C_GetPlaybackThumbnailCmd(
		I_CommandAgent* pCommandAgnet, I_ViedoFileAgent *pFileAgent,
		PFN_CmdStatusCallBack StatusCallBack, int i32Index,
		const char* pathToSave) :
		C_VideoSteamCommmadBase(pCommandAgnet, pFileAgent, StatusCallBack,
				E_CommandType_Get), m_i32Index(i32Index) {
	strncpy(m_szFilePath, pathToSave, 256);
	BYTE byIndex[2];
	int i32DeviceIdx = m_pFileAgent->GetFileIndex(i32Index);

	byIndex[0] = i32DeviceIdx & 0xFF;
	byIndex[1] = (i32DeviceIdx >> 8) & 0xFF;

	SetPacketload(GP_SOCK_TYPE_CMD, GPSOCK_MODE_Playback,
			GPSOCK_Playback_CMD_GetThumbnail, byIndex, 2);
	m_fp = fopen(m_szFilePath, "wb+");
}

C_GetPlaybackThumbnailCmd::~C_GetPlaybackThumbnailCmd() {
	if (m_fp) {
		fclose(m_fp);
		m_fp = NULL;
	}
}

//----------------------------------------------------------
E_HandleAck_Retcode C_GetPlaybackThumbnailCmd::HandleAck(
		I_PacketParser *packet) {
	E_HandleAck_Retcode RetCode = C_VideoSteamCommmadBase::HandleAck(packet);
	if (RetCode != E_HandleAck_Retcode_NoError) {
		return RetCode;
	}

	if (m_fp == NULL) {
		short ErrorCode = Error_GetThumbnailFail;

		if (m_CmdStatuCallBack)
			m_CmdStatuCallBack(GetCMDIdex(), GP_SOCK_TYPE_NAK, GetMode(),
					GetCMD(), sizeof(ErrorCode), (BYTE*) &ErrorCode);

		return E_HandleAck_Retcode_Failed;
	}

	if (packet->GetPayloadSize() == 0) {
		DEBUG_PRINT("Thumbnail ok\n");
		fclose(m_fp);
		m_fp = NULL;

		BYTE Data[512];
		Data[0] = m_i32Index & 0xFF;
		Data[1] = (m_i32Index >> 8) & 0xFF;

		int i32Len = strlen(m_szFilePath);
		Data[2] = i32Len & 0xFF;
		Data[3] = (i32Len >> 8) & 0xFF;

		memcpy(&Data[4], m_szFilePath, i32Len);

		if (m_CmdStatuCallBack)
			m_CmdStatuCallBack(GetCMDIdex(), packet->GetType(), GetMode(),
					GetCMD(), 512, Data);

	} else {

		fwrite(packet->GetPayload(), 1, packet->GetPayloadSize(), m_fp);
		return E_HandleAck_Retcode_RequestMore;
	}

	return RetCode;
}

//----------------------------------------------------------
//Playback get file raw data
//----------------------------------------------------------
C_GetPlaybackFileRawDataCmd::C_GetPlaybackFileRawDataCmd(
		I_CommandAgent* pCommandAgnet, I_ViedoFileAgent *pFileAgent,
		PFN_CmdStatusCallBack StatusCallBack, int i32Index,
		const char* pathToSave) :
		C_VideoSteamCommmadBase(pCommandAgnet, pFileAgent, StatusCallBack,
				E_CommandType_Get), m_i32Index(i32Index) {
	strncpy(m_szFilePath, pathToSave, 256);
	BYTE byIndex[2];
	int i32DeviceIdx = m_pFileAgent->GetFileIndex(i32Index);

	m_i32FileSize = m_pFileAgent->GetFileSize(i32Index);
	m_i32SizeGot = 0;

	byIndex[0] = i32DeviceIdx & 0xFF;
	byIndex[1] = (i32DeviceIdx >> 8) & 0xFF;

	SetPacketload(GP_SOCK_TYPE_CMD, GPSOCK_MODE_Playback,
			GPSOCK_Playback_CMD_GetRawData, byIndex, 2);
	m_fp = fopen(m_szFilePath, "wb+");
}

C_GetPlaybackFileRawDataCmd::~C_GetPlaybackFileRawDataCmd() {
	if (m_fp) {
		fclose(m_fp);
		m_fp = NULL;
	}
}

//----------------------------------------------------------
E_HandleAck_Retcode C_GetPlaybackFileRawDataCmd::HandleAck(
		I_PacketParser *packet) {
	E_HandleAck_Retcode RetCode = C_VideoSteamCommmadBase::HandleAck(packet);
	if (RetCode != E_HandleAck_Retcode_NoError) {
		return RetCode;
	}

	if (m_fp == NULL) {
		short ErrorCode = Error_WriteFail;

		if (m_CmdStatuCallBack)
			m_CmdStatuCallBack(GetCMDIdex(), GP_SOCK_TYPE_NAK, GetMode(),
					GetCMD(), sizeof(ErrorCode), (BYTE*) &ErrorCode);

		return E_HandleAck_Retcode_Failed;
	}

	if (packet->GetPayloadSize() == 0) {
		DEBUG_PRINT("RawData ok\n");
		fclose(m_fp);
		m_fp = NULL;

		BYTE Data[512];

		Data[0] = 0x01; //Finish
		Data[1] = m_i32Index & 0xFF;
		Data[2] = (m_i32Index >> 8) & 0xFF;

		int i32Len = strlen(m_szFilePath);
		Data[3] = i32Len & 0xFF;
		Data[4] = (i32Len >> 8) & 0xFF;

		memcpy(&Data[5], m_szFilePath, i32Len);

		if (m_CmdStatuCallBack)
			m_CmdStatuCallBack(GetCMDIdex(), packet->GetType(), GetMode(),
					GetCMD(), 512, Data);

	} else {
		fwrite(packet->GetPayload(), 1, packet->GetPayloadSize(), m_fp);
		m_i32SizeGot += packet->GetPayloadSize();

		BYTE Data[2];
		Data[0] = 0x00; //Finish
		Data[1] = (m_i32SizeGot / 1024 * 100) / m_i32FileSize;

		DEBUG_PRINT("RawData %d/%d (%d percent)\n", m_i32SizeGot/1024,
				m_i32FileSize, Data[1]);

		if (m_CmdStatuCallBack)
			m_CmdStatuCallBack(GetCMDIdex(), packet->GetType(), GetMode(),
					GetCMD(), 2, Data);

		return E_HandleAck_Retcode_RequestMore;
	}

	return RetCode;
}

//----------------------------------------------------------
//Playback Stop
//----------------------------------------------------------
C_PlaybackStopCmd::C_PlaybackStopCmd(I_CommandAgent* pCommandAgnet,
		I_ViedoFileAgent *pFileAgent, PFN_CmdStatusCallBack StatusCallBack,
		int i32Index) :
		C_VideoSteamCommmadBase(pCommandAgnet, pFileAgent, StatusCallBack,
				E_CommandType_Set) {
	BYTE byType;
	byType = m_pFileAgent->GetFileExt(i32Index);
	SetPacketload(GP_SOCK_TYPE_CMD, GPSOCK_MODE_Playback,
			GPSOCK_Playback_CMD_Stop, &byType, 1);
}

C_PlaybackStopCmd::~C_PlaybackStopCmd() {

}

//----------------------------------------------------------
//Playback Query File Index
//----------------------------------------------------------
C_GetPlaybackSpecificName::C_GetPlaybackSpecificName(
		I_CommandAgent* pCommandAgnet, I_ViedoFileAgent *pFileAgent,
		PFN_CmdStatusCallBack StatusCallBack, int i32Index, int i32MaxCount) :
		C_VideoSteamCommmadBase(pCommandAgnet, pFileAgent, StatusCallBack,
				E_CommandType_Get), m_i32QueryIndex(i32Index), m_i32MaxCount(
				i32MaxCount) {
	BYTE byData[2];

	i32Index++; //Device Index start from 1
	byData[0] = i32Index & 0xFF;
	byData[1] = (i32Index >> 8) & 0xFF;

	SetPacketload(GP_SOCK_TYPE_CMD, GPSOCK_MODE_Playback,
			GPSOCK_Playback_CMD_GetSpecificName, byData, sizeof(byData));
}

C_GetPlaybackSpecificName::~C_GetPlaybackSpecificName() {

}

//----------------------------------------------------------
E_HandleAck_Retcode C_GetPlaybackSpecificName::HandleAck(
		I_PacketParser *packet) {
	E_HandleAck_Retcode RetCode = C_VideoSteamCommmadBase::HandleAck(packet);
	if (RetCode != E_HandleAck_Retcode_NoError) {
		return RetCode;
	}

	BYTE* pbyPlayload = packet->GetPayload();
	int i32FileIndex = pbyPlayload[0];
	i32FileIndex |= pbyPlayload[1] << 8;

	C_GetPlaybackNameListCmd *pNameList = new C_GetPlaybackNameListCmd(
			m_pCommnadAgent, m_pFileAgent, m_CmdStatuCallBack, m_i32QueryIndex,
			m_i32MaxCount, i32FileIndex);
	m_pCommnadAgent->QueueCmd(pNameList);

	DEBUG_PRINT("Query:%d Get Index OnDevice: %d\n", m_i32QueryIndex,
			i32FileIndex);

	return RetCode;
}

//----------------------------------------------------------
//Playback Delete File
//----------------------------------------------------------
C_GetPlaybackDeleteFile::C_GetPlaybackDeleteFile(I_CommandAgent* pCommandAgnet,
		I_ViedoFileAgent *pFileAgent, PFN_CmdStatusCallBack StatusCallBack,
		int i32DeviceIndex, int i32Index) :
		C_VideoSteamCommmadBase(pCommandAgnet, pFileAgent, StatusCallBack,
				E_CommandType_Set), m_i32DeviceIndex(i32DeviceIndex), m_i32Index(
				i32Index) {
	BYTE byData[2];

	byData[0] = i32DeviceIndex & 0xFF;
	byData[1] = (i32DeviceIndex >> 8) & 0xFF;

	SetPacketload(GP_SOCK_TYPE_CMD, GPSOCK_MODE_Playback,
			GPSOCK_Playback_CMD_DeleteFile, byData, sizeof(byData));
}

C_GetPlaybackDeleteFile::~C_GetPlaybackDeleteFile() {

}

//----------------------------------------------------------
E_HandleAck_Retcode C_GetPlaybackDeleteFile::HandleAck(I_PacketParser *packet) {
	E_HandleAck_Retcode RetCode = C_VideoSteamCommmadBase::HandleAck(packet);
	if (RetCode != E_HandleAck_Retcode_NoError) {
		return RetCode;
	}

	DEBUG_PRINT("Delete file at %d Device Index %d \n", m_i32Index,
			m_i32DeviceIndex);

	m_pFileAgent->RemoveFileAttr(m_i32Index);

	return RetCode;
}

/***************************************************************************************
 Menu mode
 ***************************************************************************************/

//----------------------------------------------------------
//Get parameter
//----------------------------------------------------------
C_GetParameterCmd::C_GetParameterCmd(I_CommandAgent* pCommandAgnet,
		I_ViedoFileAgent *pFileAgent, PFN_CmdStatusCallBack StatusCallBack,
		int i32ID) :
		C_VideoSteamCommmadBase(pCommandAgnet, pFileAgent, StatusCallBack,
				E_CommandType_Get) {
	BYTE byID[4];

	byID[0] = i32ID & 0xFF;
	byID[1] = (i32ID >> 8) & 0xFF;
	byID[2] = (i32ID >> 16) & 0xFF;
	byID[3] = (i32ID >> 24) & 0xFF;
	m_i32ID = i32ID;
	SetPacketload(GP_SOCK_TYPE_CMD, GPSOCK_MODE_Menu,
			GPSOCK_Menu_CMD_GetParameter, byID, 4);
}

C_GetParameterCmd::~C_GetParameterCmd() {

}

//----------------------------------------------------------
E_HandleAck_Retcode C_GetParameterCmd::HandleAck(I_PacketParser *packet) {
	E_HandleAck_Retcode RetCode = C_VideoSteamCommmadBase::HandleAck(packet);
	if (RetCode != E_HandleAck_Retcode_NoError) {
		return RetCode;
	}

	m_TempData[0] = m_i32ID & 0xFF;
	m_TempData[1] = (m_i32ID >> 8) & 0xFF;
	m_TempData[2] = (m_i32ID >> 16) & 0xFF;
	m_TempData[3] = (m_i32ID >> 24) & 0xFF;

	memcpy(&m_TempData[4], packet->GetPayload(), packet->GetPayloadSize());

	if (m_CmdStatuCallBack)
		m_CmdStatuCallBack(GetCMDIdex(), packet->GetType(), GetMode(), GetCMD(),
				packet->GetPayloadSize() + sizeof(m_i32ID), m_TempData);

	return RetCode;
}

//----------------------------------------------------------
//Set parameter
//----------------------------------------------------------
C_SetParameterCmd::C_SetParameterCmd(I_CommandAgent* pCommandAgnet,
		I_ViedoFileAgent *pFileAgent, PFN_CmdStatusCallBack StatusCallBack,
		int i32ID, int i32Size, BYTE*pbyData) :
		C_VideoSteamCommmadBase(pCommandAgnet, pFileAgent, StatusCallBack,
				E_CommandType_Set) {
	BYTE byPlayload[1024];

	byPlayload[0] = i32ID & 0xFF;
	byPlayload[1] = (i32ID >> 8) & 0xFF;
	byPlayload[2] = (i32ID >> 16) & 0xFF;
	byPlayload[3] = (i32ID >> 24) & 0xFF;

	byPlayload[4] = i32Size & 0xFF;
	memcpy(&byPlayload[5], pbyData, i32Size);

	SetPacketload(GP_SOCK_TYPE_CMD, GPSOCK_MODE_Menu,
			GPSOCK_Menu_CMD_SetParameter, byPlayload, i32Size + 5);
}

C_SetParameterCmd::~C_SetParameterCmd() {

}
/***************************************************************************************
 Firmware mode
 ***************************************************************************************/

//----------------------------------------------------------
//Firmware Download
//----------------------------------------------------------
C_FirmwareDownloadCmd::C_FirmwareDownloadCmd(I_CommandAgent* pCommandAgnet,
		I_ViedoFileAgent *pFileAgent, PFN_CmdStatusCallBack StatusCallBack,
		unsigned int ui32FileSize, unsigned int ui32CheckSum) :
		C_VideoSteamCommmadBase(pCommandAgnet, pFileAgent, StatusCallBack,
				E_CommandType_Set) {
	BYTE byPlayload[8];

	byPlayload[0] = ui32FileSize & 0xFF;
	byPlayload[1] = (ui32FileSize >> 8) & 0xFF;
	byPlayload[2] = (ui32FileSize >> 16) & 0xFF;
	byPlayload[3] = (ui32FileSize >> 24) & 0xFF;

	byPlayload[4] = ui32CheckSum & 0xFF;
	byPlayload[5] = (ui32CheckSum >> 8) & 0xFF;
	byPlayload[6] = (ui32CheckSum >> 16) & 0xFF;
	byPlayload[7] = (ui32CheckSum >> 24) & 0xFF;

	SetPacketload(GP_SOCK_TYPE_CMD, GPSOCK_MODE_Firmware,
			GPSOCK_Firmware_CMD_Download, byPlayload, sizeof(byPlayload));
}

C_FirmwareDownloadCmd::~C_FirmwareDownloadCmd() {

}

//----------------------------------------------------------
//Firmware Send raw data
//----------------------------------------------------------
C_FirmwareSendRawDataCmd::C_FirmwareSendRawDataCmd(
		I_CommandAgent* pCommandAgnet, I_ViedoFileAgent *pFileAgent,
		PFN_CmdStatusCallBack StatusCallBack, unsigned int i32DataSize,
		BYTE *pbyData) :
		C_VideoSteamCommmadBase(pCommandAgnet, pFileAgent, StatusCallBack,
				E_CommandType_Set) {
	BYTE *pbyPayload = new BYTE[i32DataSize + 2];

	pbyPayload[0] = i32DataSize & 0xFF;
	pbyPayload[1] = (i32DataSize >> 8) & 0xFF;

	if (i32DataSize > 0)
		memcpy(&pbyPayload[2], pbyData, i32DataSize);

	SetPacketload(GP_SOCK_TYPE_CMD, GPSOCK_MODE_Firmware,
			GPSOCK_Firmware_CMD_SendRawData, pbyPayload, i32DataSize + 2);

	SAFE_DELETE_ARRAY(pbyPayload)
}

C_FirmwareSendRawDataCmd::~C_FirmwareSendRawDataCmd() {

}

//----------------------------------------------------------
//Firmware upgrade
//----------------------------------------------------------
C_FirmwareUpgradeCmd::C_FirmwareUpgradeCmd(I_CommandAgent* pCommandAgnet,
		I_ViedoFileAgent *pFileAgent, PFN_CmdStatusCallBack StatusCallBack) :
		C_VideoSteamCommmadBase(pCommandAgnet, pFileAgent, StatusCallBack,
				E_CommandType_Set) {

	SetPacketload(GP_SOCK_TYPE_CMD, GPSOCK_MODE_Firmware,
			GPSOCK_Firmware_CMD_Upgrade, NULL, 0);
}

C_FirmwareUpgradeCmd::~C_FirmwareUpgradeCmd() {

}

/***************************************************************************************
 CV Firmware mode
 ***************************************************************************************/

//----------------------------------------------------------
//CV Firmware Download
//----------------------------------------------------------
C_CVFirmwareDownloadCmd::C_CVFirmwareDownloadCmd(I_CommandAgent* pCommandAgnet,
		I_ViedoFileAgent *pFileAgent, PFN_CmdStatusCallBack StatusCallBack,
		unsigned int ui32FileSize, unsigned int ui32CheckSum) :
		C_VideoSteamCommmadBase(pCommandAgnet, pFileAgent, StatusCallBack,
				E_CommandType_Set) {
	BYTE byPlayload[8];

	byPlayload[0] = ui32FileSize & 0xFF;
	byPlayload[1] = (ui32FileSize >> 8) & 0xFF;
	byPlayload[2] = (ui32FileSize >> 16) & 0xFF;
	byPlayload[3] = (ui32FileSize >> 24) & 0xFF;

	byPlayload[4] = ui32CheckSum & 0xFF;
	byPlayload[5] = (ui32CheckSum >> 8) & 0xFF;
	byPlayload[6] = (ui32CheckSum >> 16) & 0xFF;
	byPlayload[7] = (ui32CheckSum >> 24) & 0xFF;

	SetPacketload(GP_SOCK_TYPE_CMD, GPSOCK_MODE_Firmware_CV,
			GPSOCK_Firmware_CMD_Download, byPlayload, sizeof(byPlayload));
}

C_CVFirmwareDownloadCmd::~C_CVFirmwareDownloadCmd() {

}

//----------------------------------------------------------
//CV Firmware Send raw data
//----------------------------------------------------------
C_CVFirmwareSendRawDataCmd::C_CVFirmwareSendRawDataCmd(
		I_CommandAgent* pCommandAgnet, I_ViedoFileAgent *pFileAgent,
		PFN_CmdStatusCallBack StatusCallBack, unsigned int i32DataSize,
		BYTE *pbyData) :
		C_VideoSteamCommmadBase(pCommandAgnet, pFileAgent, StatusCallBack,
				E_CommandType_Set) {
	BYTE *pbyPayload = new BYTE[i32DataSize + 2];

	pbyPayload[0] = i32DataSize & 0xFF;
	pbyPayload[1] = (i32DataSize >> 8) & 0xFF;

	if (i32DataSize > 0)
		memcpy(&pbyPayload[2], pbyData, i32DataSize);

	SetPacketload(GP_SOCK_TYPE_CMD, GPSOCK_MODE_Firmware_CV,
			GPSOCK_Firmware_CMD_SendRawData, pbyPayload, i32DataSize + 2);

	SAFE_DELETE_ARRAY(pbyPayload)
}

C_CVFirmwareSendRawDataCmd::~C_CVFirmwareSendRawDataCmd() {

}

//----------------------------------------------------------
//C Firmware upgrade
//----------------------------------------------------------
C_CVFirmwareUpgradeCmd::C_CVFirmwareUpgradeCmd(I_CommandAgent* pCommandAgnet,
		I_ViedoFileAgent *pFileAgent, PFN_CmdStatusCallBack StatusCallBack,
		unsigned int i32Area) :
		C_VideoSteamCommmadBase(pCommandAgnet, pFileAgent, StatusCallBack,
				E_CommandType_Set) {
	BYTE *pbyPayload = new BYTE[1];
	pbyPayload[0] = i32Area & 0xFF;
	SetPacketload(GP_SOCK_TYPE_CMD, GPSOCK_MODE_Firmware_CV,
			GPSOCK_Firmware_CMD_Upgrade, pbyPayload, 1);

	SAFE_DELETE_ARRAY(pbyPayload)
}

C_CVFirmwareUpgradeCmd::~C_CVFirmwareUpgradeCmd() {

}

/***************************************************************************************
 Vendor mode
 ***************************************************************************************/

//----------------------------------------------------------
//send Vendor command
//----------------------------------------------------------
C_SendVendorCmd::C_SendVendorCmd(I_CommandAgent* pCommandAgnet,
		I_ViedoFileAgent *pFileAgent, PFN_CmdStatusCallBack StatusCallBack,
		BYTE *pbyData, int i32Value) :
		C_VideoSteamCommmadBase(pCommandAgnet, pFileAgent, StatusCallBack,
				E_CommandType_Set) {
	SetPacketload(GP_SOCK_TYPE_CMD, GPSOCK_MODE_Vendor,
			GPSOCK_Vendor_CMD_Vendor, pbyData, i32Value);
}

C_SendVendorCmd::~C_SendVendorCmd() {

}
/***************************************************************************************
 Action
 ***************************************************************************************/

C_DelayCmd::C_DelayCmd(I_CommandAgent* pCommandAgnet,
		I_ViedoFileAgent *pFileAgent, PFN_CmdStatusCallBack StatusCallBack,
		int i32Delayms) :
		C_VideoSteamCommmadBase(pCommandAgnet, pFileAgent, StatusCallBack,
				E_CommandType_Action), m_i32Delayms(i32Delayms) {

}

C_DelayCmd::~C_DelayCmd() {

}

void C_DelayCmd::DoAction() {
	DEBUG_PRINT("Delay %dms \n", m_i32Delayms);
	usleep(m_i32Delayms * 1000);
}

//----------------------------------------------------------
//Playback get file name list for Reverse
//----------------------------------------------------------
C_GetPlaybackNameListCmdReverse::C_GetPlaybackNameListCmdReverse(
		I_CommandAgent* pCommandAgnet, I_ViedoFileAgent *pFileAgent,
		PFN_CmdStatusCallBack StatusCallBack, int i32Index, int i32MaxCount,
		int i32IndexOnDevice) :
		C_VideoSteamCommmadBase(pCommandAgnet, pFileAgent, StatusCallBack,
				E_CommandType_Get), m_i32Index(i32Index), m_i32MaxCount(
				i32MaxCount) {
	BYTE byData[3];
	int i32DeviceIdx = 0x00;

	if (i32Index == -1) {
		byData[0] = 0x04;
		if (i32IndexOnDevice == -1)
			i32DeviceIdx = pFileAgent->GetFileIndex(i32Index);
		else
			i32DeviceIdx = i32IndexOnDevice;

		m_i32Index = 0;
	} else if (i32Index == 0)
		byData[0] = 0x01; // last file
	else {
		byData[0] = 0x00;

		if (i32IndexOnDevice == -1)
			i32DeviceIdx = pFileAgent->GetFileIndex(i32Index);
		else
			i32DeviceIdx = i32IndexOnDevice;

		DEBUG_PRINT("Index:%d Device index: %d\n", m_i32Index, i32DeviceIdx);

		m_i32Index--;
	}

	byData[1] = i32DeviceIdx & 0xFF;
	byData[2] = (i32DeviceIdx >> 8) & 0xFF;

	SetPacketload(GP_SOCK_TYPE_CMD, GPSOCK_MODE_Playback,
			GPSOCK_Playback_CMD_GetNameList, byData, sizeof(byData));
}

C_GetPlaybackNameListCmdReverse::~C_GetPlaybackNameListCmdReverse() {

}

//----------------------------------------------------------
E_HandleAck_Retcode C_GetPlaybackNameListCmdReverse::HandleAck(
		I_PacketParser *packet) {
	E_HandleAck_Retcode RetCode = C_VideoSteamCommmadBase::HandleAck(packet);
	if (RetCode != E_HandleAck_Retcode_NoError) {
		return RetCode;
	}

	if (packet->GetPayloadSize() == 0) {
		if (m_CmdStatuCallBack)
			m_CmdStatuCallBack(GetCMDIdex(), packet->GetType(), GetMode(),
					GetCMD(), 0, 0);
	} else {

		BYTE* pbyPlayload = packet->GetPayload();
		BYTE byCount = pbyPlayload[0];
		if (byCount < 1) {
			return RetCode;
		}

		int i32FileAttrSize = Get_FileAttribute_RealSize;
		int i32TotalSize = Get_FileAttribute_RealSize * byCount;
		int i32ExtraSize = (((packet->GetPayloadSize() - 1) - i32TotalSize)
				/ byCount);
		i32FileAttrSize += i32ExtraSize;

		for (int i = 0; i < 1; i++)
			m_pFileAgent->AddOneFileAttr(m_i32Index + i,
					&pbyPlayload[1 + (i32FileAttrSize * i)], i32ExtraSize);

		BYTE byData[3];
		byData[0] = m_i32Index & 0xFF;
		byData[1] = (m_i32Index >> 8) & 0xFF;
		byData[2] = 1;

		if (m_CmdStatuCallBack)
			m_CmdStatuCallBack(GetCMDIdex(), packet->GetType(), GetMode(),
					GetCMD(), sizeof(byData), byData);

		DEBUG_PRINT("\nFileName, Index: %d Size:%d\n", m_i32Index, byCount);

//        int i32NextPreview = m_i32Index - byCount+1;
//        if( byCount > 0 && m_pFileAgent->GetNextPreview(&i32NextPreview))
//        {
//            DEBUG_PRINT("\nNextPreview, Index: %d\n",i32NextPreview);
//
//            if(m_pFileAgent->GetFileIndex(i32NextPreview)==-1)
//            {
//                DEBUG_PRINT("\nSpecificName Index: %d\n",i32NextPreview);
//                C_GetPlaybackSpecificName *pNameList = new C_GetPlaybackSpecificName(m_pCommnadAgent,m_pFileAgent,m_CmdStatuCallBack,i32NextPreview,m_i32MaxCount);
//                m_pCommnadAgent->QueueCmd(pNameList);
//            }
//            else
//            {
//            	C_GetPlaybackNameListCmdReverse *pNameList = new C_GetPlaybackNameListCmdReverse(m_pCommnadAgent,m_pFileAgent,m_CmdStatuCallBack,i32NextPreview,m_i32MaxCount);
//                m_pCommnadAgent->QueueCmd(pNameList);
//            }
//        }
	}

	return RetCode;
}

