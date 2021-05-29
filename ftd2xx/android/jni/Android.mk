LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libusb
LOCAL_SRC_FILES := libusb1.0.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libftdi
LOCAL_SRC_FILES := libftdi1.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := ../../driver.cpp ../../generator.cpp ../../meterfeeder.cpp

LOCAL_SHARED_LIBRARIES := libusb libftdi

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include

LOCAL_MODULE:= meterfeeder
include $(BUILD_SHARED_LIBRARY)

LOCAL_CFLAGS := -Wall -std=c++11
