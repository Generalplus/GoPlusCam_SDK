//
//  EventManager.cpp
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2017/10/3.
//  Copyright © 2017年 generalplus_sa1. All rights reserved.
//

#include "EventManager.h"
//----------------------------------------------------------------------
C_EventManager::C_EventManager()
{
    
}
//----------------------------------------------------------------------
C_EventManager::~C_EventManager()
{
    for (EventSinksMap::iterator mit = m_EventSinks.begin(); mit != m_EventSinks.end(); ++mit)
    {
        while (mit->second.size())
        {
            delete (*(mit->second.begin()));
            mit->second.erase(mit->second.begin());
        }
    }
}
//----------------------------------------------------------------------
C_EventManager* C_EventManager::GetEvnetManager()
{
    static C_EventManager *g_EventManager = NULL;
    if(!g_EventManager)
        g_EventManager = new C_EventManager();
    
    return g_EventManager;
}
//----------------------------------------------------------------------
void C_EventManager::RegisterEventSink(E_EventType eventType, I_EventFunctorBase<C_Event>* functor)
{
    m_EventSinks[eventType].push_back(functor);
}
//----------------------------------------------------------------------
void C_EventManager::ProcessEvent(C_Event &event)
{
    EventSinksMap::iterator mit = m_EventSinks.find(event.GetType());
    if (mit != m_EventSinks.end())
    {
        for (EventSinksArray::iterator it = mit->second.begin(); it != mit->second.end(); ++it)
            (*it)->Call(event);
    }
}
//----------------------------------------------------------------------

