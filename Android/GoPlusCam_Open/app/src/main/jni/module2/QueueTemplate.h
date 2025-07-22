//
//  QueueTemplate.h
//  ffmpegTest
//
//  Created by generalplus_sa1 on 7/6/16.
//  Copyright © 2016 generalplus_sa1. All rights reserved.
//

#ifndef QueueTemplate_h
#define QueueTemplate_h

#include <pthread.h>
#include <deque>
#include <sys/time.h>

#define QUEUE_WAIT_TIMEOUT_SEC    30

template <class CType>
class T_Queue
{
    typedef typename std::deque<CType*>           QueueVector;
    typedef typename std::deque<CType*>::iterator Queue_iterator;
    
public:
    T_Queue()  { InitLock(); }
    ~T_Queue() { ClearQueue(); }
    
    void UnLockQueue()
    {
        if(pthread_mutex_trylock(&m_Queuelock) == 0)
        {
            pthread_cond_signal(&m_Condition);
            pthread_mutex_unlock(&m_Queuelock);
        }
    }
    
    bool IsHaveWaiting()
    {
        bool bWaiting;
        
        pthread_mutex_lock(&m_Queuelock);
        bWaiting = m_bIsWaiting;
        pthread_mutex_unlock(&m_Queuelock);
        
        return bWaiting;
    }
    
    CType* PopObject(bool bWait = false)
    {
        CType* pObject = NULL;
     
        pthread_mutex_lock(&m_Queuelock);
        
        if(bWait && m_Queue.size()==0)
        {
            struct timespec timeToWait;
            struct timeval now;
            
            gettimeofday(&now,NULL);

            timeToWait.tv_sec = now.tv_sec + QUEUE_WAIT_TIMEOUT_SEC;
            timeToWait.tv_nsec = now.tv_usec;
            
            m_bIsWaiting = true;
            pthread_cond_timedwait(&m_Condition,&m_Queuelock,&timeToWait);
            m_bIsWaiting = false;
        }
        
        if(m_Queue.size()!=0 && !m_bIsClearing)
        {
            pObject = m_Queue.front();
            m_Queue.pop_front();
        }
        pthread_mutex_unlock(&m_Queuelock);
        
        
        return pObject;
    }
    
    void PushOject(CType* pObject)
    {
        pthread_mutex_lock(&m_Queuelock);
        m_Queue.push_back(pObject);
        if(m_bIsWaiting)
            pthread_cond_signal(&m_Condition);
        pthread_mutex_unlock(&m_Queuelock);
    }
    
    void ClearQueue()
    {
        pthread_mutex_lock(&m_Queuelock);
        m_bIsClearing = true;
        Queue_iterator iterator;
        for(iterator=m_Queue.begin(); iterator!=m_Queue.end(); ++iterator)
        {
            CType *pObject = (CType *)(*iterator);
            
            FreeObjectContent(pObject);
            delete pObject;
        }
        
        m_Queue.clear();
        m_bIsClearing = false;
        pthread_mutex_unlock(&m_Queuelock);
    }
    
    int GetSize() { return (int)m_Queue.size(); }
    
    virtual void FreeObjectContent(CType* pObject)
    {
        
    }
    
    
private:
    
    void InitLock()
    {
        pthread_mutex_init(&m_Queuelock, NULL);
        pthread_cond_init(&m_Condition, NULL);
        
        m_bIsWaiting = false;
        m_bIsClearing = false;
    }
    
    bool                m_bIsWaiting;
    bool                m_bIsClearing;
    QueueVector         m_Queue;
    pthread_mutex_t     m_Queuelock;
    pthread_cond_t      m_Condition;
};



#endif /* QueueTemplate_h */
