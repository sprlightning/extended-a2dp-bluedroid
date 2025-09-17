obj-y := audioplayers/ common/ main/ key/ pwl/ battery/ factory/ cmd/ mic/ audio_test/
ifeq ($(FJH_APPS),1)
obj-y += fjh_apps/
endif

ifeq ($(FJH_BOX_MODULE),1)
obj-y += box_2/
endif

ifeq ($(APP_TEST_AUDIO),1)
obj-y += apptester/
endif

ifeq ($(BTUSB_AUDIO_MODE),1)
obj-y += usbaudio/
endif
ifeq ($(BT_USB_AUDIO_DUAL_MODE),1)
obj-y += btusbaudio/
obj-y += usbaudio/
endif

ifeq ($(APP_TEST_SDMMC),1)
obj-y += sdmmc/
endif

ifeq ($(ANC_APP),1)
obj-y += anc/
endif

ifeq ($(ANC_ASSIST_ENABLED),1)
obj-y += voice_assist/
endif

ifeq ($(VOICE_DETECTOR_EN),1)
obj-y += voice_detector/
endif

ifeq ($(BOX_DET_FJH),1)
obj-y += box_det/
endif

ifeq ($(SNDP_TX_3MIC_ENABLE),1)
obj-y += sp_ec_mv03/
endif

ifeq ($(JS1228_SENSOR),1)
obj-y += jsa1228/
endif

ifeq ($(EAR_SENSOR),1)
obj-y += earsensor/
endif

ifeq ($(NDT_SENSOR),1)
obj-y += ndtsensor/
endif

ifeq ($(LONGWEI),1)
obj-y +=sp_audio/
endif

subdir-ccflags-y += -Iapps/apptester \
					-Iapps/audioplayers \
					-Iapps/anc/inc \
					-Iapps/voice_assist/inc \
					-Iapps/common \
					-Iapps/sdmmc \
					-Iapps/main \
					-Iapps/cmd \
					-Iapps/key \
					-Iapps/pwl \
					-Iapps/battery \
					-Iservices/voicepath \
					-Iservices/audio_dump/include \
					-Iservices/ble_app/app_datapath \
					-Iservices/bt_app/a2dp_codecs/include \
					-Iutils/list \
					-Iutils/heap \
					-Iservices/multimedia/audio/process/filters/include
ifeq ($(FJH_BOX_MODULE),1)
subdir-ccflags-y += -Iapps/box_2
endif	

ifeq ($(FJH_APPS),1)
subdir-ccflags-y += -Iapps/fjh_apps
endif		

ifeq ($(JS1228_SENSOR),1)
subdir-ccflags-y += -Iapps/jsa1228
endif		

ifeq ($(EAR_SENSOR),1)
subdir-ccflags-y += -Iapps/earsensor
endif	

ifeq ($(NDT_SENSOR),1)
subdir-ccflags-y += -Iapps/ndtsensor
endif		
ifeq ($(BT_USB_AUDIO_DUAL_MODE),1)
subdir-ccflags-y += -Iapps/btusbaudio
endif

ifeq ($(SNDP_TX_3MIC_ENABLE),1)
subdir-ccflags-y += -Iapps/sp_ec_mv03
endif
ifeq ($(A2DP_LDAC_ON),1)
subdir-ccflags-y += -Ithirdparty/audio_codec_lib/ldac/inc
endif

ifeq ($(BOX_DET_FJH),1)
subdir-ccflags-y += -Iapps/box_det
endif
