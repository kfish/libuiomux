LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES := \
	external/libuiomux/include \

#LOCAL_CFLAGS := -DDEBUG
LOCAL_CFLAGS := -DVERSION=\"1.0.0\"

LOCAL_SRC_FILES := \
	uio.c \
	uiomux.c

LOCAL_SHARED_LIBRARIES += libbinder libcutils libutils

LOCAL_MODULE := libuiomux
LOCAL_PRELINK_MODULE := false
include $(BUILD_SHARED_LIBRARY)
