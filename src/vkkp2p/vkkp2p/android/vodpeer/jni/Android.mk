
#编译时到src目录，执行以下命令即可编译,只能不带APP_BUILD_SCRIPT路径参数才可以用上default.properties 来改变平台
# ndk-build
# ndk-build APP_BUILD_SCRIPT=./jni/Android.mk NDK_PROJECT_PATH=../android NDK_DEBUG=0 V=1
#TARGET_PLATFORM 试过无效果，只有通过AndroidManifest.xml,default.properties 来控制使用平台版本
#TARGET_PLATFORM = android-19


#LOCAL_PATH := $(call my-dir)
LOCAL_PATH := ../../..


include $(CLEAR_VARS)
LOCAL_MODULE :=vkkp2p_android.e
MY_SPATH_1 := $(LOCAL_PATH)/comm/src/libutil
MY_SPATH_2 := $(LOCAL_PATH)/comm/src/libhttpsvr
MY_SPATH_3 := $(LOCAL_PATH)/comm/src/libcomm
MY_SPATH_4 := $(LOCAL_PATH)/comm/src/libuac
MY_SPATH_5 := $(LOCAL_PATH)/vkkp2p/src/libprotocol
MY_SPATH_6 := $(LOCAL_PATH)/vkkp2p/src/libpeer
MY_SPATH_7 := $(LOCAL_PATH)/vkkp2p/src/vodpeer

#
MY_SRC_LIST := $(wildcard  $(MY_SPATH_1)/*.cpp)
MY_SRC_LIST += $(wildcard  $(MY_SPATH_2)/*.cpp)
MY_SRC_LIST += $(wildcard  $(MY_SPATH_3)/*.cpp)
MY_SRC_LIST += $(wildcard  $(MY_SPATH_4)/*.cpp)
MY_SRC_LIST += $(wildcard  $(MY_SPATH_5)/*.cpp)
MY_SRC_LIST += $(wildcard  $(MY_SPATH_6)/*.cpp)
MY_SRC_LIST += $(wildcard  $(MY_SPATH_7)/*.cpp)
#

LOCAL_C_INCLUDES := $(MY_SPATH_1) $(MY_SPATH_2) $(MY_SPATH_3) $(MY_SPATH_4) $(MY_SPATH_5) $(MY_SPATH_6) $(MY_SPATH_7)
LOCAL_C_INCLUDES += \
	 $(JNI_H_INCLUDE)
LOCAL_CPPFLAGS += -Wall -Werror -DNDEBUG -DLINUX32 -D_FILE_OFFSET_BITS=64 -D_ANDROID -DSM_DBG -DSM_MODIFY
LOCAL_SRC_FILES := $(MY_SRC_LIST:$(LOCAL_PATH)/%=%)

#
include $(BUILD_EXECUTABLE)


