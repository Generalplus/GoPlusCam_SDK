//
//  ThreadBase.cpp
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2017/10/3.
//  Copyright © 2017年 generalplus_sa1. All rights reserved.
//
#include "ThreadBase.h"
//----------------------------------------------------------------------
C_ThreadBase::C_ThreadBase(C_PlayerInfo *pPlayerInfo):
m_Thread(0),m_pPlayerInfo(pPlayerInfo),m_FinishJob(true)
{
    pthread_mutex_init(&m_ThreadLock, NULL);
}
//----------------------------------------------------------------------
C_ThreadBase::~C_ThreadBase()
{
    
}
//----------------------------------------------------------------------
bool C_ThreadBase::IsRunning()
{
    bool bRunning = false;
    
    if(m_Thread != 0)
        if(pthread_kill(m_Thread, 0) == 0)
            bRunning = true;
    
    return bRunning;
}
//----------------------------------------------------------------------
void*  C_ThreadBase::ThreadEntry(void *param)
{
    C_ThreadBase* pThread = (C_ThreadBase*)param;

    pThread->SetFinish(false);
    pThread->ThreadFunction();
    pThread->SetFinish(true);

    C_Event event(E_EventType_Thread_End,(void*)pThread);
    C_EventManager::GetEvnetManager()->ProcessEvent(event);
    
    return FFMPEGPLAYER_NOERROR;
}
//----------------------------------------------------------------------
int  C_ThreadBase::ThreadFunction()
{
    return FFMPEGPLAYER_NOERROR;
}
//----------------------------------------------------------------------
bool  C_ThreadBase::Start()
{
    bool bRet = true;
    
    if(pthread_mutex_trylock(&m_ThreadLock)==0) {
    
        if(IsRunning())
            bRet = false;
    
        if(bRet)
        {
            pthread_create(&m_Thread, NULL, ThreadEntry, this);
        }
        
        pthread_mutex_unlock(&m_ThreadLock);
    }
    else
        bRet = false;
    
    return bRet;
}
//----------------------------------------------------------------------
bool  C_ThreadBase::WaitFinish()
{
    pthread_mutex_lock(&m_ThreadLock);
    
    if(IsRunning())
    {
        void *ret = NULL;
        pthread_join(m_Thread, &ret);
        m_Thread = 0;
    }
    
    pthread_mutex_unlock(&m_ThreadLock);
    
    return true;
}
//----------------------------------------------------------------------
