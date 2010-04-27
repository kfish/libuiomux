LOCAL_PATH:= $(call my-dir)

# noop
include $(CLEAR_VARS)
LOCAL_C_INCLUDES := external/libuiomux/include
LOCAL_CFLAGS := -DVERSION=\"1.0.0\"
LOCAL_SRC_FILES := noop.c
LOCAL_SHARED_LIBRARIES := libuiomux
LOCAL_MODULE := noop
include $(BUILD_EXECUTABLE)

# double-open
include $(CLEAR_VARS)
LOCAL_C_INCLUDES := external/libuiomux/include
LOCAL_CFLAGS := -DVERSION=\"1.0.0\"
LOCAL_SRC_FILES := double-open.c
LOCAL_SHARED_LIBRARIES := libuiomux
LOCAL_MODULE := double-open
include $(BUILD_EXECUTABLE)

# multiple-open
include $(CLEAR_VARS)
LOCAL_C_INCLUDES := external/libuiomux/include
LOCAL_CFLAGS := -DVERSION=\"1.0.0\"
LOCAL_SRC_FILES := multiple-open.c
LOCAL_SHARED_LIBRARIES := libuiomux
LOCAL_MODULE := multiple-open
include $(BUILD_EXECUTABLE)

# lock-unlock
include $(CLEAR_VARS)
LOCAL_C_INCLUDES := external/libuiomux/include
LOCAL_CFLAGS := -DVERSION=\"1.0.0\"
LOCAL_SRC_FILES := lock-unlock.c
LOCAL_SHARED_LIBRARIES := libuiomux
LOCAL_MODULE := lock-unlock
include $(BUILD_EXECUTABLE)

# fork
include $(CLEAR_VARS)
LOCAL_C_INCLUDES := external/libuiomux/include
LOCAL_CFLAGS := -DVERSION=\"1.0.0\"
LOCAL_SRC_FILES := fork.c
LOCAL_SHARED_LIBRARIES := libuiomux
LOCAL_MODULE := fork
include $(BUILD_EXECUTABLE)

# threads
include $(CLEAR_VARS)
LOCAL_C_INCLUDES := external/libuiomux/include
LOCAL_CFLAGS := -DVERSION=\"1.0.0\"
LOCAL_SRC_FILES := threads.c
LOCAL_SHARED_LIBRARIES := libuiomux
LOCAL_MODULE := threads
include $(BUILD_EXECUTABLE)

# fork-threads
include $(CLEAR_VARS)
LOCAL_C_INCLUDES := external/libuiomux/include
LOCAL_CFLAGS := -DVERSION=\"1.0.0\"
LOCAL_SRC_FILES := fork-threads.c
LOCAL_SHARED_LIBRARIES := libuiomux
LOCAL_MODULE := fork-threads
include $(BUILD_EXECUTABLE)

# exit-locked
include $(CLEAR_VARS)
LOCAL_C_INCLUDES := external/libuiomux/include
LOCAL_CFLAGS := -DVERSION=\"1.0.0\"
LOCAL_SRC_FILES := exit-locked.c
LOCAL_SHARED_LIBRARIES := libuiomux
LOCAL_MODULE := exit-locked
include $(BUILD_EXECUTABLE)

