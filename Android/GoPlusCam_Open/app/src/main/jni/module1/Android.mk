LOCAL_PATH := $(call my-dir)
TOP_PATH := $(call my-dir)
APP_ABI := armeabi-v7a arm64-v8a x86 x86_64
LOCAL_DISABLE_FATAL_LINKER_WARNINGS:=true

#ffmpeg core library
include $(CLEAR_VARS)
LOCAL_PATH := $(TOP_PATH)
LOCAL_MODULE := avfilter
LOCAL_CPPFLAGS := -fPIC
LOCAL_LDLIBS :=-fPIC
LOCAL_SRC_FILES := $(LOCAL_PATH)/jniLibs/$(TARGET_ARCH_ABI)/lib/libavfilter.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_PATH := $(TOP_PATH)
LOCAL_MODULE := avformat
LOCAL_LDLIBS :=-fPIC
LOCAL_SRC_FILES := $(LOCAL_PATH)/jniLibs/$(TARGET_ARCH_ABI)/lib/libavformat.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_PATH := $(TOP_PATH)
LOCAL_MODULE := avcodec
LOCAL_CPPFLAGS := -fPIC
LOCAL_LDLIBS :=-fPIC
LOCAL_SRC_FILES := $(LOCAL_PATH)/jniLibs/$(TARGET_ARCH_ABI)/lib/libavcodec.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_PATH := $(TOP_PATH)
LOCAL_MODULE := swresample
LOCAL_LDLIBS :=-fPIC
LOCAL_SRC_FILES := $(LOCAL_PATH)/jniLibs/$(TARGET_ARCH_ABI)/lib/libswresample.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_PATH := $(TOP_PATH)
LOCAL_MODULE := swscale
LOCAL_LDLIBS :=-fPIC
LOCAL_CPPFLAGS := -fPIC
LOCAL_SRC_FILES := $(LOCAL_PATH)/jniLibs/$(TARGET_ARCH_ABI)/lib/libswscale.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_PATH := $(TOP_PATH)
LOCAL_MODULE := avdevice
LOCAL_LDLIBS :=-fPIC
LOCAL_CPPFLAGS := -fPIC
LOCAL_SRC_FILES := $(LOCAL_PATH)/jniLibs/$(TARGET_ARCH_ABI)/lib/libavdevice.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_PATH := $(TOP_PATH)
LOCAL_MODULE := avutil
LOCAL_LDLIBS :=-fPIC
LOCAL_CPPFLAGS := -fPIC
LOCAL_SRC_FILES := $(LOCAL_PATH)/jniLibs/$(TARGET_ARCH_ABI)/lib/libavutil.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_PATH := $(TOP_PATH)
LOCAL_MODULE := postproc
LOCAL_SRC_FILES := $(LOCAL_PATH)/jniLibs/$(TARGET_ARCH_ABI)/lib/libpostproc.a
LOCAL_LDLIBS :=-fPIC
LOCAL_CPPFLAGS := -fPIC
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_PATH := $(TOP_PATH)
LOCAL_MODULE := x264
LOCAL_CPPFLAGS := -fPIC
LOCAL_SRC_FILES := $(LOCAL_PATH)/jniLibs/$(TARGET_ARCH_ABI)/lib/libx264.a
LOCAL_LDLIBS :=-fPIC
include $(PREBUILT_STATIC_LIBRARY)


#ffmpeg
include $(CLEAR_VARS)
LOCAL_PATH := $(TOP_PATH)
LOCAL_MODULE    := ffmpeg
LOCAL_CPPFLAGS := -fPIC
LOCAL_SRC_FILES := $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/*.cpp))
LOCAL_STATIC_LIBRARIES := avfilter avformat avcodec swresample swscale avdevice avutil postproc x264
LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -fPIC\
                -llog -landroid -lOpenSLES -lEGL -lGLESv2 -ljnigraphics -lz


LOCAL_C_INCLUDES := $(LOCAL_PATH)
include $(BUILD_SHARED_LIBRARY)

