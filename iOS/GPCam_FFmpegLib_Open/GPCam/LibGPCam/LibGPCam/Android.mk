LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := GPCam
LOCAL_SRC_FILES := generalplus_GPCam_LibGPCam.cpp \
                   GPCamCommandAPI.cpp \
                   VideoCommandAgent.cpp \
                   VideoPacketParser.cpp \
                   VideoSteamCommmad.cpp \
                   VideoStreamCommandBase.cpp \
                   Base/CommandAgent.cpp \
                   Base/PacketParser.cpp \
                   Base/StreamCommandBase.cpp \
                   Base/TCPSocket.cpp

LOCAL_STATIC_LIBRARIES := GP_ComAir6Lib
LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog
LOCAL_C_INCLUDES := $(LOCAL_PATH) \
					$(LOCAL_PATH)/Base


include $(BUILD_SHARED_LIBRARY)
