//
//  EventDefine.h
//  ffmpegTest
//
//  Created by generalplus_sa1 on 2017/10/3.
//  Copyright © 2017年 generalplus_sa1. All rights reserved.
//

#ifndef EventDefine_h
#define EventDefine_h
//----------------------------------------------------------------------
typedef struct tag_Decode_AudioData
{
    uint8_t **pData ;
    int i32SampleCnt;
    int64_t i64SampleIndex;
    int i32SampleRate;
    int i32Channel;

}S_Decode_AudioData;
//----------------------------------------------------------------------
typedef enum
{                                           // Event Data
    E_EventType_Queue_Clear,                // No Data
    
    E_EventType_Audio_RampUp,               // No Data
    E_EventType_Audio_RampDown,             // No Data
    E_EventType_Audio_PlayedSample,         // int64_t
    
    E_EventType_Input_Packet,               // AVPacket

    E_EventType_Decode_DisplayFrame,        // AVFrame
    E_EventType_Decode_VideoFrame,          // AVFrame
    E_EventType_Decode_AudioData,           // S_Decode_AudioData
    
    E_EventType_Save_SnapshotComplete,      // No Data
    E_EventType_Save_VideoComplete,         // No Data
    
    E_EventType_Buffer_Start,               // No Data
    E_EventType_Buffer_Complete,            // No Data
    
    E_EventType_Player_Init_Failed,         // No Data
    E_EventType_Player_Init_Complete,       // No Data
    
    E_EventType_Player_Play,                // No Data
    E_EventType_Player_Stop,                // No Data
    E_EventType_Player_Seek,                // No Data
    E_EventType_Player_Pause,               // No Data

    E_EventType_Thread_End,                 // C_ThreadBase
    
}E_EventType;
//----------------------------------------------------------------------
/** Base abstract event functor class. All event functors must extend this interface.*/
template<typename EventType> class I_EventFunctorBase
{
public:
    virtual ~I_EventFunctorBase(){}
    virtual void* GetThis() = 0;
    virtual void Call(EventType& event) = 0;
};

/** Event functor class. */
template<class ClassName, typename EventType> class C_EventFunctor : public I_EventFunctorBase<EventType>
{
private:
    typedef void (ClassName::*EventHandler)(EventType&);
    ClassName* m_pThis;
    EventHandler m_pEventHandler;
public:
    C_EventFunctor(ClassName* pThis, EventHandler pHandler) : m_pThis(pThis), m_pEventHandler(pHandler) {}

    virtual void* GetThis() { return m_pThis; }
    virtual void Call(EventType& event) { if (m_pThis) (m_pThis->*m_pEventHandler)(event); }
};
//----------------------------------------------------------------------
#define SET_EVENT_HANDLER(EventType,ClassName,FunctionName) \
C_EventManager::GetEvnetManager()->RegisterEventSink(\
                                                     EventType,\
                                                     new C_EventFunctor<ClassName, C_Event>(this, &ClassName::FunctionName)\
                                                     );
//----------------------------------------------------------------------
class C_Event
{
public:
    C_Event(E_EventType Type,void *pData=NULL):
    m_Type(Type),m_pData(pData){}
    ~C_Event(){}
    
    E_EventType GetType() { return m_Type; }
    void*       GetData() { return m_pData;}
private:
    E_EventType m_Type;
    void*       m_pData;
};
//----------------------------------------------------------------------

#endif /* EventDefine_h */
