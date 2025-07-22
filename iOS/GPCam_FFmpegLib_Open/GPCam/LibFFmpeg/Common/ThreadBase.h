//
//  ThreadBase.hpp
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2017/10/3.
//  Copyright © 2017年 generalplus_sa1. All rights reserved.
//

#ifndef ThreadBase_hpp
#define ThreadBase_hpp

#include "CodecDefine.h"
#include "EventManager.h"
#include "PlayerInfo.h"
//----------------------------------------------------------------------
class C_ThreadBase
{
public:
    C_ThreadBase(C_PlayerInfo *pPlayerInfo);
    ~C_ThreadBase();
    
    virtual bool Start();
    virtual bool WaitFinish();
    virtual bool IsFinish() { return m_FinishJob; }
    
protected:
    
    static void* ThreadEntry(void *param);
    virtual int ThreadFunction();
    bool IsRunning();
    void SetFinish(bool bFinish) { m_FinishJob = bFinish; }
    
    void CheckAndDoPause()
    {
        while(m_pPlayerInfo->GetPlayRate() == PAUSE_PLAYRATE &&
              !m_pPlayerInfo->GetSeekInfo()->bSeeking &&
              m_pPlayerInfo->GetStatus() == E_PlayerStatus_Playing)
            DoDelay(PAUSE_DEALY);
    }
    
    void ThreadDelay(int64_t delayTime)
    {
        if(delayTime > 0)
        {
            if(delayTime > AV_TIME_BASE)
                delayTime = AV_TIME_BASE;
            
            int64_t timeTarget = GetClock() + delayTime;
            int64_t sleepUnit =  SYNC_DURATION  > delayTime ? delayTime : SYNC_DURATION ;
            
            while(!m_pPlayerInfo->GetSeekInfo()->bAssignSeek &&
                  !m_pPlayerInfo->GetSeekInfo()->bSeeking &&
                  timeTarget>GetClock() &&
                  m_pPlayerInfo->GetStatus()== E_PlayerStatus_Playing
                  )
            {
                DoDelay(sleepUnit);
            }
        }
    }
    
    void UpdateAvgThreadTime(int64_t NewTime,int64_t *aAvgTime)
    {
        if(*aAvgTime == 0)
            *aAvgTime = NewTime;
        
        *aAvgTime += NewTime;
        *aAvgTime /= 2;
    }
    
    pthread_t           m_Thread;
    pthread_mutex_t     m_ThreadLock;
    C_PlayerInfo        *m_pPlayerInfo;
    bool                m_FinishJob;
};


#endif /* ThreadBase_hpp */
