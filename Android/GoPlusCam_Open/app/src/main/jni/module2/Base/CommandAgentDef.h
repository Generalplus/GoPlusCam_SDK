//
//  CommandAgentDef.h
//  GPCam
//
//  Created by generalplus_sa1 on 9/22/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#ifndef GPCam_CommandAgentDef_h
#define GPCam_CommandAgentDef_h

#include "GPCommondef.h"
//--------------------------------------------------------------
//Define
//--------------------------------------------------------------
//Tag 8Byte
#define GP_SOCK_Tag                            "GPSOCKET"

#ifndef SAFE_DELETE
#define	SAFE_DELETE(p)	{\
if (p != NULL)\
{\
delete p;\
p = NULL;\
}\
}
#endif //SAFE_DELETE

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p)	{\
if (p != NULL)\
{\
delete [] p;\
p = NULL;\
}\
}
#endif //SAFE_DELETE_ARRAY

#if defined(DEBUG)

    #if defined(ANDROID) || defined(__ANDROID__)
        #include <android/log.h>
        #define DEBUG_PRINT(...) __android_log_print(ANDROID_LOG_ERROR  , "CommandAgent",__VA_ARGS__)
    #else
        #define DEBUG_PRINT(format, ...)  printf( \
        "%*.*s(%.5d): " \
        format, 40, 40 , __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__ \
        )
    #endif

#else
    #define DEBUG_PRINT(format, ...)  ((void)0)
#endif

typedef enum
{
    E_PacketType_CMD = GP_SOCK_TYPE_CMD,
    E_PacketType_ACK = GP_SOCK_TYPE_ACK,
    E_PacketType_NAK = GP_SOCK_TYPE_NAK,
    
}E_PacketType;

#define PACKET_NOERROR          0
#define PACKET_DISCONNECTED     1
#define PACKET_GPTAGMISMATCH    2
#define PACKET_TIMEOUT          3

//timeout
#define DEFAULT_READ_TIMEOUT      20
#define DEFAULT_READRETRY_TIMEOUT 2
#define DEFAULT_CONNECT_TIME      5000

//--------------------------------------------------------------
//Interface
//--------------------------------------------------------------
class I_StreamCommand
{
public:
    
};

class I_CommandAgent
{
public:
    virtual void  QueueCmd(I_StreamCommand* pCommand)=0;
};

class I_PacketParser
{
public:
    virtual int          Parse()=0;
    
    virtual int          GetPayloadSize()=0;
    virtual BYTE*        GetPayload()=0;
    virtual short        GetErrorCode()=0;
    virtual E_PacketType GetType()=0;
    virtual BYTE         GetMode()=0;
    virtual BYTE         GetCMD()=0;
};
//--------------------------------------------------------------
//Socket Error Code
//--------------------------------------------------------------
#define Error_Socket_Broken                      -1
#define Error_Socket_Timeout                     -2


#endif
