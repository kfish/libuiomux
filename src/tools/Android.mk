LOCAL_PATH:= $(call my-dir)

# shveu-convert
include $(CLEAR_VARS)
LOCAL_C_INCLUDES := external/libuiomux/include
LOCAL_CFLAGS := -DVERSION=\"1.0.0\"
LOCAL_SRC_FILES := uiomux.c uiomux-alloc.c
LOCAL_SHARED_LIBRARIES := libuiomux
LOCAL_MODULE := uiomux
include $(BUILD_EXECUTABLE)
