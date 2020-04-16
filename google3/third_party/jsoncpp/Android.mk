LOCAL_PATH      := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE        := jsoncpp
LOCAL_SRC_FILES     := json_reader.cc \
                       json_value.cc \
                       json_writer.cc

LOCAL_C_INCLUDES  += $(APP_GOOGLE3_PATH)

LOCAL_CPP_EXTENSION := .cc

LOCAL_CPPFLAGS := -DNO_LOCALE_SUPPORT

include $(BUILD_STATIC_LIBRARY)