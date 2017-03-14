
#编译时到src目录，执行以下命令即可编译,只能不带APP_BUILD_SCRIPT路径参数才可以用上default.properties 来改变平台
# ndk-build
# ndk-build APP_BUILD_SCRIPT=./jni/Android.mk NDK_PROJECT_PATH=../android NDK_DEBUG=0 V=1
#TARGET_PLATFORM 试过无效果，只有通过AndroidManifest.xml,default.properties 来控制使用平台版本
#TARGET_PLATFORM = android-19


#LOCAL_PATH := $(call my-dir)
LOCAL_PATH := ../../..


include $(CLEAR_VARS)
LOCAL_MODULE :=update_android.e
MY_SPATH_1 := $(LOCAL_PATH)/comm/src/libutil
MY_SPATH_2 := $(LOCAL_PATH)/comm/src/update

#
MY_SRC_LIST := $(wildcard  $(MY_SPATH_1)/*.cpp)
MY_SRC_LIST += $(wildcard  $(MY_SPATH_2)/*.cpp)
#

LOCAL_C_INCLUDES := $(MY_SPATH_1) $(MY_SPATH_2)
LOCAL_CPPFLAGS += -Wall -Werror -DNDEBUG -DLINUX32 -D_FILE_OFFSET_BITS=64 -DSM_DBG -DSM_MODIFY
LOCAL_SRC_FILES := $(MY_SRC_LIST:$(LOCAL_PATH)/%=%)

#
include $(BUILD_EXECUTABLE)


