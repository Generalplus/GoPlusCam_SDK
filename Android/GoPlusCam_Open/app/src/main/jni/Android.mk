PROJ_PATH	:= $(call my-dir)
APP_STL := c++_static
include $(CLEAR_VARS)
include $(PROJ_PATH)/module1/Android.mk
include $(PROJ_PATH)/module2/Android.mk


