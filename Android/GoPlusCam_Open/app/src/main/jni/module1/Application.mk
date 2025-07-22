APP_ABI := armeabi-v7a arm64-v8a x86 x86_64
APP_PLATFORM := android-14
APP_OPTIM := debug
APP_CPPFLAGS += -std=gnu++11 -fexceptions -Wno-deprecated-declarations -Wno-nonportable-include-path -Wno-unused-result
APP_STL := c++_static
STLPORT_FORCE_REBUILD := true