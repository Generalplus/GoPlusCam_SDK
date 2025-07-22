//
//  Typedef.h
//  GPCam
//
//  Created by generalplus_sa1 on 8/11/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#ifndef GPCam_Commondef_h
#define GPCam_Commondef_h

//--------------------------------------------------------------
//Packet format
//--------------------------------------------------------------
//GP_SOCK_TYPE 2Byte
#define GP_SOCK_TYPE_CMD                        0x0001
#define GP_SOCK_TYPE_ACK                        0x0002
#define GP_SOCK_TYPE_NAK                        0x0003

//--------------------------------------------------------------
//Define
//--------------------------------------------------------------
#if __LP64__
typedef unsigned int                    UInt32;
typedef signed int                      SInt32;
#else
typedef unsigned long                   UInt32;
typedef signed long                     SInt32;
#endif

typedef unsigned short                  UInt16;

#define UINT16 UInt16
#define BYTE unsigned char

#ifdef _UNICODE
typedef wchar_t				TCHAR;
typedef const wchar_t*		LPCSTR;
typedef	const wchar_t*		LPCTSTR;
#else
typedef char                TCHAR;
typedef const char*         LPCSTR;
typedef	const char*         LPCTSTR;
#endif

#ifndef	FALSE
#define	FALSE		false
#endif

#ifndef	TRUE
#define	TRUE		true
#endif

//--------------------------------------------------------------
//Nak Error Code
//--------------------------------------------------------------
#define Error_ServerIsBusy                       -1
#define Error_InvalidCommand                     -2
#define Error_RequestTimeOut                     -3
#define Error_ModeError                          -4
#define Error_NoStorage                          -5
#define Error_WriteFail                          -6
#define Error_GetFileListFail                    -7
#define Error_GetThumbnailFail                   -8
#define Error_FullStorage                        -9
#define Error_BatteryLow                         -10
#define Error_MemMallocError                     -11
#define Error_ChecksumError                      -12
#define Error_SDNoFile                           -13
#define Error_MAXDeviceErrorCode                 Error_SDNoFile

#define Error_LostConnection                     -64

//--------------------------------------------------------------
//Call back functions
//--------------------------------------------------------------
/**
 * \brief
 *	The callback function of command processing status.
 *
 * \details
 *	The parameters of PFN_CmdStatusCallBack as follow:\n
 *	<DL>
 *   <dt><EM>i32CMDIndex</dt></EM>  	<dd>Command queue index</dd>
 *   <dt><EM>i32Type</dt></EM>  		<dd>GP_SOCK_TYPE_ACK or GP_SOCK_TYPE_NAK. GP_SOCK_TYPE_ACK means command is complete , GP_SOCK_TYPE_ACK is command failed.</dd>
 *   <dt><EM>i32Mode</dt></EM> 		<dd>Command Mode ID</dd>
 *   <dt><EM>i32CMDID</dt></EM>  	<dd>Command ID</dd>
 *   <dt><EM>i32DataSize</dt></EM>  	<dd>The Data Size. If i32Type is GP_SOCK_TYPE_NAK , the size always is 2</dd>
 *   <dt><EM>pbyData</dt></EM>  		<dd>The Data. If i32Type is GP_SOCK_TYPE_NAK , the data is Error code.\n</dd>
 *	</DL>
 *
 */
typedef int (*PFN_CmdStatusCallBack) (int i32CMDIndex,int i32Type,int i32Mode, int i32CMDID, int i32DataSize ,BYTE* pbyData);
/**
 * \brief
 *	The callback function of socket data.
 *
 * \details
 *	The parameters of PFN_SocketDataCallBack as follow:\n
 *	<DL>
 *   <dt><EM>bIsWrite</dt></EM>  	<dd>Read/Write data from/to socket.</dd>
 *   <dt><EM>i32DataSize</dt></EM>  	<dd>The Data Size.</dd>
 *   <dt><EM>pbyData</dt></EM> 		<dd>The Data</dd>
 *	</DL>
 *
 */
typedef int (*PFN_SocketDataCallBack) (bool bIsWrite ,int i32DataSize ,BYTE* pbyData);

//--------------------------------------------------------------
//Status
//--------------------------------------------------------------
typedef enum
{
    E_ConnectionStatus_Idle,
    E_ConnectionStatus_Connecting,
    E_ConnectionStatus_Connected,
    E_ConnectionStatus_DisConnected,
    
}E_ConnectionStatus;


#endif
