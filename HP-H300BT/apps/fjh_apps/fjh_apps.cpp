/***************************************************************************
 *
 * Copyright 2015-2019 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#ifdef __FJH_BOX_MODULE__
//#include "app_box.h"
//#include "hds_sys_status_manage.h"
//#include "hds_apply_thread.h"
#endif 
#include "stdio.h"
#include "cmsis_os.h"
#include "list.h"
#include "string.h"
#include "nvrecord_env.h"

#include "hwtimer_list.h"

#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_bootmode.h"

#include "audioflinger.h"
#include "apps.h"
#include "fjh_apps.h"
#include "app_thread.h"
#include "app_key.h"
#ifdef __PC_CMD_UART__
#include "app_cmd.h"
#endif
#include "app_pwl.h"
#include "app_audio.h"
#include "app_overlay.h"
#include "app_battery.h"
#include "app_utils.h"
#include "app_status_ind.h"
#ifdef __FACTORY_MODE_SUPPORT__
#include "app_factory.h"
#include "app_factory_bt.h"
#endif
#include "bt_drv_interface.h"
#include "besbt.h"
#include "norflash_api.h"
#include "nvrecord.h"
#include "nvrecord_dev.h"
#include "nvrecord_env.h"
#include "crash_dump_section.h"
#include "log_section.h"
#include "factory_section.h"

#include "a2dp_api.h"
#include "me_api.h"
#include "os_api.h"
#include "btapp.h"
#include "app_bt.h"
#include "app_ble_include.h"
#include "bt_if.h"
#ifdef __INTERACTION__
#include "app_interaction.h"
#endif
#ifdef GSOUND_OTA_ENABLED
#include "gsound_ota.h"
#endif

#ifdef BES_OTA_BASIC
#include "ota_bes.h"
#endif
#include "pmu.h"
#ifdef MEDIA_PLAYER_SUPPORT
#include "resources.h"
#include "app_media_player.h"
#endif
#include "app_bt_media_manager.h"
#include "hal_sleep.h"
#ifdef VOICE_DATAPATH
#include "app_voicepath.h"
#endif
#ifdef BT_USB_AUDIO_DUAL_MODE
#include "btusb_audio.h"
#include "usbaudio_thread.h"
#endif

#if defined(IBRT)
#include "app_ibrt_if.h"
#include "app_ibrt_customif_ui.h"
#include "app_ibrt_ui_test.h"
#include "app_ibrt_voice_report.h"
#include "app_ibrt_rssi.h"
#endif

#ifdef APP_TRACE_RX_ENABLE
#include "app_trace_rx.h"
#endif

#ifdef GFPS_ENABLED
#include "app_gfps.h"
#endif

#ifdef PROMPT_IN_FLASH
#include "nvrecord_prompt.h"
#endif

#ifdef ANC_APP
#include "app_anc.h"
#endif

#ifdef AUDIO_DEBUG_V0_1_0
extern "C" int speech_tuning_init(void);
#endif

#if defined(BTUSB_AUDIO_MODE)
extern "C"  bool app_usbaudio_mode_on(void) ;
#endif

#ifdef BES_OTA_BASIC
extern "C" void ota_flash_init(void);
#endif

#ifdef BTIF_BLE_APP_DATAPATH_SERVER
#include "app_ble_cmd_handler.h"
#endif

#if defined(ANC_ASSIST_ENABLED)
#include "app_anc_assist.h"
#include "app_voice_assist_ai_voice.h"
#include "app_voice_assist_wd.h"
#include "app_voice_assist_pilot_anc.h"
#include "app_voice_assist_ultrasound.h"
#include "app_voice_assist_custom_leak_detect.h"
#include "app_voice_assist_prompt_leak_detect.h"
#if defined(AUDIO_ADAPTIVE_EQ)
#include "app_voice_assist_adaptive_eq.h"
#endif
#include "app_voice_assist_noise_adapt_anc.h"
#endif

#ifdef AUDIO_OUTPUT_DC_AUTO_CALIB
//#include "codec_calib.h"
#endif

#ifdef ANC_APP
#include "app_anc.h"
#endif

#ifdef __AI_VOICE__
#include "ai_manager.h"
#include "ai_thread.h"
#include "ai_control.h"
#include "ai_spp.h"
#include "app_ai_voice.h"
#endif

#include "gapm_task.h"
#include "app_ble_mode_switch.h"

#ifdef __THIRDPARTY
#include "app_thirdparty.h"
#endif
#include "app_ibrt_rssi.h"
#include "watchdog/watchdog.h"
#include "app_tota.h"
#include "app_ibrt_nvrecord.h"
#include "app_tws_if.h"
#include "fjh_apps_thread.h"
#include "app_ibrt_customif_cmd.h"

#include "app_box2.h"

#ifdef __TWSPAIRLISTNV__
#include "fjh_section.h"
#endif

#ifdef __BLEAPPKEY__
#include "bleapp_section.h"
#endif

#include "app_hfp.h"


#ifdef __CLOSEBOX_TWS_SWITCH_PATCH__
bool twsswitchok_enterclosebox = false;
#endif


#ifdef __LINKLOSS_NO_CONNECT_TONE__
bool linklossnoconnectedtoneflg = false;
#endif


bool boxopenbugflg = false;

#ifdef __BLE_S80__
extern "C" u16 check_battery(uint16_t battery_val);
#endif

#ifdef __ANC_SWITCH_DELAY_FJH__
#define anc_tone_off_timetest (MS_TO_TICKS(500 * 3))
app_anc_mode_t mode_fjh = APP_ANC_MODE_OFF;
static HWTIMER_ID anc_tonetimerid = NULL;
static uint32_t anc_led_request = 0;
#endif

fjh_apps_data apps_data;



#ifdef __FJH_SLAVER_BUG__
static void slaverbug_delay_timehandler(void const * param);
osTimerDef(SLAVERBUG_GENSORDELAY, slaverbug_delay_timehandler);
static osTimerId       slaverbug_delay_timer = NULL;
bool slaver_reconectflg = false;
static void slaverbug_delay_timehandler(void const * param)
{
     apps_data.slaver_reconectflg = false;
}


void slaverbug_delay_set_time(uint32_t millisec)
{
    if (slaverbug_delay_timer == NULL)
    {
	   slaverbug_delay_timer	= osTimerCreate(osTimer(SLAVERBUG_GENSORDELAY), osTimerOnce, NULL);
	   osTimerStop(slaverbug_delay_timer);
    } 
       osTimerStart(slaverbug_delay_timer, millisec);
}

void slaverbug_delay_close_time(void)
{
    if (slaverbug_delay_timer != NULL)
    {
	   osTimerStop(slaverbug_delay_timer);
    } 
	apps_data.slaver_reconectflg = false;
}
#endif




static void bug_delay_timehandler(void const * param);
osTimerDef(BUG_GENSORDELAY, bug_delay_timehandler);
static osTimerId      bug_delay_timer = NULL;
void bug_delay_set_time(uint32_t millisec);

static void bug_delay_timehandler(void const * param)
{
return;
#ifdef __JS1228_SENSOR__
ibrt_ctrl_t *p_ibrt_ctrl = app_ibrt_if_get_bt_ctrl_ctx(); 
#else
//ibrt_ctrl_t *p_ibrt_ctrl = app_ibrt_if_get_bt_ctrl_ctx(); 
#endif
#if 0
if(apps_data.bugflg == true)
{

if(app_tws_ibrt_tws_link_connected())
{
#if 0
        #ifdef ANC_APP
	    app_anc_sync_status();
        #endif
	    if (p_ibrt_ctrl->current_role == IBRT_MASTER &&
			app_ibrt_ui_get_enter_pairing_mode())
		{
		    if(app_status_indication_get()!=APP_STATUS_INDICATION_BOTHSCAN)
			{
			   app_voice_report(APP_STATUS_INDICATION_BOTHSCAN, 0);
			   app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);
			}
			app_start_10_second_timer(APP_POWEROFF_TIMER_ID);
		}
		if(p_ibrt_ctrl->current_role == IBRT_MASTER)
		{
			//app_voice_report(APP_STATUS_INDICATION_CONNECTED, 0);
		}
		else if(p_ibrt_ctrl->current_role == IBRT_SLAVE)
		{
			 app_stop_10_second_timer(APP_POWEROFF_TIMER_ID); 
			 app_status_indication_set(APP_STATUS_INDICATION_INITIAL);
		}
#endif		
		return;
}


	

    apps_data.bugtime++;
	if(apps_data.bugtime>20)
	{
		app_ibrt_ui_set_enter_pairing_mode(IBRT_MOBILE_CONNECTED);
        #if 1
		app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);
        #ifdef MEDIA_PLAYER_SUPPORT
		app_voice_report(APP_STATUS_INDICATION_BOTHSCAN,0);
        #endif	  
		app_tws_ibrt_set_access_mode(BTIF_BAM_GENERAL_ACCESSIBLE);
        #endif	
		app_ibrt_ui_set_enter_pairing_mode(IBRT_MOBILE_CONNECTED);
	}
	else
	{
        bug_delay_set_time(1000);
	}
}
#endif
#ifdef __JS1228_SENSOR__
//Á¬½ÓÊÖ»úÖ®Ç°Èë¶ú£¬ÐèÔÙ´ÎÈÃ¶ú»úµ×²ãÈë¶ú.........
if(get_inoutbox_status()== false)
{
  if(apps_data.earinpatchflg == true)
  {
	 if(p_ibrt_ctrl->current_role == IBRT_MASTER)
	 {
		 if(app_tws_ibrt_mobile_link_connected())
		 {
			  //app_ibrt_if_event_entry(IBRT_WEAR_UP_EVENT);
			  send_key_event(HAL_KEY_CODE_PWR,HAL_KEY_EVENT_EARIN);
		 }
	 }
	 else if(p_ibrt_ctrl->current_role == IBRT_SLAVE)
	 {
		 if(app_tws_ibrt_slave_ibrt_link_connected())
		 {
			  //app_ibrt_if_event_entry(IBRT_WEAR_UP_EVENT);
			  send_key_event(HAL_KEY_CODE_PWR,HAL_KEY_EVENT_EARIN);
		 }
	 }
	 apps_data.earinpatchflg = false;
   }
}

#else
/*
if(get_inoutbox_status()== false)
{
	 if(p_ibrt_ctrl->current_role == IBRT_MASTER)
	 {
		 if(app_tws_ibrt_mobile_link_connected())
		 {
			  app_ibrt_if_event_entry(IBRT_WEAR_UP_EVENT);
		 }
	 }
	 else if(p_ibrt_ctrl->current_role == IBRT_SLAVE)
	 {
		 if(app_tws_ibrt_slave_ibrt_link_connected())
		 {
			  app_ibrt_if_event_entry(IBRT_WEAR_UP_EVENT);
		 }
	 }
	 apps_data.boxinoutpatchflg = false;
}
*/
#endif

}


void bug_delay_set_time(uint32_t millisec)
{
    if (bug_delay_timer == NULL)
    {
	   bug_delay_timer	= osTimerCreate(osTimer(BUG_GENSORDELAY), osTimerOnce, NULL);
	   osTimerStop(bug_delay_timer);
    } 
       osTimerStart(bug_delay_timer, millisec);
}

void bug_delay_close_time(void)
{
    if (bug_delay_timer != NULL)
    {
	   osTimerStop(bug_delay_timer);
    } 
	apps_data.bugtime = 0;
	#ifdef __JS1228_SENSOR__
	apps_data.earinpatchflg = false;
	#else
	apps_data.boxinoutpatchflg = false;
	#endif
}

void set_disconnet03errorflg(void)
{
	if(app_tws_ibrt_tws_link_connected())
	{
	   twscmdsync[0] = TWSSYNCCMD_DISCONNECTERROR03;
	   if(app_ibrt_ui_get_enter_pairing_mode())
	   {
           twscmdsync[1] = 1;
	   }
	   else
	   {
	       twscmdsync[1] = 0;
	   }
	   app_ibrt_customif_test1_cmd_send(twscmdsync,10);	
	   osDelay(50);
	}
}



void spp_cmd_poweroff(void)
{
	osDelay(1000);
	apps_data.charingpoweroffflg = false;
	app_shutdown();
}


#ifdef __RELEASE_A2DP_VOLUME__
extern bool a2dp_releaseflg;
extern bool btapp_hfp_is_sco_active(void);
void release_a2dp_volume(void)
{
  if(!btapp_hfp_is_sco_active())
  {
      if(a2dp_releaseflg == false)
      {

      }
      else
      {
         app_bt_set_volume(APP_BT_STREAM_A2DP_SBC,app_bt_stream_a2dpvolume_get());
         btapp_a2dp_report_speak_gain();
         a2dp_releaseflg = false;
     }
  }
}
#endif
void sync_key_cmd(void)
{
   twscmdsync[0] = TWSSYNCCMD_APPKEY;
   twscmdsync[1] = bleappdata.leftclicldata;
   twscmdsync[2] = bleappdata.rightclicldata;
   twscmdsync[3] = bleappdata.leftdoubleclicldata;
   twscmdsync[4] = bleappdata.rightdoubleclicldata;
   twscmdsync[5] = bleappdata.lefttrubleclicldata;
   twscmdsync[6] = bleappdata.righttrubleclicldata;


   twscmdsync[7] = bleappdata.leftfourclicldata;
   twscmdsync[8] = bleappdata.rightfourclicldata;
   twscmdsync[9] = bleappdata.leftlongdata;
   twscmdsync[10] = bleappdata.rightlongdata;

   
   twscmdsync[11] = bleappdata.setnameflg;
   app_ibrt_customif_test1_cmd_send(twscmdsync,12);
}



void sync_btname_cmd(uint8_t* ptrData, uint32_t dataLength)
{
   twscmdsync[0] = TWSSYNCCMD_APPNMAE;
   twscmdsync[1] = dataLength;
   memcpy(&twscmdsync[2], bleappdata.rev2_bt_name,dataLength);
   app_ibrt_customif_test1_cmd_send(twscmdsync,dataLength+2);
}



void sync_eq_cmd(uint8_t* ptrData, uint32_t dataLength)
{
   twscmdsync[0] = TWSSYNCCMD_APPEQ;
   memcpy(&twscmdsync[1], ptrData,dataLength);
   app_ibrt_customif_test1_cmd_send(twscmdsync,dataLength+1);
}



void sync_ring_cmd(uint8_t onoff)
{
   twscmdsync[0] = TWSSYNCCMD_APPRING;
   twscmdsync[1] = onoff;
   app_ibrt_customif_test1_cmd_send(twscmdsync,2);
}



/**************************************************************************************************************/
void sync_pre_cmd(void)
{
   twscmdsync[0] = TWSSYNCCMD_APPPRE;
 //  memcpy(&twscmdsync[1], ptrData,dataLength);
    app_ibrt_customif_test1_cmd_send(twscmdsync,1);
}


void sync_nex_cmd(void)
{
   twscmdsync[0] = TWSSYNCCMD_APPNEX;
  // memcpy(&twscmdsync[1], ptrData,dataLength);
   app_ibrt_customif_test1_cmd_send(twscmdsync,1);
}


void sync_paly_cmd(void)
{
   twscmdsync[0] = TWSSYNCCMD_APPPLAY;
  // memcpy(&twscmdsync[1], ptrData,dataLength);
    app_ibrt_customif_test1_cmd_send(twscmdsync,1);
}



void sync_stop_cmd(void)
{
   twscmdsync[0] = TWSSYNCCMD_APPSTOP;
   app_ibrt_customif_test1_cmd_send(twscmdsync,1);
}
void sync_sco_volume_cmd(void)
{
   twscmdsync[0] = TWSSYNCCMD_SCOVOLUME;
  // memcpy(&twscmdsync[1], ptrData,dataLength);
   app_ibrt_customif_test1_cmd_send(twscmdsync,1);
}


void sco_volume_sync(void)
{
	app_bt_set_volume(APP_BT_STREAM_HFP_PCM,12);
	//hfp_update_local_volume(0, 12);
	//btapp_hfp_report_speak_gain(); 
}

/**************************************************************************************************************/
#ifdef __INOUTBOX_PATCH__
void set_inout_status_sync(void)
{
	if(app_tws_ibrt_tws_link_connected())
	{
	   twscmdsync[0] = TWSSYNCCMD_INOUTSTATUS;
	   twscmdsync[1] = get_inoutbox_status();
	   app_ibrt_customif_test1_cmd_send(twscmdsync,2);	
	}
	inoutbox_delay_set_time(2000);


	#ifdef __INOUTBOX_STATUS_APP_PATCH__
	#ifdef __BLE_S80__
    check_battery(app_battery_current_currvolt());
	#endif
	#endif
}
#endif




void app_poweroff_box(void)
{
    apps_data.charingpoweroffflg = true;
    app_shutdown();
}

#ifdef __BLE_ADV_PATCH__
extern uint8_t  app_poweroff_flag;
#endif
extern bt_status_t LinkDisconnectDirectly(void);
int app_nvrecord_rebuild_app(bool appdata)
{

 //if(apps_data.poweron5sflg == true)

#ifdef __BLE_ADV_PATCH__
	 app_poweroff_flag = true;
#endif

#ifdef __TWSPAIRLISTNV__
	twslistkey_section_data[6] = 0;
	twslistkey_section_data[7] = 0xff;

	twslistkey_data_set();
#endif	

 if(appdata == true)
 {
    twscmdsync[0] = TWSSYNCCMD_APPFACTORYRESET;
    app_ibrt_customif_test1_cmd_send(twscmdsync,1);
	osDelay(50);
 }
 else
 {
   // if(apps_data.poweron5sflg != true)
    {
   //    return 0;
    }
 }
 {
  //  struct nvrecord_env_t *nvrecord_env;
	//app_ibrt_ui_event_entry(IBRT_CLOSE_BOX_EVENT);
    app_status_indication_set(APP_STATUS_INDICATION_CLEARSUCCEED);

	bleappdata.leftclicldata = 0x05;
	bleappdata.rightclicldata = 0x05;
	bleappdata.leftdoubleclicldata = 0x03;
	bleappdata.rightdoubleclicldata = 0x04;
	bleappdata.lefttrubleclicldata = 0x0D;
	bleappdata.righttrubleclicldata = 0x02;
	bleappdata.leftfourclicldata = 0x0D;
	bleappdata.rightfourclicldata = 0x0D;	
	bleappdata.leftlongdata = 0x0a;
	bleappdata.rightlongdata = 0x09;
	bleappdata.setnameflg = false;
	bleappdata.eqmode = 0;
	bleappdata.eq_gain[0] = 0;
	bleappdata.eq_gain[1] = 0;
	bleappdata.eq_gain[2] = 0;
	bleappdata.eq_gain[3] = 0;
	bleappdata.eq_gain[4] = 0;
	bleappdata.eq_gain[5] = 0;
	bleappdata.eq_gain[6] = 0;
	


	
	LinkDisconnectDirectly();
    //osDelay(1000);
	/*
    nv_record_env_get(&nvrecord_env);
	
	nvrecord_env->flashint =0;
	nvrecord_env->leftclicldata = leftclicldata;
	nvrecord_env->rightclicldata = rightclicldata;
	nvrecord_env->leftdoubleclicldata = leftdoubleclicldata;
	nvrecord_env->rightdoubleclicldata = rightdoubleclicldata;
	nvrecord_env->lefttrubleclicldata = lefttrubleclicldata;
	nvrecord_env->righttrubleclicldata = righttrubleclicldata;	
	nvrecord_env->leftlongdata = leftlongdata;	
	nvrecord_env->rightlongdata = rightlongdata;	
	nvrecord_env->setnameflg = setnameflg;	
	memcpy(nvrecord_env->rev2_bt_name,rev2_bt_name ,32);
	nvrecord_env->eqmode = eqmode;
	memcpy(nvrecord_env->eq_gain, eq_gain,7);
	*/
	

    nv_record_sector_clear();
    nv_record_env_init();
    nv_record_sector_clear2();
   // nv_record_update_factory_tester_status(NVRAM_ENV_FACTORY_TESTER_STATUS_TEST_PASS);
   // nv_record_env_set(nvrecord_env);
    nv_record_flash_flush();
	app_ibrt_remove_history_paired_device();
	nv_record_clear_ibrt_info();
	app_ibrt_nvrecord_delete_all_mobile_record();
	bleapp_data_set();

	//bleapp_data_set();
	osDelay(1500);

	
	 //if(appdata == true)
	 {
        // app_reset();
	 }
	 //else
	 {
	    app_shutdown();
	 }
  }
    return 0;
}








void BT_PAIRING(void)
{
  ibrt_ctrl_t *p_ibrt_ctrl = app_ibrt_if_get_bt_ctrl_ctx();
 // app_ibrt_ui_t *p_ibrt_ui = app_ibrt_ui_get_ctx();

  struct nvrecord_env_t *nvrecord_env;
  nv_record_env_get(&nvrecord_env);


  apps_data.featchoutflg = true;

/**********************************************************************************************/

  {
/**********************************************************************************************/  
  if(is_find_tws_peer_device_onprocess())
  {
     find_tws_peer_device_stop();
  }


  if(app_tws_ibrt_tws_link_connected())
  {
	 if(p_ibrt_ctrl->nv_role == IBRT_SLAVE)
	 {

	    twscmdsync[0] = TWSSYNCCMD_ENTERPAIRING;
	    // twscmdsync[1] = 1;
	    app_ibrt_customif_test1_cmd_send(twscmdsync,2);
	 }
  }


  
  
  if(IBRT_UNKNOW == nvrecord_env->ibrt_mode.mode)
  {
  	   if(app_tws_ibrt_mobile_link_connected())
	   {
            //app_ibrt_ui_event_entry(IBRT_DISCONNECT_MOBILE_TWS_EVENT);
			app_ibrt_if_event_entry(IBRT_SASS_DISCONNECT_MOBILE);
			app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);
	   }
	   else
	   {
          //app_tws_ibrt_set_access_mode(BTIF_BAM_GENERAL_ACCESSIBLE);
          app_ibrt_ui_event_entry(IBRT_FREEMAN_PAIRING_EVENT);
		  app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);
	   }
  }
  else
  {
	  if(app_tws_ibrt_tws_link_connected())
	  {
	    if (p_ibrt_ctrl->nv_role != IBRT_SLAVE)
		{
	       app_ibrt_if_pairing_mode_refresh();
		   app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);
		}
	  }
  }
  }
#if defined(__BTIF_AUTOPOWEROFF__)
	app_start_10_second_timer(APP_POWEROFF_TIMER_ID);
#endif	




  
}



bool tws_test_boxoff_delay(void)
{
	ibrt_ctrl_t *p_ibrt_ctrl = app_ibrt_if_get_bt_ctrl_ctx();

	if(app_tws_ibrt_tws_link_connected())
	{
	  if (p_ibrt_ctrl->nv_role == IBRT_SLAVE)
	  {
          return true;
	  }
	}
    return false;
}


void app_enter_bttest_fjh(void)
{
	 app_ibrt_remove_history_paired_device();
	 nv_record_sector_clear2();
	 nv_record_clear_ibrt_info();
	// app_ibrt_nvrecord_rebuild_fjh();
	 app_ibrt_nvrecord_delete_all_mobile_record();
 
	 hal_sw_bootmode_clear(HAL_SW_BOOTMODE_REBOOT);
	 hal_sw_bootmode_set(HAL_SW_BOOTMODE_BT_SING_REASET);

	 nv_record_flash_flush();
	 norflash_api_flush_all();

	 app_reset(); 
}


#ifdef __BLE_ADV_PATCH__
extern uint8_t  app_poweroff_flag;
#endif
void app_enter_twspairing_fjh(void)
{
	 app_status_indication_set(APP_STATUS_INDICATION_CLEARSUCCEED);

#ifdef __BLE_ADV_PATCH__
	 app_poweroff_flag = true;
#endif
	 //éœ€è¦åŠ ä¸ªclose boxäº‹ä»¶ï¼Œè®©è€³æœºå¿«é€Ÿæ–­å¼€ 4.7åŽç»­ç‰ˆæœ¬éœ€æ·»åŠ 

     app_ibrt_ui_event_entry(IBRT_CLOSE_BOX_EVENT);
	 LinkDisconnectDirectly();

#ifdef __BLE_S80__
	 app_ble_force_switch_adv(BLE_SWITCH_USER_BOX, false);
#endif
	 



#ifdef __TWSPAIRLISTNV__
	 twslistkey_section_data[6] = 0;
	 twslistkey_section_data[7] = 0xff;

	 twslistkey_data_set();
#endif	

	 //app_bt_stream_hfpvolume_reset();
	// app_bt_stream_a2dpvolume_reset();


 
	 bleappdata.leftclicldata = 0x05;
	 bleappdata.rightclicldata = 0x05;
	 bleappdata.leftdoubleclicldata = 0x03;
	 bleappdata.rightdoubleclicldata = 0x04;
	 bleappdata.lefttrubleclicldata = 0x0D;
	 bleappdata.righttrubleclicldata = 0x02;

	 bleappdata.leftfourclicldata = 0x0D;
	 bleappdata.rightfourclicldata = 0x0D;	 
	 bleappdata.leftlongdata = 0x0a;
	 bleappdata.rightlongdata = 0x09;
	 bleappdata.setnameflg = false;
	 bleappdata.eqmode = 0;
	 bleappdata.eq_gain[0] = 0;
	 bleappdata.eq_gain[1] = 0;
	 bleappdata.eq_gain[2] = 0;
	 bleappdata.eq_gain[3] = 0;
	 bleappdata.eq_gain[4] = 0;
	 bleappdata.eq_gain[5] = 0;
	 bleappdata.eq_gain[6] = 0;

	 bleapp_data_set();

	 app_ibrt_remove_history_paired_device();
	 nv_record_sector_clear2();
	 nv_record_clear_ibrt_info();
	// app_ibrt_nvrecord_rebuild_fjh();
	 app_ibrt_nvrecord_delete_all_mobile_record();
 
	 hal_sw_bootmode_clear(HAL_SW_BOOTMODE_REBOOT);
	 hal_sw_bootmode_set(HAL_SW_BOOTMODE_BT_TWS_REASET);

	 nv_record_flash_flush();
	 norflash_api_flush_all();

	 //éœ€è¦åŠ ä¸ªå»¶æ—¶æ¥æ˜¾ç¤ºLEDï¼Œå……ç”µä»“ä¼špattonæœåŠ¡....................  4.7åŽç»­ç‰ˆæœ¬éœ€æ·»åŠ 

	 osDelay(1500);

	 app_reset();
}



void app_clear_btpailist_fjh(bool appdata)
{
#if 0
	   ibrt_ctrl_t *p_ibrt_ctrl = app_ibrt_if_get_bt_ctrl_ctx();
	  // app_ibrt_ui_t *p_ibrt_ui = app_ibrt_ui_get_ctx();
	 
	   struct nvrecord_env_t *nvrecord_env;
	   nv_record_env_get(&nvrecord_env);
	 
	 
	   apps_data.featchoutflg = true;
	 
	 /**********************************************************************************************/
	 
	   {
	 /**********************************************************************************************/  
	   if(is_find_tws_peer_device_onprocess())
	   {
		  find_tws_peer_device_stop();
	   }
	   
	   if(IBRT_UNKNOW == nvrecord_env->ibrt_mode.mode)
	   {
			if(app_tws_ibrt_mobile_link_connected())
			{
				 //app_ibrt_ui_event_entry(IBRT_DISCONNECT_MOBILE_TWS_EVENT);
				 app_ibrt_if_event_entry(IBRT_SASS_DISCONNECT_MOBILE);
				 app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);
			}
			else
			{
			   //app_tws_ibrt_set_access_mode(BTIF_BAM_GENERAL_ACCESSIBLE);
			   app_ibrt_ui_event_entry(IBRT_FREEMAN_PAIRING_EVENT);
			   app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);
			}
	   }
	   else
	   {
		   if(app_tws_ibrt_tws_link_connected())
		   {
			 if (p_ibrt_ctrl->nv_role != IBRT_SLAVE)
			 {
				app_ibrt_if_pairing_mode_refresh();
				app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);
			 }
		   }
	   }
	   }
#if defined(__BTIF_AUTOPOWEROFF__)
		 app_start_10_second_timer(APP_POWEROFF_TIMER_ID);
#endif	
#endif

	 app_status_indication_set(APP_STATUS_INDICATION_CLEARSUCCEED);

#ifdef __BLE_ADV_PATCH__
     app_poweroff_flag = true;
#endif


/*****************************************************************************************/
/*
	if(appdata == true)
	{
	   twscmdsync[0] = TWSSYNCCMD_APPFACTORYRESET_PHONELOST;
	   app_ibrt_customif_test1_cmd_send(twscmdsync,1);
	   osDelay(50);
	}

	app_enter_twspairing_fjh();
*/
/*****************************************************************************************/


#if 1

		  //éœ€è¦åŠ ä¸ªclose boxäº‹ä»¶ï¼Œè®©è€³æœºå¿«é€Ÿæ–­å¼€ 4.7åŽç»­ç‰ˆæœ¬éœ€æ·»åŠ 
#ifdef __BLE_S80__
		//  app_ble_force_switch_adv(BLE_SWITCH_USER_BOX, false);
#endif
		 // app_ibrt_ui_event_entry(IBRT_CLOSE_BOX_EVENT);



	 bleappdata.leftclicldata = 0x05;
	 bleappdata.rightclicldata = 0x05;
	 bleappdata.leftdoubleclicldata = 0x03;
	 bleappdata.rightdoubleclicldata = 0x04;
	 bleappdata.lefttrubleclicldata = 0x0D;
	 bleappdata.righttrubleclicldata = 0x02;

	 bleappdata.leftfourclicldata = 0x0D;
	 bleappdata.rightfourclicldata = 0x0D;	 
	 bleappdata.leftlongdata = 0x0a;
	 bleappdata.rightlongdata = 0x09;
	 bleappdata.setnameflg = false;
	 bleappdata.eqmode = 0;
	 bleappdata.eq_gain[0] = 0;
	 bleappdata.eq_gain[1] = 0;
	 bleappdata.eq_gain[2] = 0;
	 bleappdata.eq_gain[3] = 0;
	 bleappdata.eq_gain[4] = 0;
	 bleappdata.eq_gain[5] = 0;
	 bleappdata.eq_gain[6] = 0;

 
	 if(appdata == true)
	 {
		twscmdsync[0] = TWSSYNCCMD_APPFACTORYRESET_PHONELOST;
		app_ibrt_customif_test1_cmd_send(twscmdsync,1);
		osDelay(50);
	 }

     LinkDisconnectDirectly();
	 osDelay(500);

	 app_ibrt_remove_history_paired_device();
	 nv_record_sector_clear2();
	// nv_record_clear_ibrt_info();
	// app_ibrt_nvrecord_rebuild_fjh();
	 app_ibrt_nvrecord_delete_all_mobile_record();
	 nv_record_flash_flush();
	 norflash_api_flush_all();
	 bleapp_data_set();
	 osDelay(1000);

     app_shutdown();
	// app_reset();
	 #endif
}





void app_enter_dut_fjh(void)
{
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_TEST_MODE|HAL_SW_BOOTMODE_TEST_SIGNALINGMODE);
    pmu_reboot();
}


void app_enter_ota_fjh(void)
{

    if(apps_data.poweron5sflg == true)
    {
	   TRACE(0, "app_single_dld_mode_enter");
	   hal_sw_bootmode_set(HAL_SW_BOOTMODE_SINGLE_LINE_DOWNLOAD);
	   pmu_reboot();
	   return;
    }


	if(apps_data.poweron5sflg == true)
	{

	#if 1
			hal_norflash_disable_protection(HAL_FLASH_ID_0);
		
			hal_sw_bootmode_set(HAL_SW_BOOTMODE_ENTER_HIDE_BOOT);
   #ifdef __KMATE106__
			app_status_indication_set(APP_STATUS_INDICATION_OTA);
			app_voice_report(APP_STATUS_INDICATION_WARNING, 0);
			osDelay(1200);
   #endif
            osDelay(100);
			hal_cmu_sys_reboot();
   #endif
			


	}
}



void app_enter_ota_box(void)
{
	//if(apps_data.poweron5sflg == true)
	{

	#if 0
			hal_norflash_disable_protection(HAL_FLASH_ID_0);
		
			hal_sw_bootmode_set(HAL_SW_BOOTMODE_ENTER_HIDE_BOOT);
   #ifdef __KMATE106__
			app_status_indication_set(APP_STATUS_INDICATION_OTA);
			app_voice_report(APP_STATUS_INDICATION_WARNING, 0);
			osDelay(1200);
   #endif
            osDelay(100);
			hal_cmu_sys_reboot();
   #endif
			

	TRACE(0, "app_single_dld_mode_enter");
	hal_sw_bootmode_set(HAL_SW_BOOTMODE_SINGLE_LINE_DOWNLOAD);
	pmu_reboot();		

	}
}



void app_poweron_pairing_fjh(void)
{
	struct nvrecord_env_t *nvrecord_env;
	nv_record_env_get(&nvrecord_env);

	if(IBRT_UNKNOW == nvrecord_env->ibrt_mode.mode)
	{
          app_ibrt_ui_event_entry(IBRT_FREEMAN_PAIRING_EVENT);
		  app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);
          #ifdef MEDIA_PLAYER_SUPPORT
		  app_voice_report(APP_STATUS_INDICATION_BOTHSCAN,0);
          #endif		  
	}
	else
	{
	      app_ibrt_ui_set_enter_pairing_mode(IBRT_MOBILE_CONNECTED);
	      #if 1
		  app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);
          #ifdef MEDIA_PLAYER_SUPPORT
		  app_voice_report(APP_STATUS_INDICATION_BOTHSCAN,0);
          #endif	
          app_tws_ibrt_set_access_mode(BTIF_BAM_GENERAL_ACCESSIBLE);
		  #endif  
		  //app_ibrt_ui_set_enter_pairing_mode(IBRT_MOBILE_CONNECTED);
    }
    apps_data.poweron5sflg = true;
    #if defined(__BTIF_AUTOPOWEROFF__)
	app_start_10_second_timer(APP_POWEROFF_TIMER_ID);
    #endif	
}



void app_tws_pairing_fjh(void)
{
    app_status_indication_set(APP_STATUS_TWS_PAIRING);

#ifdef __TWSPAIRLISTNV__
			factory_tws_pairing_flg = true;
#endif

	app_ibrt_enter_limited_mode();
	//app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);
    if(app_tws_is_left_side())
	{
       app_start_tws_serching_direactly();
	}
}



void app_tws_pairing_fjh2(void)
{
    app_status_indication_set(APP_STATUS_TWS_PAIRING);

#ifdef __TWSPAIRLISTNV__
			factory_tws_pairing_flg = true;
#endif

	app_ibrt_enter_limited_mode();
	//app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);
    //if(app_tws_is_left_side())
	{
       app_start_tws_serching_direactly();
	}
}



void app_mobile_connected_fjh(void)
{
    fatch_delay_close_time();
    apps_data.poweron5sflg = false;
	apps_data.moblieconnectedflg = true;

#ifdef __INOUTBOX_PATCH__
	set_inout_status_sync();
#endif	

#ifdef __POWERON_RECONNECT_PAIIRNG__
		fjh_thread_msg_send(THREAD_MSG_APPS_POWER_ON_10S_STOP);
#endif

}


#ifdef __LICKEY__
extern "C" bool keylic_flg;

uint8_t bt_addr_start[6] = {
	0x1e,0x57,0x34,0x45,0x56,0x67
};
uint8_t bt_addr_end[6] = {
	0x1e,0x57,0x34,0x45,0x56,0x67
};


const uint8_t PROJECT_NAME[2] ={0x30,0x31};



uint8_t CNCE_CheckSum(uint8_t *buff,uint32_t len)
{
    uint8_t i = 0,crc = 0;
    while(len--)
    {
	   crc ^= *buff++;
	   for(i = 0;i<8;i++)
	   {
		   if(crc & 0X80)
			   crc = (crc<<1)^0X07;
		   else
			   crc <<= 1;
	   }
     }
     return crc;
}

bool lickey_auth(uint8_t* key, int Key_len)
{
   uint8_t i=0;
   uint8_t checksumdata = 0;
   uint8_t readdat[15] ={0};
   uint8_t ascilltohex[30] ={0};

   //ascill Âë×ª 16½øÖÆ
   for(i=0;i<30;i++)
   {
      if((key[i]<0x3A)&(key[i]>=0x30))
	  {
          ascilltohex[i] = key[i] - 0x30;
	  }
	  else if((key[i]<0x67)&(key[i]>0x60))
	  {
          ascilltohex[i] = key[i] - 0x57;
	  }
   }

   for(i=0;i<15;i++)
   {
       readdat[i] = ascilltohex[i*2]*0x10 + ascilltohex[i*2 +1];
   }

   TRACE(0,"readdat------------");
   DUMP8("0x%02x ", readdat, 15);   
   
   for(i=0;i<6;i++)
   {
	   bt_addr_start[i] = readdat[i+2];
   }
   for(i=0;i<6;i++)
   {
	   bt_addr_end[i] = readdat[i+8];
   }   
   
   DUMP8("0x%02x ", bt_addr, 6); 
   DUMP8("0x%02x ", bt_addr_start, 6); 
   DUMP8("0x%02x ", bt_addr_end, 6); 


   
   if((PROJECT_NAME[0]==readdat[0])&(PROJECT_NAME[1]==readdat[1]))               //ÏîÄ¿ÃûµÄÆ¥Åä
   {
       //chencksum ¼ì²â......
	   checksumdata = (0xff- CNCE_CheckSum(readdat,14));
	   TRACE(0,"checksumdata = %x",checksumdata);
	   if(checksumdata == readdat[14])
	   {
          if((bt_addr[5] == bt_addr_start[5])&(bt_addr[4] == bt_addr_start[4])&(bt_addr[3] == bt_addr_start[3]))
		  {
               if(bt_addr[2]==bt_addr_start[2])
			   {
			       if(bt_addr[1]==bt_addr_start[1])
				   {
                       if(bt_addr[0]>=bt_addr_start[0])
					   {
					       keylic_flg = true;
                           return true;
					   }
				   }
				   else if((bt_addr[1]>bt_addr_start[1])&(bt_addr[1]<=bt_addr_end[1]))
				   {
				       keylic_flg = true;
				       return true;
				   }
			   }
			   else if((bt_addr[2]>bt_addr_start[2])&(bt_addr[2]==bt_addr_end[2]))
			   {
                   if((bt_addr[1]<=bt_addr_end[1]))  
				   {
				       keylic_flg = true;
                       return true;
				   }
			   }
			   else if((bt_addr[2]>bt_addr_start[2])&(bt_addr[2]<bt_addr_end[2]))
			   {
			           keylic_flg = true;
                       return true;
			   }
		  }
          return false;
	   }
   }
   else
   {
       return false;
   }
   return false;
}
#endif





//bool reconect_error_flg;
//bool leftrighttone_flg;
//bool poweron_leftright_flg;
extern bool repeortlongleftflg;
extern bool repeortlongrightflg;
bool twspairingpatchflg = false;
/******************************************************************************************/
bool app_box_poweron_fjh(void)
{
	struct nvrecord_env_t *nvrecord_env;
	nv_record_env_get(&nvrecord_env);
	
	//hal_gpio_pin_set(HAL_GPIO_PIN_P1_1);

#ifdef __CLOSEBOX_TWS_SWITCH_PATCH__
		twsswitchok_enterclosebox = false;
#endif


	
	
	if(apps_data.featchoutflg == true)
	{
        return false;
	}
	app_datainit_fjh();

	repeortlongleftflg = false;
	repeortlongrightflg = false;		

	apps_data.featchoutflg = true;

#if defined(__BTIF_AUTOPOWEROFF__)
	app_start_10_second_timer(APP_POWEROFF_TIMER_ID);
#endif		

	app_status_indication_set(APP_STATUS_INDICATION_POWERON);
	app_status_indication_set(APP_STATUS_INDICATION_PAGESCAN);





#ifdef __POWERON_ENTERPAIRING__
	apps_data.featchoutflg = true;
	app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);

	app_ibrt_ui_event_entry(IBRT_FREEMAN_PAIRING_EVENT);
								 //app_status_indication_set(APP_STATUS_BT_PAIRING_TEST_MODE);
#ifdef MEDIA_PLAYER_SUPPORT
	app_voice_report(APP_STATUS_INDICATION_BOTHSCAN,0);
#endif		
	apps_data.featchoutflg = true;
	osDelay(1000);
	return true;
#endif


	if(IBRT_UNKNOW == nvrecord_env->ibrt_mode.mode)
	{
		TRACE(0,"ibrt_ui_log:power on unknow mode");
		//app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);
		app_status_indication_set(APP_STATUS_TWS_PAIRING);
#ifdef __TWSPAIRLISTNV__
		factory_tws_pairing_flg = true;
#endif				
		app_ibrt_enter_limited_mode();
		//app_status_indication_set(APP_STATUS_TWS_PAIRING);
		if(app_tws_is_left_side())
		{
		   twspairingpatchflg = true;
		   app_start_tws_serching_direactly();
		}

		//app_start_10_second_timer(APP_POWEROFF_TIMER_ID);
	}
	else
	{
		TRACE(0,"ibrt_ui_log:power on %d fetch out", nvrecord_env->ibrt_mode.mode);

        #ifdef __FJH_SLAVER_BUG__
		if((IBRT_MASTER != nvrecord_env->ibrt_mode.mode))
		{
					slaverbug_delay_set_time(10000);
					apps_data.slaver_reconectflg = true;
		}
        #endif
		//app_ibrt_ui_event_entry(IBRT_OPEN_BOX_EVENT);
		//app_status_indication_set(APP_STATUS_INDICATION_CONNECTING);
		app_ibrt_ui_event_entry(IBRT_FETCH_OUT_EVENT);


#if 0
        if(boxopenbugflg == true)
		{
            apps_data.bugflg = true;
            bug_delay_set_time(1000);
		}
#endif
		
		#ifdef __FJH_FATCH_OUT_ERROR__
		//fatch_out_timeout_check();
		#endif		

#ifdef __POWERON_RECONNECT_PAIIRNG__
		fjh_thread_msg_send(THREAD_MSG_APPS_POWER_ON_10S_PAIRING);
#endif

		
	}


#ifdef __BLE_S80__
	app_ble_force_switch_adv(BLE_SWITCH_USER_BOX, true);
#endif




	
	return true;
}


extern uint8_t Left_batteryvolume;
extern uint8_t Right_batteryvolume;
#ifndef __BATTERY_APP_BUG__
extern "C" uint8_t oldret;
#endif

void app_box_close_fjh(void)
{
	app_ibrt_ui_t *p_ibrt_ui = app_ibrt_ui_get_ctx();
	TRACE(0,"p_ibrt_ui->active_event.event = %d ",p_ibrt_ui->active_event.event);

#ifdef __BLE_S80__
	app_ble_force_switch_adv(BLE_SWITCH_USER_BOX, false);
    if(app_voice_gfps_find_state())
    {
        app_voice_stop_gfps_find();
    }
#endif

    twspairingpatchflg = false;

	//¸æÖªÁíÍâ¶ú»ú disconnecttone OK
	set_disconnet03errorflg();
	#ifdef __HFPAUTOTOPHONE__
	earinout_delay_close_time();
	#endif

	//earinout_delay_close_time();

	if(is_find_tws_peer_device_onprocess())
	{
	   find_tws_peer_device_stop();
	}

	apps_data.bugflg = false;
	boxopenbugflg = true;

#ifdef ANC_APP
	app_anc_switch_locally(APP_ANC_MODE_OFF);
#endif
	
    disconnecttone_delay_close();
    fatch_delay_close_time();

#ifdef __INOUTBOX_PATCH__
	inoutbox_delay_close_time();
#endif

	
	bug_delay_close_time();
	app_status_indication_set(APP_STATUS_INDICATION_CHARGING);
	app_stop_10_second_timer(APP_PAIR_TIMER_ID);
    app_stop_10_second_timer(APP_POWEROFF_TIMER_ID);

	if(!app_tws_ibrt_tws_link_connected())
	{
	    if(app_tws_ibrt_mobile_link_connected())
		{
		    TRACE(0,"............IBRT_SASS_DISCONNECT_MOBILE");
            //app_ibrt_if_event_entry(IBRT_SASS_DISCONNECT_MOBILE);
		}
	}	



#ifdef __CLOSEBOX_TWS_SWITCH_PATCH__
    //å…ˆè¿›è¡Œä¸»ä»Žåˆ‡æ¢
	if(app_tws_ibrt_tws_link_connected()&(app_tws_ibrt_mobile_link_connected())&(apps_data.peerinboxstatus == false))
	{
	      twsswitchok_enterclosebox = true;
	      if(app_tws_if_trigger_role_switch())
	      {
		     TRACE(0,"TWS SWITCH OK.........");
	      }
		  else
		  {
		     TRACE(0,"TWS SWITCH error close box.........");
		     twsswitchok_enterclosebox = false;
             app_ibrt_ui_event_entry(IBRT_CLOSE_BOX_EVENT);
		  }
	}
	else
	{
	if((p_ibrt_ui->active_event.event == IBRT_RECONNECT_EVENT))
	{
		p_ibrt_ui->active_event.event = IBRT_CLOSE_BOX_EVENT;
	}
	app_ibrt_ui_event_entry(IBRT_CLOSE_BOX_EVENT);
	}
#else	
    if((p_ibrt_ui->active_event.event == IBRT_RECONNECT_EVENT))
	{
		p_ibrt_ui->active_event.event = IBRT_CLOSE_BOX_EVENT;
	}
	app_ibrt_ui_event_entry(IBRT_CLOSE_BOX_EVENT);
#endif	
	

	apps_data.featchoutflg = false;
#ifdef __CLOSEBOX_PATCH__
	fjh_thread_msg_send(THREAD_MSG_APPS_EVENT_CLOSE_BOX_PATCH_TEST);
#endif	


    resetcharingflg();
#ifdef __CLOSEBOX_RESET_BATTERY_LVE__
	charing_reset_test();
#endif

#ifdef __BLE_S80__

	Left_batteryvolume = 0xff;
	Right_batteryvolume = 0xff;
#endif

#ifndef __BATTERY_APP_BUG__
#ifdef __BLE_S80__

	oldret = 100;
#endif
#endif

#ifdef __LINKLOSS_NO_CONNECT_TONE__
		linklossnoconnectedtoneflg = false;
#endif	

#ifdef __POWERON_RECONNECT_PAIIRNG__
			   fjh_thread_msg_send(THREAD_MSG_APPS_POWER_ON_10S_STOP);
#endif

	
	//hal_gpio_pin_clr(HAL_GPIO_PIN_P1_1);
}




void set_battery_lev_to_peer(uint8_t lev)
{
	if(app_tws_ibrt_tws_link_connected())
	{
	   twscmdsync[0] = TWSSYNCCMD_TWSBATTERYSTATUS;
	   twscmdsync[1] = lev;
	   app_ibrt_customif_test1_cmd_send(twscmdsync,2);
	}
}






void app_box_in_fjh(void)
{

#ifdef __INOUTBOX_PATCH__
	   set_inout_status_sync();
#endif

#if 0
	app_ibrt_ui_t *p_ibrt_ui = app_ibrt_ui_get_ctx();
	TRACE(0,"p_ibrt_ui->active_event.event = %d ",p_ibrt_ui->active_event.event);

	//¸æÖªÁíÍâ¶ú»ú disconnecttone OK
	set_disconnet03errorflg();
	#ifdef __HFPAUTOTOPHONE__
	earinout_delay_close_time();
	#endif

	earinout_delay_close_time();
	
	if(is_find_tws_peer_device_onprocess())
	{
	   find_tws_peer_device_stop();
	}

	apps_data.bugflg = false;
	boxopenbugflg = true;

#ifdef ANC_APP
	app_anc_switch_locally(APP_ANC_MODE_OFF);
#endif
	
    disconnecttone_delay_close();
    fatch_delay_close_time();
	bug_delay_close_time();
	app_status_indication_set(APP_STATUS_INDICATION_CHARGING);
	app_stop_10_second_timer(APP_PAIR_TIMER_ID);
    app_stop_10_second_timer(APP_POWEROFF_TIMER_ID);

	if(!app_tws_ibrt_tws_link_connected())
	{
	    if(app_tws_ibrt_mobile_link_connected())
		{
		    TRACE(0,"............IBRT_SASS_DISCONNECT_MOBILE");
            //app_ibrt_if_event_entry(IBRT_SASS_DISCONNECT_MOBILE);
		}
	}	

	if((p_ibrt_ui->active_event.event == IBRT_RECONNECT_EVENT))
	{
		p_ibrt_ui->active_event.event = IBRT_CLOSE_BOX_EVENT;
	}

	app_ibrt_ui_event_entry(IBRT_CLOSE_BOX_EVENT);
	apps_data.featchoutflg = false;
#ifdef __CLOSEBOX_PATCH__
	fjh_thread_msg_send(THREAD_MSG_APPS_EVENT_CLOSE_BOX_PATCH_TEST);
#endif	
	//hal_gpio_pin_clr(HAL_GPIO_PIN_P1_1);
#endif
}



/******************************************************************************************/
bool app_box_out_fjh(void)
{
#ifdef __INOUTBOX_PATCH__
		set_inout_status_sync();
#endif
return true;

#if 0

	struct nvrecord_env_t *nvrecord_env;
	nv_record_env_get(&nvrecord_env);
	
	//hal_gpio_pin_set(HAL_GPIO_PIN_P1_1);
	
	if(apps_data.featchoutflg == true)
	{
        return false;
	}
	app_datainit_fjh();

	repeortlongleftflg = false;
	repeortlongrightflg = false;		

	apps_data.featchoutflg = true;

#if defined(__BTIF_AUTOPOWEROFF__)
	app_start_10_second_timer(APP_POWEROFF_TIMER_ID);
#endif		

	app_status_indication_set(APP_STATUS_INDICATION_POWERON);

#ifdef __POWERON_ENTERPAIRING__
		apps_data.featchoutflg = true;
		app_ibrt_ui_event_entry(IBRT_FREEMAN_PAIRING_EVENT);
		//app_status_indication_set(APP_STATUS_BT_PAIRING_TEST_MODE);
#ifdef MEDIA_PLAYER_SUPPORT
        app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);
		app_voice_report(APP_STATUS_INDICATION_BOTHSCAN,0);
#endif		
		apps_data.featchoutflg = true;
		osDelay(1000);
		return true;
#endif



	if(IBRT_UNKNOW == nvrecord_env->ibrt_mode.mode)
	{
		TRACE(0,"ibrt_ui_log:power on unknow mode");
		
		app_ibrt_enter_limited_mode();
		app_status_indication_set(APP_STATUS_TWS_PAIRING);
		if(app_tws_is_left_side())
		app_start_tws_serching_direactly();

		//app_start_10_second_timer(APP_POWEROFF_TIMER_ID);
	}
	else
	{
		TRACE(0,"ibrt_ui_log:power on %d fetch out", nvrecord_env->ibrt_mode.mode);

        #ifdef __FJH_SLAVER_BUG__
		if((IBRT_MASTER != nvrecord_env->ibrt_mode.mode))
		{
					slaverbug_delay_set_time(10000);
					apps_data.slaver_reconectflg = true;
		}
        #endif
		//app_ibrt_ui_event_entry(IBRT_OPEN_BOX_EVENT);
		app_status_indication_set(APP_STATUS_INDICATION_CONNECTING);
		app_ibrt_ui_event_entry(IBRT_FETCH_OUT_EVENT);


#if 0
        if(boxopenbugflg == true)
		{
            apps_data.bugflg = true;
            bug_delay_set_time(1000);
		}
#endif
		
		#ifdef __FJH_FATCH_OUT_ERROR__
		//fatch_out_timeout_check();
		#endif		
	}
	return true;
	#endif
}


#ifdef __GAMMODE__
bool gamemodeflg = false;
extern uint8_t A2DP_PLAYER_PLAYBACK_DELAY_AAC_MTU;
extern uint8_t A2DP_PLAYER_PLAYBACK_DELAY_SBC_MTU;
void gamemusicmode_switch(void)
{
    if(gamemodeflg == true)
	{
        gamemodeflg = false;
		A2DP_PLAYER_PLAYBACK_DELAY_AAC_MTU = A2DP_PLAYER_PLAYBACK_DELAY_AAC_MTU_MUSIC;
		A2DP_PLAYER_PLAYBACK_DELAY_SBC_MTU = A2DP_PLAYER_PLAYBACK_DELAY_SBC_MTU_MUSCI;
        twscmdsync[0] = TWSSYNCCMD_GAMEMUSICTEST;
		twscmdsync[1] = 0;
        app_ibrt_customif_test1_cmd_send(twscmdsync,10);		
        app_voice_report(APP_STATUS_MUSIC_MODE,0);
	}
	else
	{
        gamemodeflg = true;
		A2DP_PLAYER_PLAYBACK_DELAY_AAC_MTU = A2DP_PLAYER_PLAYBACK_DELAY_AAC_MTU_GAME;
		A2DP_PLAYER_PLAYBACK_DELAY_SBC_MTU = A2DP_PLAYER_PLAYBACK_DELAY_SBC_MTU_GAME;
        twscmdsync[0] = TWSSYNCCMD_GAMEMUSICTEST;
		twscmdsync[1] = 1;
        app_ibrt_customif_test1_cmd_send(twscmdsync,10);			
		app_voice_report(APP_STATUS_GAME_MODE,0);
	}
}


void gamemodeset(void)
{
    gamemodeflg = true;
    A2DP_PLAYER_PLAYBACK_DELAY_AAC_MTU = A2DP_PLAYER_PLAYBACK_DELAY_AAC_MTU_GAME;
    A2DP_PLAYER_PLAYBACK_DELAY_SBC_MTU = A2DP_PLAYER_PLAYBACK_DELAY_SBC_MTU_GAME;
}

void musicmodeset(void)
{
    gamemodeflg = false;
    A2DP_PLAYER_PLAYBACK_DELAY_AAC_MTU = A2DP_PLAYER_PLAYBACK_DELAY_AAC_MTU_MUSIC;
    A2DP_PLAYER_PLAYBACK_DELAY_SBC_MTU = A2DP_PLAYER_PLAYBACK_DELAY_SBC_MTU_MUSCI;
}
#endif


void close_box_test(void)
{
	app_ibrt_ui_event_entry(IBRT_CLOSE_BOX_EVENT);
}


void app_box_poweroffcharing_fjh(void)
{
	if(is_find_tws_peer_device_onprocess())
	{
	   find_tws_peer_device_stop();
	}
	
    disconnecttone_delay_close();
    fatch_delay_close_time();
	app_status_indication_set(APP_STATUS_INDICATION_CHARGING);
	app_stop_10_second_timer(APP_PAIR_TIMER_ID);
    app_stop_10_second_timer(APP_POWEROFF_TIMER_ID);
	app_ibrt_ui_event_entry(IBRT_CLOSE_BOX_EVENT);
	apps_data.featchoutflg = false;
	//hal_gpio_pin_clr(HAL_GPIO_PIN_P1_1);
}



void app_bt_disconnect_fjh(void)
{
 // disconnecttone_delay_set_time(10000);
  app_ibrt_ui_t *p_ibrt_ui = app_ibrt_ui_get_ctx();

#ifdef __LINKLOSS_NO_CONNECT_TONE__
  linklossnoconnectedtoneflg = true;
#endif

  
  if((apps_data.link_disconnet == 0x08)||(p_ibrt_ui->active_event.event == IBRT_RECONNECT_EVENT)||(p_ibrt_ui->active_event.event  == IBRT_PEER_RECONNECT_EVENT))
  {
     apps_data.linklossflg = true;
	 TRACE(0,"............LINK LOSS");
     fatch_delay_set_time(10000);
  }
  else if(apps_data.leftrighttone_flg == false)
  {
     TRACE(0,"............LINK LOSS 2");
     apps_data.linklossflg = true;
     fatch_delay_set_time(10000); 
  }
  else
  {
      TRACE(0,"............NO LINK LOSS");
	  fatch_delay_set_time(100);   //v0.2.0   1500æ”¹ä¸º100
      #ifdef __LINKLOSS_NO_CONNECT_TONE__
	  linklossnoconnectedtoneflg = false;  //v0.2.0
      #endif  
  }
  apps_data.btconnectdtoboxcmdflg = false;


#ifdef __LINKLOSS_NO_CONNECT_TONE__
  linklossnoconnectedtoneflg = true;
#endif

  
  //bug_delay_close_time();
}


void app_bt_connect_fjh(void)
{
	ibrt_ctrl_t *p_ibrt_ctrl = app_ibrt_if_get_bt_ctrl_ctx();

	if(is_find_tws_peer_device_onprocess())
	{
	   find_tws_peer_device_stop();
	}	

	apps_data.moblieconnectedflg = true;
	apps_data.link_disconnet = 0;
	if(app_status_indication_get()!= APP_STATUS_INDICATION_CONNECTED)
	{
	    if(p_ibrt_ctrl->current_role == IBRT_SLAVE)
		{
		 twscmdsync[0] = TWSSYNCCMD_CONNECTEDLED;
		 app_ibrt_customif_test1_cmd_send(twscmdsync,10);
		} 
		app_status_indication_set(APP_STATUS_INDICATION_CONNECTED);
	}
	if(p_ibrt_ctrl->current_role != IBRT_SLAVE)
	{
#ifdef __LINKLOSS_NO_CONNECT_TONE__
			  if(linklossnoconnectedtoneflg == true)
			  {
	   
			  }
			  else
			  {
				 app_voice_report(APP_STATUS_INDICATION_CONNECTED,0);
			  }
#else
	          app_voice_report(APP_STATUS_INDICATION_CONNECTED,0);
#endif


       if(box_status_test2()==box_status_open)
       {
          if(twspairingpatchflg == true)
          {
          twspairingpatchflg = false;
          app_ibrt_ui_t *p_ibrt_ui = app_ibrt_ui_get_ctx();
          p_ibrt_ui->box_state = IBRT_OUT_BOX;
          }
       }

	   
	}

    #ifdef __JS1228_SENSOR__
	if(apps_data.earinpatchflg == true)
	{
        bug_delay_set_time(2000);
        //apps_data.earinpatchflg = false;
	}
	#else

	if(apps_data.boxinoutpatchflg == true)
	{
      //  bug_delay_set_time(2000);
      //  apps_data.boxinoutpatchflg = false;
	}
	#endif

    // ¸æËßºÐ×Ó¶ú»úÒÑ¾­Á¬½Ó³É¹¦.....
    apps_data.btconnectdtoboxcmdflg = true;

#ifdef __LINKLOSS_NO_CONNECT_TONE__
		linklossnoconnectedtoneflg = false;
#endif

	
    //c_Send_Cmd(BTCONNECTED);

#ifdef __POWERON_RECONNECT_PAIIRNG__
		fjh_thread_msg_send(THREAD_MSG_APPS_POWER_ON_10S_STOP);
#endif

	
}

static void disconnecttone_delay_delay_timehandler(void const * param);
osTimerDef(DISCONNECT_TONEELAY, disconnecttone_delay_delay_timehandler);
static osTimerId       disconnecttone_delay_timer = NULL;
extern uint8_t link_disconnet;
static void disconnecttone_delay_delay_timehandler(void const * param)
{
	apps_data.leftrighttone_flg = true;
    if(!app_tws_ibrt_tws_link_connected())
	{
	   if(!app_tws_ibrt_mobile_link_connected())
	   {
	      app_start_10_second_timer(APP_POWEROFF_TIMER_ID);  
	   }
	   //å…³é—­anc
	   #ifdef ANC_APP
	   app_anc_switch_locally(APP_ANC_MODE_OFF);
	   
	   apps_data.poweronautoancflg = true;
	   #endif
	}
}



void disconnecttone_delay_set_time(uint32_t millisec)
{
  if (disconnecttone_delay_timer == NULL)
  {
	 disconnecttone_delay_timer	= osTimerCreate(osTimer(DISCONNECT_TONEELAY), osTimerOnce, NULL);
	 osTimerStop(disconnecttone_delay_timer);
  } 
     osTimerStart(disconnecttone_delay_timer, millisec);
}


void disconnecttone_delay_close(void)
{
	if (disconnecttone_delay_timer != NULL)
	{
	   osTimerStop(disconnecttone_delay_timer);
	} 
	apps_data.leftrighttone_flg = true;
}



void app_tws_disconnect_fjh(void)
{

   if(apps_data.disconnet03errorflg == true)
   {
      apps_data.leftrighttone_flg = false;
      disconnecttone_delay_set_time(10000);
   }


   if(apps_data.moblieconnectedpatch03flg == false)
   {
      apps_data.twsslaverpatch02flg = true;
   }
   if(!app_tws_ibrt_mobile_link_connected())
   app_start_10_second_timer(APP_POWEROFF_TIMER_ID);


   
/*********************************************************************************************************************/
//TWS æ–­å¼€è¿žæŽ¥åŽï¼Œä¸ŠæŠ¥ç”µæ± ç”µé‡   
#ifdef __BLE_S80__

   if(app_tws_is_left_side())
   {
	  Right_batteryvolume = 0xff;
   }
   else if(app_tws_is_right_side())
   {
	  Left_batteryvolume = 0xff;
   }
   
   check_battery(app_battery_current_currvolt());
   
/*********************************************************************************************************************/


   peerbatterylev = 0;
#endif
}

#ifdef __JS1228_SENSOR__
extern "C" bool earinflg;
#endif

//extern uint16_t battery_curr_vol;
#ifdef __BLE_S80__
extern "C" u16 check_battery(uint16_t battery_val);
#endif
void app_tws_connect_fjh(void)
{
  ibrt_ctrl_t *p_ibrt_ctrl = app_ibrt_if_get_bt_ctrl_ctx(); 
  apps_data.twsslaverpatch02flg = false;

  apps_data.disconnet03errorflg = true;

  TRACE(0,"............app_tws_connect_fjh");
  #ifdef __POWERONAUTOOPENANC__
   TRACE(0,"apps_data.poweronautoancflg = %d p_ibrt_ctrl->current_role = %d",apps_data.poweronautoancflg,p_ibrt_ctrl->current_role);
  #endif

  if (p_ibrt_ctrl->current_role == IBRT_MASTER &&
		app_ibrt_ui_get_enter_pairing_mode())
	{
	    if(app_status_indication_get()!=APP_STATUS_INDICATION_BOTHSCAN)
		{
		   app_voice_report(APP_STATUS_INDICATION_BOTHSCAN, 0);
		   app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);
		}
		app_start_10_second_timer(APP_POWEROFF_TIMER_ID);
	}
	else
	{
	    //app_status_indication_set(APP_STATUS_INDICATION_CONNECTED);
	}
	if(p_ibrt_ctrl->current_role == IBRT_MASTER)
	{
		//app_voice_report(APP_STATUS_INDICATION_CONNECTED, 0);
		#ifdef __POWERONAUTOOPENANC__
		if(apps_data.poweronautoancflg == false)
		{
          #ifdef ANC_APP
          app_anc_sync_status();
          #endif	
		}
		#endif

        #ifdef __GAMMODE__
		//ÓÎÏ·Ä£Ê½Í¬²½...........
		twscmdsync[0] = TWSSYNCCMD_GAMEMUSICTEST;
		twscmdsync[1] = gamemodeflg;
        #ifdef __POWERONAUTOOPENANC__
		twscmdsync[2] = apps_data.poweronautoancflg;
        app_ibrt_customif_test1_cmd_send(twscmdsync,3);
		#else
		app_ibrt_customif_test1_cmd_send(twscmdsync,2);
        #endif
		#endif

		#ifdef __POWERONAUTOOPENANC__
		apps_data.poweronautoancflg = false;
		#endif
	}
	else //if(p_ibrt_ctrl->current_role == IBRT_SLAVE)
	{
	    //TWS Á¬½Ó³É¹¦ÌáÊ¾Òô²¥±¨
		twscmdsync[0] = TWSSYNCCMD_TWSCONNECTTONE;
		twscmdsync[1] = apps_data.leftrighttone_flg;
		twscmdsync[2] = apps_data.poweron_leftright_flg;

        /*
        #ifdef __POWERONAUTOOPENANC__
		twscmdsync[3] = apps_data.poweronautoancflg;
        app_ibrt_customif_test1_cmd_send(twscmdsync,4);
		#else
		app_ibrt_customif_test1_cmd_send(twscmdsync,3);
        #endif
        */
		app_ibrt_customif_test1_cmd_send(twscmdsync,3);

	
		app_stop_10_second_timer(APP_POWEROFF_TIMER_ID); 
		app_status_indication_set(APP_STATUS_INDICATION_CONNECTED);

        #ifdef __POWERONAUTOOPENANC__
	    //apps_data.poweronautoancflg = false;
        #endif
	}

	apps_data.poweron_leftright_flg = false;

#ifdef __JS1228_SENSOR__	
	//×óÓÒ¶úÈë¶úÐÅÏ¢¹²Ïí....................................
	twscmdsync[0] = TWSSYNCCMD_EARINSTATUS;
	twscmdsync[1] = earinflg;
	app_ibrt_customif_test1_cmd_send(twscmdsync,2);
#endif


	if (p_ibrt_ctrl->current_role == IBRT_MASTER)
	{
       twscmdsync[0] = TWSSYNCCMD_TWSVOLUMEFLG;
       twscmdsync[1] = repeortlongleftflg;
       twscmdsync[2] = repeortlongrightflg;
       app_ibrt_customif_test1_cmd_send(twscmdsync,3);
	}



//	uint8_t level =app_battery_current_level();
//    set_battery_lev_to_peer(level);
#ifdef __INOUTBOX_STATUS_APP_PATCH__
#ifdef __INOUTBOX_PATCH__
	set_inout_status_sync();
#endif
#endif


#ifdef __BLE_S80__
	twscmdsync[0] = TWSSYNCCMD_BATTERYLOV;
	twscmdsync[1] = check_battery(app_battery_current_currvolt());
	app_ibrt_customif_test1_cmd_send(twscmdsync,2);
#endif	




	
}


void app_tws_batterylowpoweroff_fjh(void)
{
    app_ibrt_ui_event_entry(IBRT_WEAR_DOWN_EVENT);
	osDelay(500);
	if (app_tws_ibrt_tws_link_connected())
	{
       app_ibrt_ui_event_entry(IBRT_CLOSE_BOX_EVENT);
	   osDelay(1500);
	}
	app_shutdown();
}


void app_cancel_tws_serach(void)
{
  if(is_find_tws_peer_device_onprocess())
  {
     find_tws_peer_device_stop();
  }
}
/*****************************************************************************************************************/
static void delay_enterpairing_timehandler(void const * param);
osTimerDef(DELEARPAIRIN_GENSORDELAY, delay_enterpairing_timehandler);
static osTimerId       delay_enterpairing_timer = NULL;
static void delay_enterpairing_timehandler(void const * param)
{
	ibrt_ctrl_t *p_ibrt_ctrl = app_ibrt_if_get_bt_ctrl_ctx();

	if(app_tws_ibrt_mobile_link_connected())
	{
        return;
	}
	if(p_ibrt_ctrl->current_role != IBRT_SLAVE)
	{
	   app_ibrt_ui_set_enter_pairing_mode(IBRT_MOBILE_CONNECTED);
       #if 1
	   app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);
       #ifdef MEDIA_PLAYER_SUPPORT
	   app_voice_report(APP_STATUS_INDICATION_BOTHSCAN,0);
       #endif	   
	   app_tws_ibrt_set_access_mode(BTIF_BAM_GENERAL_ACCESSIBLE);
       #endif	 
	   app_ibrt_ui_set_enter_pairing_mode(IBRT_MOBILE_CONNECTED);
	   app_start_10_second_timer(APP_POWEROFF_TIMER_ID); 
	}
}


void delay_enterpairing_set_time(uint32_t millisec)
{
    if (delay_enterpairing_timer == NULL)
    {
	   delay_enterpairing_timer	= osTimerCreate(osTimer(DELEARPAIRIN_GENSORDELAY), osTimerOnce, NULL);
	   osTimerStop(delay_enterpairing_timer);
    } 
       osTimerStart(delay_enterpairing_timer, millisec);
}

void delay_enterpairing_close_time(void)
{
    if (delay_enterpairing_timer != NULL)
    {
	   osTimerStop(delay_enterpairing_timer);
    } 
}
/*****************************************************************************************************************/




#ifdef __FJH_FATCH_OUT_ERROR__
static void fatch_delay_timehandler(void const * param);
osTimerDef(FATCH_GENSORDELAY, fatch_delay_timehandler);
static osTimerId       fatch_delay_timer = NULL;
uint8_t pairingtone_n = 0;
static void fatch_delay_timehandler(void const * param)
{
	ibrt_ctrl_t *p_ibrt_ctrl = app_ibrt_if_get_bt_ctrl_ctx();
	app_ibrt_ui_t *p_ibrt_ui = app_ibrt_ui_get_ctx();

	TRACE(0,".....fatch_delay_timehandler = %d ,p_ibrt_ctrl->current_role = %d", app_ibrt_ui_get_enter_pairing_mode(),p_ibrt_ctrl->current_role);

	TRACE(0,".....apps_data.linklossflg = %d apps_data.link_disconnet = %d p_ibrt_ui->active_event.event = %d", apps_data.linklossflg,apps_data.link_disconnet,p_ibrt_ui->active_event.event);


#ifdef __LINKLOSS_NO_CONNECT_TONE__
	linklossnoconnectedtoneflg = false;
#endif	

	if(app_tws_ibrt_mobile_link_connected())
	{
        return;
	}

	
	//if(apps_data.linklossflg == true)
	//{
        apps_data.linklossflg = false;
		if((apps_data.link_disconnet == 0x08)||(p_ibrt_ui->active_event.event == IBRT_RECONNECT_EVENT)||(p_ibrt_ui->active_event.event  == IBRT_PEER_RECONNECT_EVENT))
        { 
		   if(p_ibrt_ctrl->current_role != IBRT_SLAVE)
		   {
		       app_voice_report(APP_STATUS_INDICATION_DISCONNECTED, 0);
			   app_status_indication_set(APP_STATUS_INDICATION_PAGESCAN);
			   app_start_10_second_timer(APP_POWEROFF_TIMER_ID); 
		   }
		    
		}	
	//}
	else
	{
	    if(app_tws_ibrt_tws_link_connected())
		{
            if(p_ibrt_ctrl->current_role != IBRT_SLAVE)
			{
				app_voice_report(APP_STATUS_INDICATION_DISCONNECTED, 0);
				app_status_indication_set(APP_STATUS_INDICATION_PAGESCAN);

				delay_enterpairing_set_time(1500);

				/*
				app_ibrt_ui_set_enter_pairing_mode(IBRT_MOBILE_CONNECTED);
                #if 1
				app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);
                #ifdef MEDIA_PLAYER_SUPPORT
				app_voice_report(APP_STATUS_INDICATION_BOTHSCAN,0);
                #endif	   
				app_tws_ibrt_set_access_mode(BTIF_BAM_GENERAL_ACCESSIBLE);
                #endif	 
				app_ibrt_ui_set_enter_pairing_mode(IBRT_MOBILE_CONNECTED);
				app_start_10_second_timer(APP_POWEROFF_TIMER_ID); 
				*/
			}
		}
		else
		{
			app_voice_report(APP_STATUS_INDICATION_DISCONNECTED, 0);
			app_status_indication_set(APP_STATUS_INDICATION_PAGESCAN);

			delay_enterpairing_set_time(1500);

			/*
			app_ibrt_ui_set_enter_pairing_mode(IBRT_MOBILE_CONNECTED);
            #if 1
			app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);
            #ifdef MEDIA_PLAYER_SUPPORT
			app_voice_report(APP_STATUS_INDICATION_BOTHSCAN,0);
            #endif	   
			app_tws_ibrt_set_access_mode(BTIF_BAM_GENERAL_ACCESSIBLE);
            #endif	 
			app_ibrt_ui_set_enter_pairing_mode(IBRT_MOBILE_CONNECTED);
			app_start_10_second_timer(APP_POWEROFF_TIMER_ID); 
			*/
		}
	}
}


void fatch_delay_set_time(uint32_t millisec)
{
    if (fatch_delay_timer == NULL)
    {
	   fatch_delay_timer	= osTimerCreate(osTimer(FATCH_GENSORDELAY), osTimerOnce, NULL);
	   osTimerStop(fatch_delay_timer);
    } 
       osTimerStart(fatch_delay_timer, millisec);
}

void fatch_delay_close_time(void)
{
    pairingtone_n = 0;
    if (fatch_delay_timer != NULL)
    {
	   osTimerStop(fatch_delay_timer);
    } 
}


void fatch_out_timeout_check(void)
{
   return;
   fatch_delay_set_time(3000);
}
#endif


void app_bt_scan_fjh(void)
{
	return;
#ifdef __FJH_FATCH_OUT_ERROR__
   //fatch_delay_set_time(62000);
#endif
}

extern uint8_t  app_poweroff_flag;
void app_datainit_fjh(void)
{
   apps_data.linklossflg = false;
   apps_data.leftrighttone_flg = true;
   apps_data.moblieconnectedflg = false;
   apps_data.twslink_disconnet = 0;
   apps_data.link_disconnet = 0;
   apps_data.featchoutflg = false;
   apps_data.poweron5sflg = false;
   apps_data.slaver_reconectflg = false;
   apps_data.errorpatch01flg = false;
   apps_data.twsslaverpatch02flg = false;
   apps_data.moblieconnectedpatch03flg = false;
   apps_data.charingpoweroffflg = false;
   apps_data.twsslaverbug01flg = false;
   apps_data.recnecterrorenterpairflg = true;
   apps_data.pairingflg = false;
   #ifdef __FJH_PATCH_004__
   apps_data.fjhpatch004flg = false;
   #endif
   apps_data.bugtime = 0;
   apps_data.leftrighttone_flg = true;
   apps_data.poweron_leftright_flg = true;

   if(app_poweroff_flag == 0)
   {
	   apps_data.fjhpatch003flg = false;
	   apps_data.fjhpatch003okflg = false;
   }
   #ifdef __JS1228_SENSOR__
   apps_data.earinpatchflg = false;
   #endif
   apps_data.boxinoutpatchflg = false;
   apps_data.btconnectdtoboxcmdflg = false;
   apps_data.tws_switch_status_flg = false;
   apps_data.onepowerofftwsswitcherrortime = 0;
   apps_data.disconnet03errorflg = false;
   apps_data.anckeyenflg = true; 

//   apps_data.earineventflg = false;
   apps_data.twsdisconnctedenterpairing = false;

   apps_data.otherearstatus = false;

   apps_data.waredownflg = false;


#ifdef __INOUTBOX_PATCH__ 
	apps_data.peerinboxstatus = true;
#endif

   
}


#ifdef __INOUTBOX_STATUS_APP_PATCH__

void send_battery_status_to_app(void)
{
#ifdef __BLE_S80__

	check_battery(app_battery_current_currvolt());
#endif
}
#endif	





/*******************************************************************************************************************************************/
#ifdef __ANC_SWITCH_DELAY_FJH__
static void anc_tone_timer_handler(void *p)
{
	//app_anc_switch(mode_fjh);
	apps_data.anckeyenflg = true;
	
}


void app_anc_tone_init_timer(void)
{
    if (anc_tonetimerid == NULL)
        anc_tonetimerid = hwtimer_alloc((HWTIMER_CALLBACK_T)anc_tone_timer_handler, &anc_led_request);
}


void app_anc_tone_timer_set(uint32_t request, uint32_t delay)
{
    TRACE(3," %s request %d , delay %ds ", __func__, request, (TICKS_TO_MS(delay) / 1000));
    if (anc_tonetimerid == NULL)
        return;
    hwtimer_stop(anc_tonetimerid);
    hwtimer_start(anc_tonetimerid, delay);
}


int32_t app_anc_loop_switch_fjh(void)
{
    if(!app_tws_ibrt_tws_link_connected())
	{
        return 0;
	}
    if(apps_data.anckeyenflg == true)
	{
    apps_data.anckeyenflg = false;
    twscmdsync[0] = TWSSYNCCMD_TONEDELAY_FLG;
    twscmdsync[1] = apps_data.anckeyenflg;
    app_ibrt_customif_test1_cmd_send(twscmdsync,2);



	if(app_anc_get_curr_mode()==APP_ANC_MODE_OFF)
	{
        mode_fjh = APP_ANC_MODE1;
	}
	else if(app_anc_get_curr_mode()==APP_ANC_MODE1)
	{
        mode_fjh = APP_ANC_MODE2;
	}
	else if(app_anc_get_curr_mode()==APP_ANC_MODE2)
	{
        mode_fjh = APP_ANC_MODE1;
	}
    //mode_fjh = (app_anc_mode_t)((app_anc_get_curr_mode() + 1) % APP_ANC_MODE_QTY);
	
    if(mode_fjh == APP_ANC_MODE1)
    {
       app_voice_report(APP_STATUS_INDICATION_ANC_ON,0);
	}
	else if(mode_fjh == APP_ANC_MODE_OFF)
	{
	   app_voice_report(APP_STATUS_INDICATION_ANC_OFF,0);
	}
	else if(mode_fjh == APP_ANC_MODE2)
	{
       app_voice_report(APP_STATUS_INDICATION_AMB_ON,0);
	}
	app_anc_tone_timer_set(0,anc_tone_off_timetest);
	app_anc_switch(mode_fjh);
	}
    return 0;
}


void set_anckeytongdelay(void)
{
	apps_data.anckeyenflg = false;
    app_anc_tone_timer_set(0,anc_tone_off_timetest);
}

#endif
/*******************************************************************************************************************************************/






