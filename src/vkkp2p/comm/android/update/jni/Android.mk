
#����ʱ��srcĿ¼��ִ����������ɱ���,ֻ�ܲ���APP_BUILD_SCRIPT·�������ſ�������default.properties ���ı�ƽ̨
# ndk-build
# ndk-build APP_BUILD_SCRIPT=./jni/Android.mk NDK_PROJECT_PATH=../android NDK_DEBUG=0 V=1
#TARGET_PLATFORM �Թ���Ч����ֻ��ͨ��AndroidManifest.xml,default.properties ������ʹ��ƽ̨�汾
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


