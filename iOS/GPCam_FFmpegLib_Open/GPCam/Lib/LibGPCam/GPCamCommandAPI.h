//
//  GPCamCommandAPI.h
//  GPCam library  V1.0.15
//
//  Created by generalplus_sa1 on 8/20/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#ifndef __GPCam__CommandAPI__
#define __GPCam__CommandAPI__

#include <stdio.h>
#include "GPCamTypedef.h"

#if defined(__cplusplus)
extern "C" {
#endif

    /**
     * \brief
     * 	Get the version sting of libGPCam.
     *
     * \return
     *	Return the version sting.
     *
     */
    char* GPCam_Version();

	/**
	* \brief
	* 	Connect to server.
	*
	* \param[in] pszIPAddress
	*	The string of IP address
	* \param[in] i32PortNum
	*	The port number
	* \return
	*	Return 0 if this function succeeded. Otherwise, other value returned.
	*
	*/
    int GPCam_ConnectToDevice(
                        LPCTSTR pszIPAddress,
                        int i32PortNum);

	/**
	* \brief
	* 	Disconnect with server.
	*
	*/
    void GPCam_Disconnect();

	/**
	* \brief
	*	Registers a user callback function for reporting command processing status.
	*
	* \details
	*	PFN_CmdStatusCallBack will bring the Command result. The parameters of PFN_CmdStatusCallBack as follow:\n
	*	<DL>
	*   <dt><EM>i32CMDIndex</dt></EM>  	<dd>Command queue index</dd>
	*   <dt><EM>i32Type</dt></EM>  		<dd>GP_SOCK_TYPE_ACK or GP_SOCK_TYPE_NAK. GP_SOCK_TYPE_ACK means command is complete , GP_SOCK_TYPE_ACK is command failed.</dd>
	*   <dt><EM>i32Mode</dt></EM> 		<dd>Command Mode ID</dd>
	*   <dt><EM>i32CMDID</dt></EM>  	<dd>Command ID</dd>
	*   <dt><EM>i32DataSize</dt></EM>  	<dd>The Data Size. If i32Type is GP_SOCK_TYPE_NAK , the size always is 2</dd>
	*   <dt><EM>pbyData</dt></EM>  		<dd>The Data. If i32Type is GP_SOCK_TYPE_NAK , the data is Error code.\n</dd>
	*	</DL>
	*
	* \param[in] CallBack
	* 	The user callback function.
	* \see
	*	PFN_CmdStatusCallBack
	*
	*/
    void GPCam_SetCmdStatusCallBack(PFN_CmdStatusCallBack CallBack);

	/**
	* \brief
	* 	Set the download path for saving thumbnail, media file and temporary file.
	*
	* \param[in] ptszPath
	*	The download path
	*/
    void GPCam_SetDownloadPath(const char* ptszPath);


	/**
	* \brief
	* 	Clear command queue.
	*
	*/
    void GPCam_ClearCmdQueue();

	/**
	* \brief
	* 	Try to abort the specific command in command queue.
	*
	* \param[in] i32Index
	*	The command Index in queue.
	* \return
	*	Return 0 if this function succeeded. Otherwise, other value returned.
	*
	*/
    int GPCam_Abort(int i32Index);

    //General
	/**
	*
	* \brief
	* 	Send command to switch the device mode.
	*
	* \details
	*	PFN_CmdStatusCallBack will bring the Command result. The parameters of PFN_CmdStatusCallBack as follow:\n
	*	<DL>
	*   <dt><EM>i32Mode</dt></EM>  			<dd>GPSOCK_MODE_General</dd>
	*   <dt><EM>i32CMDID</dt></EM> 			<dd>GPSOCK_General_CMD_SetMode</dd>
	*   <dt><EM>i32DataSize</dt></EM>  		<dd>The Data Size is 0</dd>
	*   <dt><EM>pbyData</dt></EM>  			<dd>No Data</dd>
	*	</DL>
	* \param[in] i32Mode
	*	The mode.
	* \return
	*	Return the command queue index.
	* \see
	*	PFN_CmdStatusCallBack
	* \see
	*	E_DeviceMode

	*/
    int GPCam_SendSetMode(int i32Mode);

	/**
	*
	* \brief
	* 	Send command to get the device status.
	*
	* \details
	*	PFN_CmdStatusCallBack will bring the Command result. The parameters of PFN_CmdStatusCallBack as follow:\n
	*   <DL>
	*   <dt><EM>i32Mode</EM><dt>  		<dd>GPSOCK_MODE_General</dd>
	*   <dt><EM>i32CMDID</EM><dt>  		<dd>GPSOCK_General_CMD_GetDeviceStatus</dd>
	*   <dt><EM>i32DataSize</EM></dt>  	<dd>The Data Size is 14</dd>
	*   <dt><EM>pbyData</EM></dt>
	*			        				<dd>Byte[0]:		Current Mode ,0x00 = Record, 0x01 = Capture picture, 0x02 = Playback, 0x03 = Menu, 0x04 = USB\n
	*										Byte[1]:   		Bit0 = Inactive/Active recording/continuous shooting, Bit1 = Audio off/on, Bit6 = TCP Low window, Bit7 = VLC reset (Restart player)\n
	*										Byte[2]:		Bit0~6 = Battery Level 0 ~ 4 , Bit7 = Is RTSP streaming\n
	*										Byte[3]:		Adapter status, 0x00 = OFF , 0x01 = ON\n
	*										Byte[4]:		Record resolution ID in setting file\n
	*										Byte[8]~[5]:	Remaining time of record (sec)\n
	*										Byte[9]:		Capture resolution ID in setting file\n
	*										Byte[13]~[10]:	Remaining number of pictures\n
  *                   Byte[14]:   Frame rate\n
	*									</dd>
	*   </DL>
	*
	* \return
	*	Return the command queue index.
	* \see
	*	PFN_CmdStatusCallBack

	*/
    int GPCam_SendGetSetPIP(int i32Type);

    /**
    *
    * \brief
    *     Send command to get the device status.
    *
    * \details
    *    PFN_CmdStatusCallBack will bring the Command result. The parameters of PFN_CmdStatusCallBack as follow:\n
    *   <DL>
    *   <dt><EM>i32Mode</EM><dt>          <dd>GPSOCK_MODE_General</dd>
    *   <dt><EM>i32CMDID</EM><dt>          <dd>GPSOCK_General_CMD_GetSetPIP</dd>
    *   <dt><EM>i32DataSize</EM></dt>      <dd>The Data Size is 1</dd>
    *   <dt><EM>pbyData</EM></dt>
    *                                    <dd>Byte[0]:        Status ,0x00 = fail, 0x01 = OK\n
    *                                    </dd>
    *   </DL>
    *
    * \return
    *    Return the command queue index.
    * \see
    *    PFN_CmdStatusCallBack

    */
    int GPCam_SendGetStatus();

	/**
	*
	* \brief
	* 	Send command to download [the device setting file](@ref the_DeviceSettingFile_section). The setting file describe the available settings on device, and these setting can be set/get by sending GPCam_SendSetParameter() / GPCam_SendGetParameter().
	*
	* \details
	*	PFN_CmdStatusCallBack will bring the Command result. The parameters of PFN_CmdStatusCallBack as follow:\n
	*   <DL>
	*   <dt><EM>i32Mode</dt></EM>  		<dd>GPSOCK_MODE_General</dd>
	*   <dt><EM>i32CMDID</dt></EM>  	<dd>GPSOCK_General_CMD_GetParameterFile</dd>
	*   <dt><EM>i32DataSize</dt></EM>  	<dd>The Data Size is 0</dd>
	*   <dt><EM>pbyData</dt></EM>  		<dd>No Data</dd>
	*   </DL>
	*
	* \param[in] ptszFilaName
	*	The setting file name.
	* \return
	*	Return the command queue index.
	* \see
	*	PFN_CmdStatusCallBack
	* \see
	*	[Setting File Format](@ref the_DeviceSettingFile_section)
	*/
    int GPCam_SendGetParameterFile(const char* ptszFilaName);

	/**
	*
	* \brief
	* 	Send command to power off the device.
	*
	* \details
	*	PFN_CmdStatusCallBack will bring the Command result. The parameters of PFN_CmdStatusCallBack as follow:\n
	*   <DL>
	*   <dt><EM>i32Mode</dt></EM>  		<dd>GPSOCK_MODE_General</dd>
	*   <dt><EM>i32CMDID</dt></EM> 		<dd>GPSOCK_General_CMD_Poweroff</dd>
	*   <dt><EM>i32DataSize</dt></EM> 	<dd>The Data Size is 0</dd>
	*   <dt><EM>pbyData</dt></EM>  		<dd>No Data</dd>
	*   </DL>
	*
	* \return
	*	Return the command queue index.
	* \see
	*	PFN_CmdStatusCallBack
	*/
    int GPCam_SendPowerOff();

	/**
	*
	* \brief
	* 	Send command to restart streaming service on port 8080.
	*
	* \details
	*	PFN_CmdStatusCallBack will bring the Command result. The parameters of PFN_CmdStatusCallBack as follow:\n
	*   <DL>
	*   <dt><EM>i32Mode</dt></EM>  		<dd>GPSOCK_MODE_General</dd>
	*   <dt><EM>i32CMDID</dt></EM>  	<dd>GPSOCK_General_CMD_RestartStreaming</dd>
	*   <dt><EM>i32DataSize</dt></EM>  	<dd>The Data Size is 0</dd>
	*   <dt><EM>pbyData</dt></EM>  		<dd>No Data</dd>
	*   </DL>
	*
	* \return
	*	Return the command queue index.
	* \see
	*	PFN_CmdStatusCallBack
	* \see
	*	[Record/Capture Streaming](@ref the_RecordCaptureStreaming_section)
	* \see
	*	[Playback Streaming](@ref the_PlaybackStreaming_section)
	*/
    int GPCam_SendRestartStreaming();

    //Record
	/**
	*
	* \brief
	* 	Send command to start/stop record. The mode must switch to \ref E_DeviceMode_Record "E_DeviceMode_Record" first.
	*
	* \details
	*	PFN_CmdStatusCallBack will bring the Command result. The parameters of PFN_CmdStatusCallBack as follow:\n
	*   <DL>
	*   <dt><EM>i32Mode</dt></EM>  		<dd>GPSOCK_MODE_Record</dd>
	*   <dt><EM>i32CMDID</dt></EM>  	<dd>GPSOCK_Record_CMD_Start</dd>
	*   <dt><EM>i32DataSize</dt></EM>  	<dd>The Data Size is 0</dd>
	*   <dt><EM>pbyData</dt></EM> 		<dd>No Data</dd>
	*   </DL>
	*
	* \return
	*	Return the command queue index.
	* \see
	*	GPCam_SendSetMode
	* \see
	*	PFN_CmdStatusCallBack
	*/
    int GPCam_SendRecordCmd();

	/**
	*
	* \brief
	* 	Send command to download the device setting file.  The mode must switch to \ref E_DeviceMode_Record "E_DeviceMode_Record" first.
	*
	* \details
	*	PFN_CmdStatusCallBack will bring the Command result. The parameters of PFN_CmdStatusCallBack as follow:\n
	*   <DL>
	*   <dt><EM>i32Mode</dt></EM>  		<dd>GPSOCK_MODE_Record</dd>
	*   <dt><EM>i32CMDID</dt></EM>  	<dd>GPSOCK_Record_CMD_Audio</dd>
	*   <dt><EM>i32DataSize</dt></EM>  	<dd>The Data Size is 0</dd>
	*   <dt><EM>pbyData</dt></EM>  		<dd>No Data</dd>
	*   </DL>
	*
	* \param[in] bOn
	*	true = The record sound. false = Mute.
	* \return
	*	Return the command queue index.
	* \see
	*	PFN_CmdStatusCallBack
	*/
    int GPCam_SendAudioOnOff(bool bOn);

    //Capture picture
	/**
	*
	* \brief
	* 	Send command to capture picture. The mode must switch to \ref E_DeviceMode_Capture "E_DeviceMode_Capture" first.
	*
	* \details
	*	PFN_CmdStatusCallBack will bring the Command result. The parameters of PFN_CmdStatusCallBack as follow:\n
	*   <DL>
	*   <dt><EM>i32Mode</dt></EM>  		<dd>GPSOCK_MODE_CapturePicture</dd>
	*   <dt><EM>i32CMDID</dt></EM>  	<dd>GPSOCK_CapturePicture_CMD_Capture</dd>
	*   <dt><EM>i32DataSize</dt></EM>  	<dd>The Data Size is 0</dd>
	*   <dt><EM>pbyData</dt></EM>  		<dd>No Data</dd>
	*   </DL>
	*
	* \return
	*	Return the command queue index.
	* \see
	*	GPCam_SendSetMode
	* \see
	*	PFN_CmdStatusCallBack
	*/
    int GPCam_SendCapturePicture();

    //Playback
	/**
	*
	* \brief
	* 	Send command to playback specific file. The mode must switch to \ref E_DeviceMode_Playback "E_DeviceMode_Playback" , call GPCam_SendGetFullFileList() and call GPCam_SendRestartStreaming() first.
	*
	* \details
	*	PFN_CmdStatusCallBack will bring the Command result. The parameters of PFN_CmdStatusCallBack as follow:\n
	*	<DL>
	*   <dt><EM>i32Mode</dt></EM>  		<dd>GPSOCK_MODE_Playback</dd>
	*   <dt><EM>i32CMDID</dt></EM>  	<dd>GPSOCK_Playback_CMD_Start</dd>
	*   <dt><EM>i32DataSize</dt></EM>  	<dd>The Data Size is 0</dd>
	*   <dt><EM>pbyData</dt></EM> 		<dd>No Data</dd>
	*	</DL>
	*
	* \param[in] i32Index
	*	The file index.
	* \return
	*	Return the command queue index.
	* \see
	*	GPCam_SendSetMode
	* \see
	*	PFN_CmdStatusCallBack
	*/
    int GPCam_SendStartPlayback(int i32Index);

	/**
	*
	* \brief
	* 	Send command to pause playback. The mode must switch to \ref E_DeviceMode_Playback "E_DeviceMode_Playback" and call GPCam_SendGetFullFileList() first.
	*
	* \details
	*	PFN_CmdStatusCallBack will bring the Command result. The parameters of PFN_CmdStatusCallBack as follow:\n
	*	<DL>
	*   <dt><EM>i32Mode</dt></EM>  		<dd>GPSOCK_MODE_Playback</dd>
	*   <dt><EM>i32CMDID</dt></EM>  	<dd>GPSOCK_Playback_CMD_Pause</dd>
	*   <dt><EM>i32DataSize</dt></EM>  	<dd>The Data Size is 0</dd>
	*   <dt><EM>pbyData</dt></EM>  		<dd>No Data</dd>
	*	</DL>
	*
	* \return
	*	Return the command queue index.
	* \see
	*	GPCam_SendSetMode
	* \see
	*	PFN_CmdStatusCallBack
	*/
    int GPCam_SendPausePlayback();

	/**
	*
	* \brief
	* 	Send command to get the file list and informations in device file. The mode must switch to \ref E_DeviceMode_Playback "E_DeviceMode_Playback" first.\n
	*   <b>This command must be called before any file command.</b>
	*
	* \details
	*	PFN_CmdStatusCallBack will bring the Command result. The parameters of PFN_CmdStatusCallBack as follow:\n
	*	<DL>
	*   <dt><EM>i32Mode</dt></EM>  		<dd>GPSOCK_MODE_Playback</dd>
	*   <dt><EM>i32CMDID</dt></EM>  	<dd>GPSOCK_Playback_CMD_GetFileCount or GPSOCK_Playback_CMD_GetNameList</dd>
	*   <dt><EM>i32DataSize</dt></EM>  	<dd>if i32CMDID = GPSOCK_Playback_CMD_GetFileCount, the size = 2, if i32CMDID = GPSOCK_Playback_CMD_GetNameList, the size = 3</dd>
	*   <dt><EM>pbyData</dt></EM>
	*									<dd>i32CMDID = GPSOCK_Playback_CMD_GetFileCount , the data is total file count in device.\n
	*										i32CMDID = GPSOCK_Playback_CMD_GetNameList , the data is fileindex and count. This means the file information is ready in the range of fileindex ~ fileindex + count.\n
	*									</dd>
	*	</DL>
	*
	* \return
	*	Return the command queue index.
	* \see
	*	GPCam_SendSetMode
	* \see
	*	PFN_CmdStatusCallBack
	* \see
	*	[File list](@ref the_Filelist_section)
	*/
    int GPCam_SendGetFullFileList();

	/**
	*
	* \brief
	* 	Send command to get file thumbnail. The mode must switch to \ref E_DeviceMode_Playback "E_DeviceMode_Playback" and call GPCam_SendGetFullFileList() first.
	*
	* \details
	*	PFN_CmdStatusCallBack will bring the Command result. The parameters of PFN_CmdStatusCallBack as follow:\n
	*	<DL>
	*   <dt><EM>i32Type</dt></EM>  		<dd>GP_SOCK_TYPE_ACK means command is complete ,\n <b>GP_SOCK_TYPE_ACK with error code @ref Error_InvalidCommand or @ref Error_GetThumbnailFail means thumbnail broken, GPCam_SendStartPlayback() and GPCam_SendGetFileRawdata() is not allow. @ref Error_ServerIsBusy means server is busy , resend this command later.</b></dd>
	*   <dt><EM>i32Mode</dt></EM>  		<dd>GPSOCK_MODE_Playback</dd>
	*   <dt><EM>i32CMDID</dt></EM>  	<dd>GPSOCK_Playback_CMD_GetThumbnail</dd>
	*   <dt><EM>i32DataSize</dt></EM>  	<dd>The Data Size is variable</dd>
	*   <dt><EM>pbyData</dt></EM>
	*									<dd>Byte[0]~[1] = File Index\n
	*										Byte[2]~[3] = String length of thumbnail path\n
	*										Byte[4]~[n] = String of thumbnail path\n
	*									</dd>
	*	</DL>
	*
	* \param[in] i32Index
	*	The file index.
	* \return
	*	Return the command queue index.
	* \see
	*	GPCam_SendSetMode
	* \see
	*	PFN_CmdStatusCallBack
	* \see
	*	[Download Thumbnail](@ref the_DownloadThumbnail_section)
	*/
    int GPCam_SendGetFileThumbnail(int i32Index);

	/**
	*
	* \brief
	* 	Send command to get file raw data. The mode must switch to \ref E_DeviceMode_Playback "E_DeviceMode_Playback" and call GPCam_SendGetFullFileList() first.
	*
	* \details
	*	PFN_CmdStatusCallBack will bring the Command result. The parameters of PFN_CmdStatusCallBack as follow:\n
	*	<DL>
	*   <dt><EM>i32Mode</dt></EM>  		<dd>GPSOCK_MODE_Playback</dd>
	*   <dt><EM>i32CMDID</dt></EM>  	<dd>GPSOCK_Playback_CMD_GetRawData</dd>
	*   <dt><EM>i32DataSize</dt></EM>  	<dd>The Data Size is variable</dd>
	*   <dt><EM>pbyData</dt></EM>
	*									<dd>Byte[0] = Result type. 0x00 is downloading. 0x01 is download complete.\n
	*       								If Byte[0] = 0x00, Byte[1] is the percent of downloading process.\n
	*       								If Byte[0] = 0x01, Byte[1]~[2] = File Index, Byte[3]~[4] = String length of download file path, Byte[5]~[n] = String of download file path\n
	*									</dd>
	*	</DL>
	*
	* \param[in] i32Index
	*	The file index.
	* \return
	*	Return the command queue index.
	* \see
	*	GPCam_SendSetMode
	* \see
	*	PFN_CmdStatusCallBack
	* \see
	*	[Download Raw Data](@ref the_Download_section)
	* \see
	*	[Stop download Raw Data](@ref the_Stopdownload_section)
	*/
    int GPCam_SendGetFileRawdata(int i32Index);

    
    /**
     *
     * \brief
     * 	Send command to delete file. The mode must switch to \ref E_DeviceMode_Playback "E_DeviceMode_Playback" and call GPCam_SendGetFullFileList() first.
     *
     * \details
     *	PFN_CmdStatusCallBack will bring the Command result. The parameters of PFN_CmdStatusCallBack as follow:\n
     *	<DL>
     *   <dt><EM>i32Mode</dt></EM>  		<dd>GPSOCK_MODE_Playback</dd>
     *   <dt><EM>i32CMDID</dt></EM>         <dd>GPSOCK_Playback_CMD_DeleteFile</dd>
     *   <dt><EM>i32DataSize</dt></EM>  	<dd>The Data Size is 0</dd>
     *   <dt><EM>pbyData</dt></EM>  		<dd>No Data</dd>
     *	</DL>
     *
     * \param[in] i32Index
     *	The file index.
     * \return
     *	Return the command queue index.
     * \see
     *	GPCam_SendSetMode
     * \see
     *	PFN_CmdStatusCallBack
     */
    int GPCam_SendDeleteFile(int i32Index);
    
	/**
	*
	* \brief
	* 	Send command to stop playback. The mode must switch to \ref E_DeviceMode_Playback "E_DeviceMode_Playback" and call GPCam_SendGetFullFileList() first.
	*
	* \details
	*	PFN_CmdStatusCallBack will bring the Command result. The parameters of PFN_CmdStatusCallBack as follow:\n
	*	<DL>
	*   <dt><EM>i32Mode</dt></EM>  		<dd>GPSOCK_MODE_Playback</dd>
	*   <dt><EM>i32CMDID</dt></EM>  	<dd>GPSOCK_Playback_CMD_Stop</dd>
	*   <dt><EM>i32DataSize</dt></EM> 	<dd>The Data Size is 0</dd>
	*   <dt><EM>pbyData</dt></EM>  		<dd>No Data</dd>
	*	</DL>
	*
	* \return
	*	Return the command queue index.
	* \see
	*	GPCam_SendSetMode
	* \see
	*	PFN_CmdStatusCallBack
	*/
	int GPCam_SendStopPlayback();

    /**
     *
     * \brief
     * 	Set the file list which needs to get informations first. The mode must switch to \ref E_DeviceMode_Playback "E_DeviceMode_Playback" and call GPCam_SendGetFullFileList() first.
     *
     * \param[in] i32Index
     *	The file index.
     * \return
     *	Return the command queue index.
     * \see
     *	GPCam_SendSetMode
     * \see
     *  GPCam_SendGetFullFileList
     * \see
     *	[File list](@ref the_Filelist_section)
     * \see
     *	[Download Thumbnail](@ref the_DownloadThumbnail_section)
     */
    int GPCam_SetNextPlaybackFileListIndex(int i32Index);

    //Menu
	/**
	*
	* \brief
	* 	Send command to get the setting value. The mode must switch to \ref E_DeviceMode_Menu "E_DeviceMode_Menu" first.
	*
	* \details
	*	PFN_CmdStatusCallBack will bring the Command result. The parameters of PFN_CmdStatusCallBack as follow:\n
	*	<DL>
	*   <dt><EM>i32Mode</dt></EM>  		<dd>GPSOCK_MODE_Menu</dd>
	*   <dt><EM>i32CMDID</dt></EM> 		<dd>GPSOCK_Menu_CMD_GetParameter</dd>
	*   <dt><EM>i32DataSize</dt></EM>  	<dd>The Data Size is variable</dd>
	*   <dt><EM>pbyData</dt></EM>
	* 									<dd>Byte[0]~[3] is setting ID.\n
	*										Byte[4]~[N] is the value of setting.\n
	*									</dd>
	*	</DL>
	*
	*
	* \param[in] i32ID
	*	The setting ID in setting file.
	* \return
	*	Return the command queue index.
	* \see
	*	GPCam_SendSetMode
	* \see
	*	PFN_CmdStatusCallBack
	* \see
	*	[Setting File Format](@ref the_DeviceSettingFile_section)
	*/

    int GPCam_SendGetParameter(int i32ID);

	/**
	*
	* \brief
	* 	Send command to set the setting value. The mode must switch to \ref E_DeviceMode_Menu "E_DeviceMode_Menu" first.
	*
	* \details
	*	PFN_CmdStatusCallBack will bring the Command result. The parameters of PFN_CmdStatusCallBack as follow:\n
	*	<DL>
	*   <dt><EM>i32Mode</dt></EM>  		<dd>GPSOCK_MODE_Menu</dd>
	*   <dt><EM>i32CMDID</dt></EM>  	<dd>GPSOCK_Menu_CMD_SetParameter</dd>
	*   <dt><EM>i32DataSize</dt></EM> 	<dd>The Data Size is 0</dd>
	*   <dt><EM>pbyData</dt></EM>  		<dd>No Data</dd>
	*	</DL>
	*
	*
	* \param[in] i32ID
	*	The setting ID in setting file.
	* \param[in] i32Size
	*	The length of the setting value.
	* \param[in] pbyData
	*	The setting value.
	* \return
	*	Return the command queue index.
	* \see
	*	GPCam_SendSetMode
	* \see
	*	PFN_CmdStatusCallBack
	* \see
	*	[Setting File Format](@ref the_DeviceSettingFile_section)
	*/
    int GPCam_SendSetParameter(int i32ID, int i32Size, BYTE* pbyData);

    //Firmware
    /**
     *
     * \brief
     * 	Send command to start download firmware to device. The mode must switch to \ref E_DeviceMode_Menu "E_DeviceMode_Menu" first.
     *
     * \details
     *	PFN_CmdStatusCallBack will bring the Command result. The parameters of PFN_CmdStatusCallBack as follow:\n
     *	<DL>
     *   <dt><EM>i32Mode</dt></EM>  	<dd>GPSOCK_MODE_Firmware</dd>
     *   <dt><EM>i32CMDID</dt></EM>  	<dd>GPSOCK_Firmware_CMD_Download</dd>
     *   <dt><EM>i32DataSize</dt></EM> 	<dd>The Data Size is 0</dd>
     *   <dt><EM>pbyData</dt></EM>  	<dd>No Data</dd>
     *	</DL>
     *
     *
     * \param[in] ui32FileSize
     *	The size fo firmware file.
     * \param[in] ui32CheckSum
     *	The checksum fo firmware file.
     * \return
     *	Return the command queue index.
     * \see
     *	GPCam_SendSetMode
     * \see
     *	PFN_CmdStatusCallBack
     * \see
     *	[Upgrade Device Firmware](@ref the_UpgradeDeviceFirmware_section)
     */
     int GPCam_SendFirmwareDownload(unsigned int ui32FileSize, unsigned int ui32CheckSum);

    /**
     *
     * \brief
     * 	Send command to send firmware data to device. The mode must switch to \ref E_DeviceMode_Menu "E_DeviceMode_Menu" and call GPCam_SendFirmwareDownload() first.
     *
     * \details
     *	PFN_CmdStatusCallBack will bring the Command result. The parameters of PFN_CmdStatusCallBack as follow:\n
     *	<DL>
     *   <dt><EM>i32Mode</dt></EM>  	<dd>GPSOCK_MODE_Firmware</dd>
     *   <dt><EM>i32CMDID</dt></EM>  	<dd>GPSOCK_Firmware_CMD_SendRawData</dd>
     *   <dt><EM>i32DataSize</dt></EM> 	<dd>The Data Size is 0</dd>
     *   <dt><EM>pbyData</dt></EM>  	<dd>No Data</dd>
     *	</DL>
     *
     *
     * \param[in] ui32Size
     *	The size fo firmware data.
     * \param[in] pbyData
     *	The firmware data.
     * \return
     *	Return the command queue index.
     * \see
     *	GPCam_SendSetMode
     * \see
     *	PFN_CmdStatusCallBack
     * \see
     *  GPCam_SendFirmwareDownload
     * \see
     *	[Upgrade Device Firmware](@ref the_UpgradeDeviceFirmware_section)
     */
     int GPCam_SendFirmwareRawData(unsigned int ui32Size, BYTE* pbyData);

    /**
     *
     * \brief
     * 	Send command to upgrade device firmware. The mode must switch to \ref E_DeviceMode_Menu "E_DeviceMode_Menu", call GPCam_SendFirmwareDownload() and GPCam_SendFirmwareRawData() first.
     *
     * \details
     *	PFN_CmdStatusCallBack will bring the Command result. The parameters of PFN_CmdStatusCallBack as follow:\n
     *	<DL>
     *   <dt><EM>i32Mode</dt></EM>  	<dd>GPSOCK_MODE_Firmware</dd>
     *   <dt><EM>i32CMDID</dt></EM>  	<dd>GPSOCK_Firmware_CMD_Upgrade</dd>
     *   <dt><EM>i32DataSize</dt></EM> 	<dd>The Data Size is 0</dd>
     *   <dt><EM>pbyData</dt></EM>  	<dd>No Data</dd>
     *	</DL>
     *
     *
     * \return
     *	Return the command queue index.
     * \see
     *	GPCam_SendSetMode
     * \see
     *	PFN_CmdStatusCallBack
     * \see
     *  GPCam_SendFirmwareDownload
     * \see
     *  GPCam_SendFirmwareRawData
     * \see
     *	[Upgrade Device Firmware](@ref the_UpgradeDeviceFirmware_section)
     */
     int GPCam_SendFirmwareUpgrade();

     //CV Firmware
	 /**
	  *
	  * \brief
	  * 	Send command to start download CV firmware to device. The mode must switch to \ref E_DeviceMode_Menu "E_DeviceMode_Menu" first.
	  *
	  * \details
	  *	PFN_CmdStatusCallBack will bring the Command result. The parameters of PFN_CmdStatusCallBack as follow:\n
	  *	<DL>
	  *   <dt><EM>i32Mode</dt></EM>  	<dd>GPSOCK_MODE_Firmware</dd>
	  *   <dt><EM>i32CMDID</dt></EM>  	<dd>GPSOCK_Firmware_CMD_Download_CV</dd>
	  *   <dt><EM>i32DataSize</dt></EM> 	<dd>The Data Size is 0</dd>
	  *   <dt><EM>pbyData</dt></EM>  	<dd>No Data</dd>
	  *	</DL>
	  *
	  *
	  * \param[in] ui32FileSize
	  *	The size fo firmware file.
	  * \param[in] ui32CheckSum
	  *	The checksum fo firmware file.
	  * \return
	  *	Return the command queue index.
	  * \see
	  *	GPCam_SendSetMode
	  * \see
	  *	PFN_CmdStatusCallBack
	  * \see
	  *	[Upgrade Device Firmware](@ref the_UpgradeDeviceFirmware_section)
	  */
	  int GPCam_SendCVFirmwareDownload(unsigned int ui32FileSize, unsigned int ui32CheckSum);

	 /**
	  *
	  * \brief
	  * 	Send command to send CV firmware data to device. The mode must switch to \ref E_DeviceMode_Menu "E_DeviceMode_Menu" and call GPCam_SendFirmwareDownload() first.
	  *
	  * \details
	  *	PFN_CmdStatusCallBack will bring the Command result. The parameters of PFN_CmdStatusCallBack as follow:\n
	  *	<DL>
	  *   <dt><EM>i32Mode</dt></EM>  	<dd>GPSOCK_MODE_Firmware</dd>
	  *   <dt><EM>i32CMDID</dt></EM>  	<dd>GPSOCK_Firmware_CMD_SendRawData_CV</dd>
	  *   <dt><EM>i32DataSize</dt></EM> 	<dd>The Data Size is 0</dd>
	  *   <dt><EM>pbyData</dt></EM>  	<dd>No Data</dd>
	  *	</DL>
	  *
	  *
	  * \param[in] ui32Size
	  *	The size fo firmware data.
	  * \param[in] pbyData
	  *	The firmware data.
	  * \return
	  *	Return the command queue index.
	  * \see
	  *	GPCam_SendSetMode
	  * \see
	  *	PFN_CmdStatusCallBack
	  * \see
	  *  GPCam_SendFirmwareDownload
	  * \see
	  *	[Upgrade Device Firmware](@ref the_UpgradeDeviceFirmware_section)
	  */
	  int GPCam_SendCVFirmwareRawData(unsigned int ui32Size, BYTE* pbyData);

	 /**
	  *
	  * \brief
	  * 	Send command to upgrade device CV firmware. The mode must switch to \ref E_DeviceMode_Menu "E_DeviceMode_Menu", call GPCam_SendFirmwareDownload() and GPCam_SendFirmwareRawData() first.
	  *
	  * \details
	  *	PFN_CmdStatusCallBack will bring the Command result. The parameters of PFN_CmdStatusCallBack as follow:\n
	  *	<DL>
	  *   <dt><EM>i32Mode</dt></EM>  	<dd>GPSOCK_MODE_Firmware</dd>
	  *   <dt><EM>i32CMDID</dt></EM>  	<dd>GPSOCK_Firmware_CMD_Upgrade_CV</dd>
	  *   <dt><EM>i32DataSize</dt></EM> 	<dd>The Data Size is 0</dd>
	  *   <dt><EM>pbyData</dt></EM>  	<dd>No Data</dd>
	  *	</DL>
	  *
	  *
	  * \param[in] ui32Area
	  *	The area of upgrade CV firmware.
	  * \return
	  *	Return the command queue index.
	  * \see
	  *	GPCam_SendSetMode
	  * \see
	  *	PFN_CmdStatusCallBack
	  * \see
	  *  GPCam_SendFirmwareDownload
	  * \see
	  *  GPCam_SendFirmwareRawData
	  * \see
	  *	[Upgrade Device Firmware](@ref the_UpgradeDeviceFirmware_section)
	  */
	  int GPCam_SendCVFirmwareUpgrade(unsigned int ui32Area);

    //Vendor
	/**
	* \brief
	* 	Send to vendor command.
	*
	* \details
	*	PFN_CmdStatusCallBack will bring the Command result. The parameters of PFN_CmdStatusCallBack as follow:\n
	*	<DL>
	*   <dt><EM>i32Mode</dt></EM>  		<dd>GPSOCK_MODE_Vendor</dd>
	*   <dt><EM>i32CMDID</dt></EM>  	<dd>GPSOCK_Vendor_CMD_Vendor</dd>
	*   <dt><EM>i32DataSize</dt></EM>  	<dd>The Data Size is variable</dd>
	*   <dt><EM>pbyData</dt></EM>  		<dd>The Data</dd>
	*	</DL>
	*
	*
	* \param[in] pbydata
	*	The data.\n
  * Ex: The setting time command\n
  * Byte[0~1]: 0x11, PayloadSize = 17\n
  * Byte[2]: 0x47, 'G'\n
  * Byte[3]: 0x50, 'P'\n
  * Byte[4]: 0x56, 'V'\n
  * Byte[5]: 0x45, 'E'\n
  * Byte[6]: 0x4E, 'N'\n
  * Byte[7]: 0x44, 'D'\n
  * Byte[8]: 0x4F, 'O'\n
  * Byte[9]: 0x52, 'R'\n
  * Byte[10~11]: 0x0000, CmdID = 0\n
  * Byte[12~13]: year\n
  * Byte[14]: month\n
  * Byte[15]: day\n
  * Byte[16]: hour\n
  * Byte[17]: minute\n
  * Byte[18]: second\n
	* \param[in] i32Size
	*	The length of data.
	* \return
	*	Return the command queue index.
	* \see
	*	PFN_CmdStatusCallBack
	*/
    int GPCam_SendVendorCmd(BYTE* pbydata ,int i32Size);

    //Action
    /**
    * \brief
    * 	Insert delay between commands.
    *
    * \param[in] i32ms
    *	The millisecond that needs to delay.
    * \return
    *	Return the command queue index.
    */
    int GPCam_InsertCommandDelay(int i32ms);

	/**
	* \brief
	* 	Get the connection status.
	*
	* \return
	*	Return E_ConnectionStatus.
	* \see
	* 	E_ConnectionStatus
	*/
    E_ConnectionStatus  GPCam_GetStatus();

    //File
	/**
	*
	* \brief
	* 	Get the file name at index. Must call GPCam_SendGetFullFileList() first.
	*
	*
	* \param[in] i32Index
	*	The file index.
	* \return
	*	Return the file name.
	*/
    char* GPCam_GetFileName(int i32Index);

	/**
	*
	* \brief
	* 	Get the file time at index. Must call GPCam_SendGetFullFileList() first.
	*
	*
	* \param[in] i32Index
	*	The file index.
	* \param[out] pTime
	*	The file time array. 6 byte: year(from 2000) , mouth , day , hour , minute , second
	* \return
	*	Return true if this function succeeded. Otherwise, false returned.
	*/
    bool  GPCam_GetFileTime(int i32Index,BYTE *pTime);

	/**
	*
	* \brief
	* 	Get the file index on device. Must call GPCam_SendGetFullFileList() first.
	*
	*
	* \param[in] i32Index
	*	The file index.
	* \return
	*	Return the file index on device.
	*/
	int   GPCam_GetFileIndex(int i32Index);

	/**
	*
	* \brief
	* 	Get the file size. Must call GPCam_SendGetFullFileList() first.
	*
	*
	* \param[in] i32Index
	*	The file index.
	* \return
	*	Return the file size.(KB)
	*/
    unsigned int   GPCam_GetFileSize(int i32Index);

	/**
	*
	* \brief
	* 	Get the file ext. Must call GPCam_SendGetFullFileList() first.
	*
	*
	* \param[in] i32Index
	*	The file index.
	* \return
	*	Return the 1 byte. 'A' = .avi , 'J' = jpeg
	*/
	BYTE  GPCam_GetFileExt(int i32Index);

    
    /**
     *
     * \brief
     * 	Get the file extra info. Must call GPCam_SendGetFullFileList() first.
     *
     *
     * \param[in] i32Index
     *	The file index.
     * \param[out] pi32Size
     *	The file extra info size.
     * \return
     *	Return the file extra info array
     */
    BYTE*  GPCam_GetFileExtraInfo(int i32Index,int *pi32Size);

  /**
   *
   * \brief
   * 	Set the file ext mapping.
   *
   *
   * \param[in] pMappingString
   *	The file mapping string. Format: Ext1=PrefixName1,FileType1;Ext2=PrefixName2,FileType2;
   * \return
   *	Return true if this function succeeded. Otherwise, false returned.
   * \see
   *  DEFAULT_MAPPING_STR
   */
  bool  GPCam_SetFileNameMapping(const char *pMappingString);

	/**
	* \brief
	*	Registers a user callback function for log raw data of command.
	*
	* \param[in] CallBack
	* 	The user callback function.
	*
	*/
    //Debug
    void  GPCam_SetDataCallBack(PFN_SocketDataCallBack CallBack);
    
    /**
     * \brief
     *    Check rtsp file mapping.
     *
     *
     */
    void  GPCam_CheckFileMapping();

	/**
	*
	* \brief
	* 	Get the file by index on device.
	*
	*
	* \param[in] i32Index
	*	The file index.
	*/
	void GPCam_GetFileByIndex(int i32Index); //for KidsCam


    /**
     *
     * \brief
     *     Send command to delete file. The mode must switch to \ref E_DeviceMode_Playback "E_DeviceMode_Playback" and call GPCam_SendGetFullFileList() first.
     *
     * \details
     *    PFN_CmdStatusCallBack will bring the Command result. The parameters of PFN_CmdStatusCallBack as follow:\n
     *    <DL>
     *   <dt><EM>i32Mode</dt></EM>          <dd>GPSOCK_MODE_Playback</dd>
     *   <dt><EM>i32CMDID</dt></EM>         <dd>GPSOCK_Playback_CMD_DeleteFile</dd>
     *   <dt><EM>i32DataSize</dt></EM>      <dd>The Data Size is 0</dd>
     *   <dt><EM>pbyData</dt></EM>          <dd>No Data</dd>
     *    </DL>
     *
     * \param[in] i32Index
     *    The file index.
     * \return
     *    Return the command queue index.
     * \see
     *    GPCam_SendSetMode
     * \see
     *    PFN_CmdStatusCallBack
     */
    int GPCam_DeleteFile(int i32Index); //for KidsCam
#if defined(__cplusplus)
}
#endif

#endif /* defined(__GPCam__CommandAgentInterface__) */
