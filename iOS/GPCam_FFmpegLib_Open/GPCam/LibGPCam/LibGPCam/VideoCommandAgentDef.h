//
//  CommandAgentDef.h
//  GPCam
//
//  Created by generalplus_sa1 on 9/22/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#ifndef GPCam_VideoCommandAgentDef_h
#define GPCam_VideoCommandAgentDef_h

#include "GPCamTypedef.h"
//--------------------------------------------------------------
//Define
//--------------------------------------------------------------
//Tag 8Byte
#define GP_SOCK_Tag                            "GPSOCKET"

#define GPSOCK_HEADER_SIZE			            (8 + 2 + 1 + 1)
#define Playback_FileAttr_MAX                   16

#pragma pack(push)
#pragma pack(1)
typedef struct tag_FileAttribute
{
    BYTE            byExt;        // 'A' = .avi , 'J' = jpeg
    UInt16          U16FileIndex;
    BYTE            byYear;       // start from 2000
    BYTE            byMouth;
    BYTE            byDay;
    BYTE            byHour;
    BYTE            byMinute;
    BYTE            bySecond;
    UInt32          i32FileSize;
    BYTE            *pbyExtraInfo;
    UInt32          i32ExtraSize;
    bool            bIsSet;
    
}S_FileAttribute;
#pragma pack(pop)

#define Get_FileAttribute_RealSize  (sizeof(S_FileAttribute) - Get_FileAttribute_ExtraSize)
#define Get_FileAttribute_ExtraSize (sizeof(bool) + sizeof(BYTE*) + sizeof(UInt32))
//--------------------------------------------------------------
//Interface
//--------------------------------------------------------------

class I_ViedoFileAgent
{
public:
    virtual int   GetFileAttrListSize()=0;
    virtual int   SetFileAttr(int i32Index,BYTE* pbyFileAttr,int i32ExtraSize)=0;
    virtual int   AddOneFileAttr(int i32Index,BYTE* pbyFileAttr,int i32ExtraSize)=0;
    virtual int   AllocFileList(int i32Count) = 0;
    virtual bool  RemoveFileAttr(int i32Index)=0;
    
    virtual int   GetFileIndex(int i32Index)=0;
    virtual unsigned int   GetFileSize(int i32Index)=0;
    virtual BYTE  GetFileExt(int i32Index)=0;
    virtual BYTE  *GetFileExtraInfo(int i32Index,int *pi32Size)=0;
    
    virtual bool  GetNextPreview(int *pi32Index)=0;
};


#endif
