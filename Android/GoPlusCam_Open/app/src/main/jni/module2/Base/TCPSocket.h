#ifndef _GPTCPSOCKET_
#define _GPTCPSOCKET_

#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
//#include <sys/filio.h>
#include <stdio.h>
#include "CommandAgentDef.h"

#define STREAM_MAX_BUFF           (1024*128)
//------------------------------------------------------------------------------------------------
//  C_Socket class
//------------------------------------------------------------------------------------------------

class C_Socket
{

protected:
     	 
	int		m_i32socketId;				

public:

			C_Socket();
    virtual ~C_Socket();

	int     CreateSocket(sa_family_t sfamily);
    void	CloseSocket();
	void	setSocketId(int socketFd);
    int		getSocketId();
};

//------------------------------------------------------------------------------------------------
//  C_TcpSocket class 
//------------------------------------------------------------------------------------------------

class C_TcpSocket : public C_Socket
{
public:
			C_TcpSocket();
	virtual	~C_TcpSocket();

	int		sendStream(BYTE* ,int);
	int		readStream(BYTE* ,int);
    void    SetDataCallBack(PFN_SocketDataCallBack CallBack) { m_DataCallBack = CallBack; }
    void    ClearBuffer()
            {
                //Clear buffer
                m_i32StreamSize = 0;
                m_i32StreamIndex = 0;
            }
private:
    
    
    BYTE   m_abyStreamBuffer[STREAM_MAX_BUFF];
    int    m_i32StreamSize;
    int    m_i32StreamIndex;
    
    char   m_tszLogPath[256];
    PFN_SocketDataCallBack m_DataCallBack;
    
};
//------------------------------------------------------------------------------------------------
class C_TcpClient : public C_TcpSocket
{

public:	
			C_TcpClient(){}
	virtual	~C_TcpClient(){}

	// connect to the server
	virtual int connectToServer(
		LPCTSTR pszIPAddress,
		int i32PortNum,
		int i32TimeOut=DEFAULT_CONNECT_TIME //MicroSec
		); 

};

#endif
