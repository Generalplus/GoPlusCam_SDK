#include "TCPSocket.h"
#include <string.h>
#include <stdio.h>
#include <netdb.h>
#include <netinet/tcp.h>	// for TCP_NODELAY

#define Close_Socket close
#define SOCKET_ERROR -1

#if defined(ANDROID) || defined(__ANDROID__)
#define TCP_KEEPALIVE TCP_KEEPIDLE
#endif

#import <Foundation/Foundation.h>
#import <CoreFoundation/CoreFoundation.h>
#include <sys/time.h>
CFWriteStreamRef g_writeStream = NULL;

void CloseFCStream()
{
    if(g_writeStream)
    {
        CFWriteStreamSetClient(g_writeStream, kCFStreamEventNone, NULL, NULL);
        CFWriteStreamClose(g_writeStream);
        CFWriteStreamUnscheduleFromRunLoop(g_writeStream, CFRunLoopGetMain(), kCFRunLoopCommonModes);
        g_writeStream = NULL;
    }
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------------------------
C_Socket::C_Socket()
{
    m_i32socketId = 0;
}

//------------------------------------------------------------------------------------------------
C_Socket::~C_Socket()
{
    CloseSocket();
}
//------------------------------------------------------------------------------------------------
int C_Socket::CreateSocket(sa_family_t sfamily)
{
    if(m_i32socketId!=0)
    {
        Close_Socket(m_i32socketId);
        m_i32socketId = 0;
    }
    
    if ( (m_i32socketId=socket(sfamily,SOCK_STREAM,0)) == -1)
    {
        return 1;
    }
    
    //reuse address and port
    int i32_value=1;
    if (setsockopt(m_i32socketId,SOL_SOCKET,SO_REUSEADDR,(const char*)&i32_value,sizeof(i32_value)) == -1)
    {
        return 2;
    }
    /*
     //SO_KEEPALIVE option to activate
     int option = 1;
     //Idle time used when SO_KEEPALIVE is enabled. Sets how long connection must be idle before keepalive is sent
     int keepaliveIdle = 60;
     //Interval between keepalives when there is no reply. Not same as idle time
     int keepaliveIntvl = 5;
     //Number of keepalives before close (including first keepalive packet)
     int keepaliveCount = 2;
     //Time after which tcp packet retransmissions will be stopped and the connection will be dropped
     int retransmissionTimeout = 5;
     
     if (setsockopt(m_i32socketId, SOL_SOCKET, SO_KEEPALIVE, &option, sizeof (int)) == -1)
     {
     DEBUG_PRINT("setsockopt SO_KEEPALIVE failed: %s", strerror(errno));
     }
     
     if (setsockopt(m_i32socketId, IPPROTO_TCP, TCP_KEEPCNT, &keepaliveCount, sizeof(int)) == -1)
     {
     DEBUG_PRINT("setsockopt TCP_KEEPCNT failed: %s", strerror(errno));
     }
     
     if (setsockopt(m_i32socketId, IPPROTO_TCP, TCP_KEEPALIVE, &keepaliveIdle, sizeof(int)) == -1)
     {
     DEBUG_PRINT("setsockopt TCP_KEEPALIVE failed: %s", strerror(errno));
     }
     
     if (setsockopt(m_i32socketId, IPPROTO_TCP, TCP_KEEPINTVL, &keepaliveIntvl, sizeof(int)) == -1)
     {
     DEBUG_PRINT("setsockopt TCP_KEEPINTVL failed: %s", strerror(errno));
     }
     
     #if defined(__APPLE__)
     if (setsockopt(m_i32socketId, IPPROTO_TCP, TCP_RXT_CONNDROPTIME, &retransmissionTimeout, sizeof(int)) == -1)
     {
     DEBUG_PRINT("setsockopt TCP_RXT_CONNDROPTIME failed: %s", strerror(errno));
     }
     
     #endif
     */
    //disable Nagle Algorithm
    i32_value=1;
    if (setsockopt(m_i32socketId,IPPROTO_TCP,TCP_NODELAY,(const char*)&i32_value,sizeof(i32_value)) == -1)
    {
        DEBUG_PRINT("Failed to disable Nagle Algorithm, errno = %i.\n", errno);
        return 4;
    }
    
    return 0;
    
}
//------------------------------------------------------------------------------------------------
void	C_Socket::CloseSocket()
{
    CloseFCStream();
    
    if(m_i32socketId!=-1)
        Close_Socket(m_i32socketId);
    
    m_i32socketId = -1;
}
//------------------------------------------------------------------------------------------------
void C_Socket::setSocketId( int socketFd )
{
    m_i32socketId = socketFd;
}
//------------------------------------------------------------------------------------------------
int C_Socket::getSocketId()
{
    return m_i32socketId;
}
//------------------------------------------------------------------------------------------------
// C_TcpSocket
//------------------------------------------------------------------------------------------------
C_TcpSocket::C_TcpSocket():
m_i32StreamSize(0),
m_i32StreamIndex(0),
m_DataCallBack(NULL)
{
    
}
//------------------------------------------------------------------------------------------------
C_TcpSocket::~C_TcpSocket()
{
    
}
//------------------------------------------------------------------------------------------------
int C_TcpSocket::sendStream(BYTE* message ,int size)
{
    int numBytes;  // the number of bytes sent
    
    numBytes = (int)send(m_i32socketId,message,size,0);
    
    if(numBytes<=0)
        return  Error_Socket_Broken;
    
    if(m_DataCallBack)
        m_DataCallBack(true,numBytes,message);
    
    return numBytes;
}
//---------------------------------------------------------------------------------------------
int C_TcpSocket::readStream(BYTE * message , int i32Size)
{
    int i32Left = m_i32StreamSize - m_i32StreamIndex;
    int i32Read = 0;
    
    if(i32Size > i32Left)
    {
        if(i32Left>0)
        {
            memcpy(message,&m_abyStreamBuffer[m_i32StreamIndex],i32Left);
            i32Size-=i32Left;
            i32Read+=i32Left;
        }
        int numBytes,i32RealRead = 0;
        
        do {
            
            int i32Retry = DEFAULT_READ_TIMEOUT / DEFAULT_READRETRY_TIMEOUT;
            int i32Ret = 0;
            
            do
            {
                
                if(m_i32socketId==-1)
                {
                    DEBUG_PRINT("Failed to recv(), errno = %i.\n", errno);
                    return Error_Socket_Broken;
                }
                
                fd_set read_fd_set;
                FD_ZERO(&read_fd_set);
                FD_SET((unsigned int)m_i32socketId, &read_fd_set);
                
                struct timeval timeout;
                timeout.tv_sec = DEFAULT_READRETRY_TIMEOUT;
                timeout.tv_usec = 0;
                
                i32Ret = select(m_i32socketId+1, &read_fd_set, NULL, NULL, &timeout);
                i32Retry--;
                
                if(i32Ret == 0)
                {
                    if(errno == ENOTCONN || errno == ENETDOWN || errno == ENETUNREACH || errno == ENETRESET|| errno == EBADF)
                    {
                        DEBUG_PRINT("Failed to recv(), errno = %i.\n", errno);
                        return Error_Socket_Broken;
                    }
                    else
                        DEBUG_PRINT("select timeout, errno = %i.\n", errno);
                }
                else if(i32Ret<0)
                {
                    DEBUG_PRINT("Failed to recv(), errno = %i.\n", errno);
                    return Error_Socket_Broken;
                }
                
            }while(i32Ret==0 && i32Retry>0);
            
            if(i32Ret == 0)
            {
                DEBUG_PRINT("recv() timed out, errno = %i.\n",errno);
                return Error_Socket_Timeout;
            }
            
            if(i32RealRead >= STREAM_MAX_BUFF)
            {
                DEBUG_PRINT("Over buffer size! Try to read %d ",i32RealRead + numBytes);
                return Error_Socket_Broken;
            }
            
            numBytes = (int)recv(m_i32socketId,&m_abyStreamBuffer[i32RealRead],STREAM_MAX_BUFF - i32RealRead,0);
            
            if (numBytes <= 0)
            {
                DEBUG_PRINT("Failed to recv(), size = %d , errno = %i.\n",numBytes, errno);
                return Error_Socket_Broken;
            }
            
            
            if(m_DataCallBack)
                m_DataCallBack(false,numBytes,&m_abyStreamBuffer[i32RealRead]);
            
            i32RealRead += numBytes;
        }while(i32RealRead < i32Size);
        
        
        m_i32StreamSize = i32RealRead;
        m_i32StreamIndex = 0;
    }
    
    
    memcpy(&message[i32Read],&m_abyStreamBuffer[m_i32StreamIndex],i32Size);
    m_i32StreamIndex+=i32Size;
    i32Read+=i32Size;
    
    return i32Read;
}
//------------------------------------------------------------------------------------------------
//	C_TcpClient class functions
//------------------------------------------------------------------------------------------------
static void MyWriteStreamClientCallBack( CFWriteStreamRef stream, CFStreamEventType type, void *clientCallBackInfo )
{

    C_TcpClient *pClient = (C_TcpClient*) clientCallBackInfo;

    switch (type)
    {
        case kCFStreamEventEndEncountered:
        {
            NSLog(@"in CallBack: kCFStreamEventEndEncounteredd");
            CFWriteStreamClose(stream);
            break;
        }
        case kCFStreamEventErrorOccurred:
        {
            NSLog(@"in CallBack: kCFStreamEventErrorOccurred");
            break;
            
        }
        case kCFStreamEventCanAcceptBytes:
        {
            NSLog(@"in CallBack: kCFStreamEventCanAcceptBytes");
            break;
            
        }
        case kCFStreamEventNone:
        {
            NSLog(@"in CallBack: kCFStreamEventNone");
            break;
        }
        case kCFStreamEventOpenCompleted:
        {
            NSLog(@"in CallBack: kCFStreamEventOpenCompleted");
            
            CFDataRef socketWData = (CFDataRef)CFWriteStreamCopyProperty(stream, kCFStreamPropertySocketNativeHandle);
            
            CFSocketNativeHandle handle;
            CFDataGetBytes(socketWData, CFRangeMake(0, sizeof(CFSocketNativeHandle)), (UInt8 *)&handle);
            pClient->setSocketId(handle);
            CFRelease(socketWData);
            
            break;
        }
    }
}
//------------------------------------------------------------------------------------------------
int C_TcpClient::connectToServer(LPCTSTR pszIPAddress,int i32PortNum,int i32TimeOut)
{
	int i32Ret = 0;

	ClearBuffer();
    CloseSocket();
    m_i32socketId = 0;
    
    NSString *ip = [NSString stringWithFormat:@"%s", pszIPAddress];
    
    static const CFOptionFlags kWriteNetworkEvents =    kCFStreamEventEndEncountered | kCFStreamEventErrorOccurred |
                                                        kCFStreamEventCanAcceptBytes | kCFStreamEventOpenCompleted |
                                                        kCFStreamEventNone;
    
    CFStreamClientContext ctxt = {0,(void*)this,NULL,NULL,NULL};
    CFStreamCreatePairWithSocketToHost(NULL, (__bridge CFStringRef)ip, i32PortNum, NULL, &g_writeStream);
    CFWriteStreamSetClient(g_writeStream, kWriteNetworkEvents, MyWriteStreamClientCallBack, &ctxt);
    
    CFWriteStreamScheduleWithRunLoop(g_writeStream, CFRunLoopGetMain(), kCFRunLoopCommonModes);
    CFWriteStreamOpen(g_writeStream);
    
    struct timeval PrevTv,curTV;
    gettimeofday(&PrevTv,NULL);
    
    timeval   timeout = { 0 , 200000 } ;

    while(m_i32socketId==0)
    {
        gettimeofday(&curTV,NULL);
        if(curTV.tv_sec - PrevTv.tv_sec >(i32TimeOut / 1000))
        {
            CFWriteStreamSetClient(g_writeStream, kCFStreamEventNone, NULL, NULL);
            CFWriteStreamClose(g_writeStream);
            CFWriteStreamUnscheduleFromRunLoop(g_writeStream, CFRunLoopGetMain(), kCFRunLoopCommonModes);
            g_writeStream = NULL;
            break;
        }
        
        select(0, NULL, NULL, NULL, &timeout);
    }
    
    if(m_i32socketId<=0)
        return -1;
    
	return i32Ret;
}

