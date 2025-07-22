//
//  CommandAgent.cpp
//  GPCam
//
//  Created by generalplus_sa1 on 8/11/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//
#include <sys/time.h>
#include "VideoCommandAgent.h"
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define Thumbnail_Folder_Name     "Thumbnail"

static pthread_mutex_t Preview_lock;
static pthread_mutex_t FileAttr_lock;

Create_Agent_Helper(C_VideoCommandAgent)

//----------------------------------------------------
C_VideoCommandAgent::C_VideoCommandAgent():
C_CommandAgent(),
m_i32CurrentPlayIndex(-1),
m_pbyRealVendorPayload(NULL),
m_pbyRealVendorPayloadSize(0),
m_PacketParser(&m_TcpSocket),
m_i32NextPreviewIndex(-1)
{
    pthread_mutex_init(&Preview_lock, NULL);
    pthread_mutex_init(&FileAttr_lock, NULL);
    SetFileNameMapping(DEFAULT_MAPPING_STR);
}
C_VideoCommandAgent::~C_VideoCommandAgent()
{
    ClearNameList();
    ClearFileList();
}
//----------------------------------------------------
void C_VideoCommandAgent::SetDownloadPath(const char* ptszPath)
{
    strncpy(m_szDownloadFileFolder,ptszPath,256);
}
//----------------------------------------------------
bool C_VideoCommandAgent::IsFileIndexReady(int i32Index)
{
    bool bRet = true;

    do
    {
        if(m_FileAttributeList.size()<=i32Index)
        {
            bRet = false;
            break;
        }
        
        S_FileAttribute *pAttr =  m_FileAttributeList.at(i32Index);
        if(!pAttr->bIsSet)
        {
            bRet = false;
            break;
        }
        
    }while(0);
    
    return bRet;
}
//----------------------------------------------------
int C_VideoCommandAgent::GetFileAttrListSize()
{
    int i32Size = 0;
    
    pthread_mutex_lock(&FileAttr_lock);
    i32Size = ((int)m_FileAttributeList.size());
    pthread_mutex_unlock(&FileAttr_lock);
    
    return i32Size;
}
//----------------------------------------------------
int C_VideoCommandAgent::SetFileAttr(int i32Index,BYTE* pbyFileAttr,int i32ExtraSize)
{
    int i32Ret = 0;
    pthread_mutex_lock(&FileAttr_lock);
    
    do
    {
        if(m_FileAttributeList.size()<=i32Index)
        {
            i32Ret = -1;
            break;
        }
        
        S_FileAttribute *pAttr =  m_FileAttributeList.at(i32Index);
        
        memcpy(pAttr,pbyFileAttr,Get_FileAttribute_RealSize);
        if(i32ExtraSize>0)
        {
            pAttr->i32ExtraSize = i32ExtraSize;
            pAttr->pbyExtraInfo = new BYTE[i32ExtraSize];
            memcpy(pAttr->pbyExtraInfo,&pbyFileAttr[Get_FileAttribute_RealSize],i32ExtraSize);
        }
        
        pAttr->bIsSet = true;
        
    }while(0);
    
    pthread_mutex_unlock(&FileAttr_lock);
    

    return 0;
}
//----------------------------------------------------
int C_VideoCommandAgent::AddOneFileAttr(int i32Index,BYTE* pbyFileAttr,int i32ExtraSize)
{
    int i32Ret = 0;
    pthread_mutex_lock(&FileAttr_lock);

    do
    {
        if(m_FileAttributeList.size()<=i32Index)
        {
            i32Ret = -1;
            break;
        }

        S_FileAttribute *pAttr = new S_FileAttribute;
        memset(pAttr,0x00,sizeof(S_FileAttribute));
        memcpy(pAttr,pbyFileAttr,Get_FileAttribute_RealSize);
        if(i32ExtraSize>0)
		{
			pAttr->i32ExtraSize = i32ExtraSize;
			pAttr->pbyExtraInfo = new BYTE[i32ExtraSize];
			memcpy(pAttr->pbyExtraInfo,&pbyFileAttr[Get_FileAttribute_RealSize],i32ExtraSize);
		}

		pAttr->bIsSet = true;

        m_FileAttributeList.insert(m_FileAttributeList.begin(),pAttr);


    }while(0);

    pthread_mutex_unlock(&FileAttr_lock);


    return 0;
}
//----------------------------------------------------
int  C_VideoCommandAgent::AllocFileList(int i32Count)
{
    pthread_mutex_lock(&FileAttr_lock);
    
    for(int i=0;i<i32Count;i++)
    {
        S_FileAttribute *pAttr = new S_FileAttribute;
        memset(pAttr,0x00,sizeof(S_FileAttribute));
        m_FileAttributeList.push_back(pAttr);
    }
    
    pthread_mutex_unlock(&FileAttr_lock);
    
    return 0;
}
//----------------------------------------------------
bool C_VideoCommandAgent::RemoveFileAttr(int i32Index)
{
    bool bRet = true;
    pthread_mutex_lock(&FileAttr_lock);
    
    do
    {
        if(m_FileAttributeList.size()<=i32Index)
        {
            bRet = false;
            break;
        }
        
        FileAttributeVector_iterator iterator = m_FileAttributeList.begin();
        std::advance(iterator, i32Index);
        S_FileAttribute *pObject = (S_FileAttribute *)(*iterator);
        m_FileAttributeList.erase(iterator);
        if(pObject->pbyExtraInfo)
            delete []pObject->pbyExtraInfo;
        
        delete pObject;
        
    }while(0);
    
    pthread_mutex_unlock(&FileAttr_lock);
    
    return bRet;
}
//----------------------------------------------------
char* C_VideoCommandAgent::GetFileName(int i32Index)
{
    static char szTempFileName[16];
    memset(szTempFileName,0x00,sizeof(szTempFileName));
    
    pthread_mutex_lock(&FileAttr_lock);
    
    do
    {
        if(!IsFileIndexReady(i32Index))
            break;
        
        S_FileAttribute *pAttr =  m_FileAttributeList.at(i32Index);
        
        bool bFound = false;
        
        FileNameMapping_iterator iterator_File;
        for(iterator_File=m_FileNameMappingList.begin(); iterator_File!=m_FileNameMappingList.end(); ++iterator_File)
        {
            S_FileNameMappingInfo *pMapping = (S_FileNameMappingInfo *)*iterator_File;
            if(pMapping->szExt == pAttr->byExt)
            {
                sprintf(szTempFileName,"%s%04d.%s",pMapping->szPrefixName,pAttr->U16FileIndex,pMapping->szType);
                bFound = true;
                break;
            }
        }
        
        if(!bFound)
        {
            sprintf(szTempFileName,"UNKOWN%04d.bin",pAttr->U16FileIndex);
        }
        
    }while(0);
    
    pthread_mutex_unlock(&FileAttr_lock);

    return szTempFileName;
}
//----------------------------------------------------
bool C_VideoCommandAgent::SetFileNameMapping(const char* ptszFileNameMapping)
{
    
    ClearNameList();
    
    char *pOption,*pToken,*pValue;
    char Key[32],Name[32],Type[32];
    
    char temp[1024];
    strncpy(temp,ptszFileNameMapping,1024);
    
    pOption = strtok (temp,";");
    while (pOption != NULL)
    {
        pToken= strchr(pOption,'=');
        strncpy(Key,pOption,pToken-pOption);
        Key[pToken-pOption] = 0;
        pToken++;
        pValue = pToken;
        
        pToken= strchr(pValue,',');
        strncpy(Name,pValue,pToken-pValue);
        Name[pToken-pValue] = 0;
        pToken++;
        
        strcpy(Type,pToken);
        
        
        S_FileNameMappingInfo *pMapping = new S_FileNameMappingInfo;
        memset(pMapping,0x00,sizeof(S_FileNameMappingInfo));
        strncpy(&pMapping->szExt,Key,1);
        strncpy(pMapping->szPrefixName,Name,16);
        strncpy(pMapping->szType,Type,16);
        
        m_FileNameMappingList.push_back(pMapping);
        
        pOption = strtok (NULL, ";");
    }
    
    return true;
}
//----------------------------------------------------
int   C_VideoCommandAgent::GetFileIndex(int i32Index)
{
    pthread_mutex_lock(&FileAttr_lock);

    int i32Ret = 0;
    
    do
    {
        if(!IsFileIndexReady(i32Index))
        {
            i32Ret = -1;
            break;
        }
        
        S_FileAttribute *pAttr =  m_FileAttributeList.at(i32Index);
        i32Ret = pAttr->U16FileIndex;
        
    }while(0);
    
    pthread_mutex_unlock(&FileAttr_lock);
    
    return i32Ret;
}
//----------------------------------------------------
bool C_VideoCommandAgent::GetFileTime(int i32Index,BYTE *pTime)
{
    
    pthread_mutex_lock(&FileAttr_lock);
    
    bool bRet = true;
    
    do
    {
        if(!IsFileIndexReady(i32Index))
        {
            bRet = false;
            break;
        }
        
        S_FileAttribute *pAttr =  m_FileAttributeList.at(i32Index);
        memcpy(pTime,&pAttr->byYear,6);
        
    }while(0);
    
    pthread_mutex_unlock(&FileAttr_lock);
    
    return bRet;
}
//----------------------------------------------------
unsigned int  C_VideoCommandAgent::GetFileSize(int i32Index)
{
    pthread_mutex_lock(&FileAttr_lock);
    
    int i32Ret = 0;
    
    do
    {
        if(!IsFileIndexReady(i32Index))
        {
            i32Ret = 0;
            break;
        }
        
        S_FileAttribute *pAttr =  m_FileAttributeList.at(i32Index);
        i32Ret = pAttr->i32FileSize;
        
    }while(0);
    
    pthread_mutex_unlock(&FileAttr_lock);
    
    return i32Ret;
}
//----------------------------------------------------
BYTE C_VideoCommandAgent::GetFileExt(int i32Index)
{
    pthread_mutex_lock(&FileAttr_lock);
    
    BYTE byRet = 0;
    
    do
    {
        if(!IsFileIndexReady(i32Index))
        {
            byRet = 0;
            break;
        }
        
        S_FileAttribute *pAttr =  m_FileAttributeList.at(i32Index);
        byRet = pAttr->byExt;
        
    }while(0);
    
    pthread_mutex_unlock(&FileAttr_lock);
    
    return byRet;
}
//----------------------------------------------------
BYTE *C_VideoCommandAgent::GetFileExtraInfo(int i32Index,int *pi32Size)
{
    BYTE *pbyExtraInfo = NULL;
    *pi32Size = 0;
    pthread_mutex_lock(&FileAttr_lock);
    
    do
    {
        if(!IsFileIndexReady(i32Index))
        {
             *pi32Size = 0;
            break;
        }
        
        S_FileAttribute *pAttr =  m_FileAttributeList.at(i32Index);
        *pi32Size = pAttr->i32ExtraSize;
        pbyExtraInfo = pAttr->pbyExtraInfo;
        
    }while(0);
    
    pthread_mutex_unlock(&FileAttr_lock);
    
    return pbyExtraInfo;
}
//----------------------------------------------------
bool C_VideoCommandAgent::GetNextPreview(int *pi32Index)
{
    bool bRet = false;
    
    //get user assing index first
    if(pthread_mutex_trylock(&Preview_lock) == 0)
    {
        if(m_i32NextPreviewIndex!=-1)
        {
            DEBUG_PRINT("\nReadNextPreview, Index: %d\n",m_i32NextPreviewIndex);
            *pi32Index = m_i32NextPreviewIndex;
            m_i32NextPreviewIndex = -1;
        }
    
        pthread_mutex_unlock(&Preview_lock);
    }
    
    pthread_mutex_lock(&FileAttr_lock);
    
    do
    {
        if(m_FileAttributeList.size() == 0)
        {
            bRet = false;
            break;
        }
        
        if(*pi32Index + 1 > m_FileAttributeList.size())
            *pi32Index = 0;
        
        S_FileAttribute *pAttr =  m_FileAttributeList.at(*pi32Index);
        if(!pAttr->bIsSet)
            bRet = true;
        else
        {
            //try to find unset attr
            int i32TryIndex = *pi32Index;
            FileAttributeVector_iterator iterator_File;
            for(iterator_File=m_FileAttributeList.begin() + *pi32Index ;
                iterator_File!=m_FileAttributeList.end();
                ++iterator_File,i32TryIndex++)
            {
                if(!(*iterator_File)->bIsSet)
                {
                    *pi32Index = i32TryIndex-1;
                    bRet = true;
                    break;
                }
            }
            
            if(!bRet)
            {
                int i32TryIndex = 0;
                for(iterator_File=m_FileAttributeList.begin();
                    iterator_File!=m_FileAttributeList.begin() + *pi32Index ;
                    ++iterator_File,i32TryIndex++)
                {
                    if(!(*iterator_File)->bIsSet)
                    {
                        *pi32Index = i32TryIndex-1;
                        bRet = true;
                        break;
                    }
                }
            }
        }
        
    }while(0);
    
    pthread_mutex_unlock(&FileAttr_lock);
    
    return bRet;
}
//----------------------------------------------------
//General
int C_VideoCommandAgent::SendSetMode(int i32Mode)
{
    return SendCommand(new C_SetModeCmd(this,this,m_CmdStatuCallBack,i32Mode));
}
//----------------------------------------------------
int C_VideoCommandAgent::SendGetSetPIP(int i32Type)
{
    return SendCommand(new C_GetSetPIPCmd(this,this,m_CmdStatuCallBack,i32Type));
}
//----------------------------------------------------
int C_VideoCommandAgent::SendGetStatus()
{
    return SendCommand(new C_GetDevuceStatusCmd(this,this,m_CmdStatuCallBack));
}
//----------------------------------------------------
int C_VideoCommandAgent::SendGetParameterFile(const char* ptszFilaName)
{
    char tempPath[256];
    strcpy(tempPath,m_szDownloadFileFolder);
    strcat(tempPath,"/");
    strcat(tempPath,ptszFilaName);
    
    return SendCommand(new C_GetParameterFileCmd(this,this,m_CmdStatuCallBack,tempPath));
}
//----------------------------------------------------
int C_VideoCommandAgent::SendPowerOff()
{
    return SendCommand(new C_SendPowerOffCmd(this,this,m_CmdStatuCallBack));
}
//----------------------------------------------------
int C_VideoCommandAgent::SendRestartStreaming()
{
    return SendCommand(new C_SendRestartStreamingCmd(this,this,m_CmdStatuCallBack));
}
//----------------------------------------------------
//Record
int C_VideoCommandAgent::SendRecordCmd()
{
    return SendCommand(new C_RecordCmd(this,this,m_CmdStatuCallBack));
}
//----------------------------------------------------
int C_VideoCommandAgent::SendAudioOnOff(bool bOn)
{
    return SendCommand(new C_AudioCmd(this,this,m_CmdStatuCallBack,bOn));
}
//----------------------------------------------------
//Capture picture
int  C_VideoCommandAgent::SendCapturePicture()
{
    return SendCommand(new C_CapturePictureCmd(this,this,m_CmdStatuCallBack));
}
//----------------------------------------------------
//Playback
int C_VideoCommandAgent::SendStartPlayback(int i32Index)
{
    m_i32CurrentPlayIndex = i32Index;

    return SendCommand(new C_PlaybackStartCmd(this,this,m_CmdStatuCallBack,i32Index));
}
//----------------------------------------------------
int C_VideoCommandAgent::SendPausePlayback()
{
    if(m_i32CurrentPlayIndex==-1)
    {
        FireNak();
        return -1;
    }
    
    return SendCommand(new C_PlaybackPauseCmd(this,this,m_CmdStatuCallBack,m_i32CurrentPlayIndex));
}
//----------------------------------------------------
int C_VideoCommandAgent::SendGetFullFileList()
{
    ClearFileList();
    m_i32NextPreviewIndex = -1;
    return SendCommand(new C_GetPlaybackFileCountCmd(this,this,m_CmdStatuCallBack));
}
//----------------------------------------------------
int C_VideoCommandAgent::SendGetFileByIndex(int i32Index)
{
    return SendCommand(new C_GetPlaybackNameListCmdReverse(this,this,m_CmdStatuCallBack,-1,1,i32Index));
}
//----------------------------------------------------
int C_VideoCommandAgent::SendGetFileThumbnail(int i32Index)
{
    bool bCheck = true;
    pthread_mutex_lock(&FileAttr_lock);
    bCheck = IsFileIndexReady(i32Index);
    pthread_mutex_unlock(&FileAttr_lock);
    if(!bCheck)
        return -1;
    
    char tempPath[256];
    strcpy(tempPath,m_szDownloadFileFolder);
    strcat(tempPath,"/");
    strcat(tempPath,Thumbnail_Folder_Name);
    strcat(tempPath,"/Thumbnail_");
    strcat(tempPath,GetFileName(i32Index));
    
    struct stat st = {0};
    char ThumbnailPath[256];
    strcpy(ThumbnailPath,m_szDownloadFileFolder);
    strcat(ThumbnailPath,"/");
    strcat(ThumbnailPath,Thumbnail_Folder_Name);
    
    if (stat(ThumbnailPath, &st) == -1)
        mkdir(ThumbnailPath, 0755);
    
    return SendCommand(new C_GetPlaybackThumbnailCmd(this,this,m_CmdStatuCallBack,i32Index,tempPath));
}
//----------------------------------------------------
int C_VideoCommandAgent::SendGetFileRawdata(int i32Index)
{
    bool bCheck = true;
    pthread_mutex_lock(&FileAttr_lock);
    bCheck = IsFileIndexReady(i32Index);
    pthread_mutex_unlock(&FileAttr_lock);
    if(!bCheck)
        return -1;
    
    char tempPath[256];
    strcpy(tempPath,m_szDownloadFileFolder);
    strcat(tempPath,"/");
    strcat(tempPath,GetFileName(i32Index));
    
    return SendCommand(new C_GetPlaybackFileRawDataCmd(this,this,m_CmdStatuCallBack,i32Index,tempPath));
}
//----------------------------------------------------
int C_VideoCommandAgent::SendStopPlayback()
{
    if(m_i32CurrentPlayIndex==-1)
    {
        FireNak();
        return -1;
    }
    
    return SendCommand(new C_PlaybackStopCmd(this,this,m_CmdStatuCallBack,m_i32CurrentPlayIndex));
    
}
//----------------------------------------------------
int C_VideoCommandAgent::SetNextPreviewFileList(int i32Index)
{
    pthread_mutex_lock(&FileAttr_lock);
    int i32Totol = (int)m_FileAttributeList.size();
    pthread_mutex_unlock(&FileAttr_lock);
    
    if(i32Index > i32Totol)
        i32Index = i32Totol;
    
    pthread_mutex_lock(&Preview_lock);
    
    m_i32NextPreviewIndex = i32Index - (i32Index % 16) - 1;
    
    if(m_i32NextPreviewIndex<0)
        m_i32NextPreviewIndex = 0;
    
    pthread_mutex_unlock(&Preview_lock);
    
    return 0;
}
//----------------------------------------------------
int C_VideoCommandAgent::SendDeleteFile(int i32Index)
{
    pthread_mutex_lock(&FileAttr_lock);
    int i32Totol = (int)m_FileAttributeList.size();
    pthread_mutex_unlock(&FileAttr_lock);
    
    if(i32Index >= i32Totol)
        return -1;
    
    int i32DeviceIndex = GetFileIndex(i32Index);
    
    return SendCommand(new C_GetPlaybackDeleteFile(this,this,m_CmdStatuCallBack,i32DeviceIndex,i32Index));
}
//----------------------------------------------------
//Menu
int C_VideoCommandAgent::SendGetParameter(int i32ID)
{
    return SendCommand(new C_GetParameterCmd(this,this,m_CmdStatuCallBack,i32ID));
}
//----------------------------------------------------
int C_VideoCommandAgent::SendSetParameter(int i32ID, int i32Size, BYTE* pbyData)
{
    return SendCommand(new C_SetParameterCmd(this,this,m_CmdStatuCallBack,i32ID,i32Size,pbyData));
}
//----------------------------------------------------
//Firmware
int C_VideoCommandAgent::SendFirmwareDownload(unsigned int ui32FileSize, unsigned int ui32CheckSum)
{
    return SendCommand(new C_FirmwareDownloadCmd(this,this,m_CmdStatuCallBack,ui32FileSize,ui32CheckSum));
}
//----------------------------------------------------
int C_VideoCommandAgent::SendFirmwareRawData(unsigned int ui32Size, BYTE* pbyData)
{
    return SendCommand(new C_FirmwareSendRawDataCmd(this,this,m_CmdStatuCallBack,ui32Size,pbyData));
}
//----------------------------------------------------
int C_VideoCommandAgent::SendFirmwareUpgrade()
{
     return SendCommand(new C_FirmwareUpgradeCmd(this,this,m_CmdStatuCallBack));
}
//----------------------------------------------------
//CV Firmware
int C_VideoCommandAgent::SendCVFirmwareDownload(unsigned int ui32FileSize, unsigned int ui32CheckSum)
{
    return SendCommand(new C_CVFirmwareDownloadCmd(this,this,m_CmdStatuCallBack,ui32FileSize,ui32CheckSum));
}
//----------------------------------------------------
int C_VideoCommandAgent::SendCVFirmwareRawData(unsigned int ui32Size, BYTE* pbyData)
{
    return SendCommand(new C_CVFirmwareSendRawDataCmd(this,this,m_CmdStatuCallBack,ui32Size,pbyData));
}
//----------------------------------------------------
int C_VideoCommandAgent::SendCVFirmwareUpgrade(unsigned int ui32Area)
{
     return SendCommand(new C_CVFirmwareUpgradeCmd(this,this,m_CmdStatuCallBack,ui32Area));
}
//----------------------------------------------------
// Vendor
int C_VideoCommandAgent::SendVendorCmd(BYTE* pbydata ,int i32Size)
{

    if(i32Size + 2 > m_pbyRealVendorPayloadSize)
    {
        SAFE_DELETE_ARRAY(m_pbyRealVendorPayload)
        m_pbyRealVendorPayload = new BYTE[i32Size+2];
        m_pbyRealVendorPayloadSize = i32Size+2;
    }
    
    m_pbyRealVendorPayload[0] = i32Size & 0xFF;
    m_pbyRealVendorPayload[1] = (i32Size>>8) & 0xFF;
    memcpy(&m_pbyRealVendorPayload[2],pbydata,i32Size);
    
    
    return SendCommand(new C_SendVendorCmd(this,this,m_CmdStatuCallBack,m_pbyRealVendorPayload,m_pbyRealVendorPayloadSize));
}
//----------------------------------------------------
//Action
int C_VideoCommandAgent::InsertCommandDelay(int i32ms)
{
    return SendCommand(new C_DelayCmd(this,this,m_CmdStatuCallBack,i32ms));
}
//----------------------------------------------------
void  C_VideoCommandAgent::ClearFileList()
{
    DEBUG_PRINT("ClearFileList\n");
    pthread_mutex_lock(&FileAttr_lock);
    
    FileAttributeVector_iterator iterator_File;
    for(iterator_File=m_FileAttributeList.begin(); iterator_File!=m_FileAttributeList.end(); ++iterator_File)
    {
        S_FileAttribute *pObject = (S_FileAttribute *)(*iterator_File);
        if(pObject->pbyExtraInfo)
            delete []pObject->pbyExtraInfo;
        
        delete (*iterator_File);
    }
    
    m_FileAttributeList.clear();
    
    pthread_mutex_unlock(&FileAttr_lock);
}
//----------------------------------------------------
void  C_VideoCommandAgent::ClearNameList()
{
    DEBUG_PRINT("ClearNameList\n");
    FileNameMapping_iterator iterator_File;
    for(iterator_File=m_FileNameMappingList.begin(); iterator_File!=m_FileNameMappingList.end(); ++iterator_File)
        delete (*iterator_File);
    
    m_FileNameMappingList.clear();
}
//----------------------------------------------------
bool C_VideoCommandAgent::CommandBeforeConnect()
{
    ClearFileList();
    return true;
}
//----------------------------------------------------
bool C_VideoCommandAgent::CommandAfterConnected()
{
    C_AuthDeviceCmd *Command = new C_AuthDeviceCmd(this,this,m_CmdStatuCallBack);
    QueueCmd(Command);
    
    return true;
}
//----------------------------------------------------
I_PacketParser* C_VideoCommandAgent::GetPacketParser()
{
    return &m_PacketParser;
}
//----------------------------------------------------
int C_VideoCommandAgent::SendCheckFileMapping()
{
    return SendCommand(new C_CheckFileMappingCmd(this,this,m_CmdStatuCallBack));
}
//----------------------------------------------------
int C_VideoCommandAgent::DeleteFile(int i32Index)
{
    pthread_mutex_lock(&FileAttr_lock);
    int i32Totol = (int)m_FileAttributeList.size();
    pthread_mutex_unlock(&FileAttr_lock);
    
    if(i32Index >= i32Totol)
        return -1;
    
    this->RemoveFileAttr(i32Index);
    return E_HandleAck_Retcode_NoError;
}
