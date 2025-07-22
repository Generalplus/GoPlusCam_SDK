The Main Section {#the_main_section}
================
# Description {#the_Description_section}

The GPCam library is for making it easily to interact with the devices which embed Generalplus chips on iOS platform.
This API specification describes all the APIs provided by the GPCam library.

The library provides following functions:
1. Connect to GPCam server on device.
2. Send/Receive TCP command to/from GPCam server.

<b>The GPCam library does not provide streaming video function, please using GPFFmpeg library or third party library to streaming video. Example: [VLC for iOS](http://www.videolan.org/vlc/download-ios.html)</b>

# Usage {#the_Usage_section}

Add the static library file and header files into your project.

## Static Library {#the_StaticLibrary_section}

The GPCam library is in static library on iOS platform. It is written by C++, before linking our library you must add the linker flag <b>-lstdc++.6</b> to compile target project.

Static Library Name: libGPCam.a

## Header Files {#the_HeaderFiles_section}

1. GPCamCommandAPI.h: The function definitions which is used to manipulate the GPCam library.
2. GPCamTypedef.h: The type definitions of the GPCam library.

## Calling Sequence {#the_CallingSequence_section}

### Connect {#the_Connect_section}

1. GPCam_SetCmdStatusCallBack(): Registers a user callback function for reporting command processing status.
2. GPCam_SetDownloadPath(): Set the download path for saving thumbnail, media file and temporary file.
3. GPCam_ConnectToDevice(): Connect to server IP="192.168.25.1",Port="8081".

### Send Command {#the_SendCommand_section}

After connected to server, all GPCam_SendXXXX() is available. @ref PFN_CmdStatusCallBack will bring the command result.

### Abort Command {#the_AbortCommand_section}

GPCam_SendXXXX() will return the command index. Pass this command index to GPCam_Abort() to abort the command.

### Switching Device Mode  {#the_SwitchingDeviceMode_section}

There are four device mode can be switch. To switch mode by sending GPCam_SendSetMode(). Following are available commands for each mode.

Mode                            | Available commands
------------------------------- | ----------------------
@ref E_DeviceMode_Record        | GPCam_SendRecordCmd()\n GPCam_SendAudioOnOff()
@ref E_DeviceMode_Capture       | GPCam_SendCapturePicture()
@ref E_DeviceMode_Playback      | GPCam_SendStartPlayback()\n GPCam_SendPausePlayback()\n GPCam_SendGetFullFileList()\n GPCam_SendGetFileThumbnail()\n GPCam_SendGetFileRawdata()\n GPCam_SendStopPlayback()
@ref E_DeviceMode_Menu          | GPCam_SendGetParameter()\n GPCam_SendSetParameter()

If command is not acceptable by current device mode, the status call back will bring @ref GP_SOCK_TYPE_NAK with error code @ref Error_ModeError.

### Record/Capture Mode{#the_RecordCapture_section}

#### Record/Capture Streaming {#the_RecordCaptureStreaming_section}

- Switch between @ref E_DeviceMode_Record and @ref E_DeviceMode_Capture
1. If is recording or continuous shooting(see @ref GPCam_SendGetStatus()), stop the recording or continuous shooting first by sending GPCam_SendRecordCmd() \\ GPCam_SendCapturePicture().
2. GPCam_SendSetMode(): Switching device mode to @ref E_DeviceMode_Record  / @ref E_DeviceMode_Capture.

- Switch from @ref E_DeviceMode_Playback/@ref E_DeviceMode_Menu mode
1. If is streaming, stop streaming first.
2. GPCam_SendSetMode(): Switching device mode to @ref E_DeviceMode_Record  / @ref E_DeviceMode_Capture.
3. Wait for switching complete.
4. Using streaming library to connect to "http://192.168.25.1:8080/?action=stream" or "rtsp://192.168.25.1:8080/?action=stream".
5. GPCam_SendRestartStreaming(): To restart streaming.

#### Stop Record/Capture Streaming {#the_StopRecordCaptureStreaming_section}

-  Using streaming library to stop streaming.

#### Recording/Capture picture{#the_RecordingCapturePicture_section}

1. Record/Capture Streaming first.
2. GPCam_SendRecordCmd() / GPCam_SendCapturePicture().

### Playback Mode{#the_Playback_section}

#### File list {#the_Filelist_section}

1. If is recording or continuous shooting(see @ref GPCam_SendGetStatus()), stop the recording or continuous shooting first by sending GPCam_SendRecordCmd() \\ GPCam_SendCapturePicture().
2. Using streaming library to stop streaming.
3. GPCam_SendSetMode(): Switching device mode to @ref E_DeviceMode_Playback "E_DeviceMode_Playback".
4. GPCam_SendGetFullFileList(): Get file list from device
5. Wait for GPCam_SendGetFullFileList() complete. The status call back will notify the file information is ready.
6. Send GPCam_SetNextPlaybackFileListIndex() with index to get the file informations that needs to read first. (Optional)

#### Playback Streaming {#the_PlaybackStreaming_section}

1. Using streaming library to connect to "http://192.168.25.1:8080/?action=stream" or "rtsp://192.168.25.1:8080/?action=stream".
2. GPCam_SendRestartStreaming(): To restart streaming.
3. GPCam_SendStartPlayback(): To streaming specific file.

#### Stop Playback Streaming {#the_StopPlaybackStreaming_section}

1. GPCam_SendStopPlayback(): To stop streaming.
2. Wait for GPCam_SendStopPlayback() complete.
3. Using streaming library to stop streaming.

#### Download Thumbnail {#the_DownloadThumbnail_section}

1. Wait for GPCam_SendGetFullFileList() complete. The status call back will notify the file information is ready.
2. Send GPCam_SetNextPlaybackFileListIndex() with index to get the file informations that needs to read first. (Optional)
3. GPCam_SendGetFileThumbnail(): To get thumbnail.

#### Download Raw Data {#the_Download_section}

1. Wait for GPCam_SendGetFullFileList() complete. The status call back will notify the file information is ready.
2. GPCam_SendGetFileRawdata(): To get file raw data.

#### Stop download Raw Data {#the_Stopdownload_section}

1. Pass the command index of GPCam_SendGetFileRawdata() to GPCam_Abort().
2. Send any command to stop GPCam_SendGetFileRawdata().
3. Previous command is not be accepted by device, and will receive @ref GP_SOCK_TYPE_NAK command. Ignore this @ref GP_SOCK_TYPE_NAK.

### Menu Mode{#the_Menu_section}

#### Read settings from device {#the_Readsettingfromdevice_section}

1. If is recording or continuous shooting(see @ref GPCam_SendGetStatus()), stop the recording or continuous shooting first by sending GPCam_SendRecordCmd() \\ GPCam_SendCapturePicture().
2. Using streaming library to stop streaming.
3. GPCam_SendSetMode(): Switching device mode to @ref E_DeviceMode_Menu.
4. GPCam_SendGetParameter(): To get all device settings.

#### Upgrade Device Firmware {#the_UpgradeDeviceFirmware_section}

1. GPCam_SendFirmwareDownload(): Send firmware checksum and file size.
2. GPCam_SendFirmwareRawData(): Firmware raw data.
3. GPCam_SendFirmwareUpgrade(): Send command to start Upgrade.

#### Device Setting File {#the_DeviceSettingFile_section}

GPCam_SendGetParameterFile() will get the setting file from device . This file describe the available settings on device, and these setting can be set/get by sending GPCam_SendSetParameter() / GPCam_SendGetParameter().

The setting file is an xml file and file format as follow:

* Menu Element
Attribute|Description
---      |---
version	 |Version

Element   |Description
---       |---
Categories|See Categories Element

* Categories Element
Element	|Description
---     |---
Category|See Category Element

* Category Element
Element	|Description
---     |---
Name	|Category name
Settings|See Settings Element

* Settings Element
Element	|Description
---     |---
Setting	|See Setting Element

* Setting Element
Element	|Description
---     |---
Name	|Setting name
ID	    |Setting ID
Type	|Setting type \n 0 = Select value from multiple value elements. \n 1 = No value element, Pop out dialog to confirm action. Ex: Format \n 2 = Input string, No value element and Default element is string. Ex: Setting wifi name. \n 3 = Display only one value element. Ex: Version  \n 4 = Action on smart device. No value element, Pop out dialog to confirm action. Ex: Clear buffer
Reflash	|Reflash value after setting parameter. Optional and default is 0. \n 0 = No reflash  \n 1 = Reflash all setting
Default	|Default value. Can be value ID or string , depend on Type Element.
Values	|See Values Element

* Values Element
Element |Description
---     |---
Element	|Description
Value	|See Value Element

* Value level
Element	|Description
---     |---
Name	|Value name
ID 	    |Value ID
<!-- end of list -->

* Example
```{.xml}
<?xml version="1.0" encoding="UTF-8"?>
<Menu version="1.0">
	<Categories>
		<Category>
			<Name>Record</Name>
			<Settings>
				<Setting>
					<Name>Resolution</Name>
					<ID>0x0000000</ID>
					<Type>0x00</Type>
					<Default>0x01</Default>
					<Values>
						<Value>
							<Name>1080FHD 1920x1080 30fps</Name>
							<ID>0x00</ID>
						</Value>
						<Value>
							<Name>1080P 1440x1080 30fps</Name>
							<ID>0x01</ID>
						</Value>
						<Value>
							<Name>720P 1280x720 30fps</Name>
							<ID>0x02</ID>
						</Value>
						<Value>
							<Name>WVGA 848x480 30fps</Name>
							<ID>0x03</ID>
						</Value>
						<Value>
							<Name>VGA 640x480 30fps</Name>
							<ID>0x04</ID>
						</Value>
					</Values>
				</Setting>
			</Settings>
		</Category>
	</Categories>
</Menu>
```
<!-- end of list -->

# Important Notice {#the_ImportantNotice_section}

GENERALPLUS TECHNOLOGY INC. reserves the right to change this documentation without prior notice.  
Information provided by GENERALPLUS TECHNOLOGY INC. is believed to be accurate and reliable.  
However, GENERALPLUS TECHNOLOGY INC. makes no warranty for any errors which may appear in this document.  
Contact GENERALPLUS TECHNOLOGY INC. to obtain the latest version of device specifications before placing your order.  
No responsibility is assumed by GENERALPLUS TECHNOLOGY INC. for any infringement of patent or other rights of third
parties which may result from its use.  In addition, GENERALPLUS products are not authorized for use as critical
components in life support devices/ systems or aviation devices/systems, where a malfunction or failure of the product may
reasonably be expected to result in significant injury to the user, without the express written approval of Generalplus.

# Author  {#the_Author_section}

Shawnliu , Shawnliu@generalplus.com

# License {#the_License_section}

Copyright (c) 2015 GeneralPlus. All rights reserved.

# Revision History {#the_RevisionHistory_section}

Version   | Date       | Description
---       | ---        | ---
V1.0.6    | 2017/08/18 | GPCam_GetFileExtraInfo() return extra file information. GPCam_SendDeleteFile() send command to delete on device.
V1.0.5    | 2017/06/13 | GPCam_SendGetStatus() return data update. GPCam_SendVendorCmd() add setting time command.
V1.0.4    | 2016/11/16 | Add GPCam_SetFileNameMapping() command.
V1.0.3    | 2016/03/10 | Add firmware upgrade commands.
V1.0.2    | 2015/11/30 | Reduce reconnect time after waking up from sleep. Add GPCam_ClearCmdQueue() command.
V1.0.1    | 2015/10/30 | Update Calling Sequence section and the description of GPCam_SendGetFileThumbnail(),GPCam_SendGetStatus().
V1.0.0.1  | 2015/10/23 | Add GPCam_Version() command and the Stop download Raw Data section.\n
V1.0.0    | 2015/10/16 | First release
