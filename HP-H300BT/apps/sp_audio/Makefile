
LIB_NAME := lib_LISO_T603D_v02
 

cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))
obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.s))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

$(LIB_NAME)-y += $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)
obj-y := $(LIB_NAME).a



ccflags-y := \
	-Iplatform/hal \
	-Iservices/bt_app \
	-Iutils/list \
	-Iutils/heap \
	-Iservices/bt_profiles/inc \
	-Iservices/bt_if_enhanced/inc \
	-Iservices/nvrecord \
	-Iinclude/cmsis_dsp  \


	
	
