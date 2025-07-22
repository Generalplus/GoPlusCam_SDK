//
//  EventManager.hpp
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2017/10/3.
//  Copyright © 2017年 generalplus_sa1. All rights reserved.
//

#ifndef EventManager_hpp
#define EventManager_hpp

#include <pthread.h>
#include <map>
#include <vector>
#include "EventDefine.h"
//----------------------------------------------------------------------
class C_EventManager
{
public:
    C_EventManager();
    ~C_EventManager();
    
    static C_EventManager *GetEvnetManager();
    
    void RegisterEventSink(E_EventType eventType, I_EventFunctorBase<C_Event>* functor);
    void ProcessEvent(C_Event &event);
    
private:
    
    typedef std::vector< I_EventFunctorBase<C_Event>* >       EventSinksArray;
    typedef std::map<E_EventType, EventSinksArray >          EventSinksMap;
    
    EventSinksMap       m_EventSinks;
};



#endif /* EventManager_hpp */
