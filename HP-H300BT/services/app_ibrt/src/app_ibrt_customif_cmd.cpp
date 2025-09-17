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
#include "string.h"
#include "app_tws_ibrt_trace.h"
#include "app_tws_ctrl_thread.h"
#include "app_tws_ibrt_cmd_handler.h"
#include "app_ibrt_customif_cmd.h"
#include "app_tws_if.h"
#if defined(USE_LOWLATENCY_LIB)
#include "app_ally.h"
#endif
#include "apps.h"
#include "fjh_apps_thread.h"
#include "fjh_apps.h"
#ifdef __POWERONAUTOOPENANC__
#include "app_anc.h"
#endif
#ifdef __HFPAUTOTOPHONE__
#include "hal_key.h"
#include "app_key.h"
#endif

//#include "jsa1228.h"
#include "app_ibrt_if.h"
#include "app_box2.h"
#include "tgt_hardware.h"
#include "btapp.h"

#ifdef __BLE_S80__
#include "ble_cmd.h"
#endif

#ifdef __BLE_S80__
#include "ble_cmd.h"
extern "C" void Ble_connect_battery_res(void);
extern "C" uint8_t Left_batteryvolume;
extern "C" uint8_t Right_batteryvolume;
#endif

#ifdef __BLEAPPKEY__
#include "bleapp_section.h"
#endif





#if defined(IBRT)
/*
* custom cmd handler add here, this is just a example
*/

#define app_ibrt_custom_cmd_rsp_timeout_handler_null   (0)
#define app_ibrt_custom_cmd_rsp_handler_null           (0)
#define app_ibrt_custom_cmd_rx_handler_null            (0)

void app_ibrt_customif_test1_cmd_send(uint8_t *p_buff, uint16_t length);
static void app_ibrt_customif_test1_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

static void app_ibrt_customif_test2_cmd_send(uint8_t *p_buff, uint16_t length);
static void app_ibrt_customif_test2_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
static void app_ibrt_customif_test2_cmd_send_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
static void app_ibrt_customif_test2_cmd_send_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
#if defined(USE_LOWLATENCY_LIB)
void app_ibrt_lly_cmd_send(uint8_t *p_buff, uint16_t length);
void app_ibrt_lly_sync_receive_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
#endif

static const app_tws_cmd_instance_t g_ibrt_custom_cmd_handler_table[]=
{
#ifdef GFPS_ENABLED
    {
        APP_TWS_CMD_SHARE_FASTPAIR_INFO,                        "SHARE_FASTPAIR_INFO",
        app_ibrt_share_fastpair_info,
        app_ibrt_shared_fastpair_info_received_handler,         0,
        app_ibrt_custom_cmd_rsp_timeout_handler_null,           app_ibrt_custom_cmd_rsp_handler_null
    },
    {
        APP_TWS_CMD_GFPS_RING_STOP_SYNC,                        "TWS_CMD_GFPS_STOP_RING_STATE",
        app_ibrt_customif_gfps_stop_ring_sync_send,
        app_ibrt_customif_gfps_stop_ring_sync_recv,             0,
        app_ibrt_custom_cmd_rsp_timeout_handler_null,           app_ibrt_custom_cmd_rsp_handler_null
    },
#endif
    {
        APP_IBRT_CUSTOM_CMD_TEST1,                              "TWS_CMD_TEST1",
        app_ibrt_customif_test1_cmd_send,
        app_ibrt_customif_test1_cmd_send_handler,               0,
        app_ibrt_custom_cmd_rsp_timeout_handler_null,           app_ibrt_custom_cmd_rsp_handler_null
    },
    {
        APP_IBRT_CUSTOM_CMD_TEST2,                              "TWS_CMD_TEST2",
        app_ibrt_customif_test2_cmd_send,
        app_ibrt_customif_test2_cmd_send_handler,               RSP_TIMEOUT_DEFAULT,
        app_ibrt_customif_test2_cmd_send_rsp_timeout_handler,   app_ibrt_customif_test2_cmd_send_rsp_handler
    },
#if defined(USE_LOWLATENCY_LIB)
    {
        APP_IBRT_CUSTOM_CMD_LLY_SYNC,                           "TWS_CMD_SYNC_LLY",
        app_ibrt_lly_cmd_send,
        app_ibrt_lly_sync_receive_handler,                      0,
        app_ibrt_custom_cmd_rsp_timeout_handler_null,           app_ibrt_custom_cmd_rsp_handler_null
    },
#endif
};

#ifdef __HFPAUTOTOPHONE__
void earinout_delay_set_time(uint32_t millisec);
void earinout_delay_close_time(void);
static void earinout_delay_timehandler(void const * param);
osTimerDef(EARINOUT_DELAY, earinout_delay_timehandler);
static osTimerId       earinout_delay_timer = NULL;

static void earinout_delay_timehandler(void const * param)
{
TRACE(0,"earinout_delay_timehandler.........");

if(get_inoutbox_status()||(box_status_test2()==box_status_close))
{
   return;
}

  if(app_tws_is_slave_mode()&(app_tws_ibrt_slave_ibrt_link_connected()))
  {
    if((apps_data.otherearstatus == true)&(earinflg == true))
	{
        
	}
	else if((apps_data.otherearstatus == true)&(earinflg == false))
	{
          //tws switch
	}
	else if((apps_data.otherearstatus == false)&(earinflg == true))
	{
	        /*
         	if(app_tws_if_trigger_role_switch())
			{
				 TRACE(0,"TWS SWITCH OK.........");
			}
			else
			{
                 earinout_delay_set_time(3000);
			}
			*/
			app_ibrt_ui_ware_down_event_test();
			apps_data.waredownflg = true;
	}
	else if((apps_data.otherearstatus == false)&(earinflg == false))
	{

	}	
  }
}


void earinout_delay_set_time(uint32_t millisec)
{
  if (earinout_delay_timer == NULL)
  {
	 earinout_delay_timer	= osTimerCreate(osTimer(EARINOUT_DELAY), osTimerOnce, NULL);
	 osTimerStop(earinout_delay_timer);
  } 
     osTimerStart(earinout_delay_timer, millisec);
}



void earinout_delay_close_time(void)
{
        apps_data.otherearstatus = false;
		if (earinout_delay_timer != NULL)
		{
		   osTimerStop(earinout_delay_timer);
		} 
}
#endif




int app_ibrt_customif_cmd_table_get(void **cmd_tbl, uint16_t *cmd_size)
{
    *cmd_tbl = (void *)&g_ibrt_custom_cmd_handler_table;
    *cmd_size = ARRAY_SIZE(g_ibrt_custom_cmd_handler_table);
    return 0;
}


void app_ibrt_customif_cmd_test(ibrt_custom_cmd_test_t *cmd_test)
{
    tws_ctrl_send_cmd(APP_IBRT_CUSTOM_CMD_TEST1, (uint8_t*)cmd_test, sizeof(ibrt_custom_cmd_test_t));
    tws_ctrl_send_cmd(APP_IBRT_CUSTOM_CMD_TEST2, (uint8_t*)cmd_test, sizeof(ibrt_custom_cmd_test_t));
}

void app_ibrt_customif_test1_cmd_send(uint8_t *p_buff, uint16_t length)
{
	//if(uart_cmd_enable == false)
	//{
    //   return;
	//}

    app_ibrt_send_cmd_without_rsp(APP_IBRT_CUSTOM_CMD_TEST1, p_buff, length);
    TRACE(1,"%s", __func__);
}

extern void leftringear_set_com(void);
extern void leftringear_claer_com(void);
extern bool repeortlongleftflg;
extern bool repeortlongrightflg;
extern bool earautoplayflg;
extern "C" void btname_sync_set(uint8_t* ptrData, uint32_t dataLength);
extern "C" void app_eq_sync_set(uint8_t* ptrData, uint32_t dataLength);
extern "C" void app_clear_btpailist_fjh(bool appdata);
static void app_ibrt_customif_test1_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
   switch(*p_buff)
   {


		  case TWSSYNCCMD_SCOVOLUME:
			 sco_volume_sync();
			 break;
           case TWSSYNCCMD_APPFACTORYRESET_PHONELOST:
		   	app_clear_btpailist_fjh(false);
		   	break;



#ifdef __INOUTBOX_PATCH__	
			case TWSSYNCCMD_INOUTSTATUS:
#ifdef __INOUTBOX_STATUS_APP_PATCH__
              if(apps_data.peerinboxstatus != p_buff[1])
              {
				//send_battery_status_to_app();
				apps_data.bugflgtest = true;
			  }
#endif					  
			
			  apps_data.peerinboxstatus = p_buff[1];
#ifdef __FJH_APPS__
			  fjh_thread_msg_send(THREAD_MSG_APPS_EVENT_INOUT_BOX_PATCH_TEST);
#endif	 
			  break;
#endif


      case TWSSYNCCMD_APPPRE:
	  	bt_key_handle_func_backward();
	  	break;

      case TWSSYNCCMD_APPNEX:
	  	bt_key_handle_func_forward();
	  	break;

      case TWSSYNCCMD_APPPLAY:
	  	bt_key_handle_func_play();
	  	break;

      case TWSSYNCCMD_APPSTOP:
	  	bt_key_handle_func_stop();
	  	break;	  



      case TWSSYNCCMD_ENTERPAIRING:
	  	BT_PAIRING();
	  	break;
   
      case TWSSYNCCMD_APPFACTORYRESET:
	  	app_nvrecord_rebuild_app(false);
	  	break;

      #ifdef __BLE_S80__

	  
	  case TWSSYNCCMD_APPRING:
	  	if(p_buff[1] == 1)
	  	{
           app_voice_start_gfps_find();
		}
		else
		{
            app_voice_stop_gfps_find();
		}
	  	break;

      case TWSSYNCCMD_APPEQ:
	  	#ifdef LONGWEI
	 	app_eq_sync_set(&p_buff[1],(length - 1));
		#endif
	 	break;
	  
      case TWSSYNCCMD_APPNMAE:
	  	btname_sync_set(&p_buff[2],p_buff[1]);
	  	break;

	  
      case TWSSYNCCMD_APPKEY:	  	
		bleappdata.leftclicldata = p_buff[1];
		bleappdata.rightclicldata = p_buff[2];
		bleappdata.leftdoubleclicldata = p_buff[3];
		bleappdata.rightdoubleclicldata = p_buff[4];
		bleappdata.lefttrubleclicldata = p_buff[5];
		bleappdata.righttrubleclicldata = p_buff[6];

		bleappdata.leftfourclicldata = p_buff[7];
		bleappdata.rightfourclicldata = p_buff[8];
		
		bleappdata.leftlongdata = p_buff[9];
		bleappdata.rightlongdata = p_buff[10];
		bleappdata.setnameflg = p_buff[11];
	  	break;


	  
      case TWSSYNCCMD_BATTERYLOV:
		if(app_tws_is_left_side())
		{
		   Right_batteryvolume = p_buff[1];
		}
		else if(app_tws_is_right_side())
		{
		   Left_batteryvolume = p_buff[1];
		}

		TRACE(1,".............Right_batteryvolume = %d   Left_batteryvolume = %d", Right_batteryvolume,Left_batteryvolume);
        Ble_connect_battery_res();
	  	break;
	  #endif	
   

      case TWSSYNCCMD_TWSBATTERYSTATUS:
	  	peerbatterylev = p_buff[1];
	  	break;


      case TWSSYNCCMD_TONEDELAY_FLG:
	  	//set_anckeytongdelay();
	  	break;

      case TWSSYNCCMD_AUTO_ANC_FLG:
        #ifdef __POWERONAUTOOPENANC__
		TRACE(1,"p_buff[2] = %d  apps_data.poweronautoancflg = %d", p_buff[3],apps_data.poweronautoancflg);
		if(apps_data.poweronautoancflg == true)
		{
			if(p_buff[1] == true)
			{
			   twscmdsync[0] = TWSSYNCCMD_ANCON_MASTER_FLG;
			   app_ibrt_customif_test1_cmd_send(twscmdsync,1);
			}
			apps_data.poweronautoancflg = false;
		}
        #endif	  	
	  	break;
		

      case AUTO_PLAY_TEST:
	  	earautoplayflg = p_buff[1];
		if(length>2)
		{
		   #if 0
           if(earinflg == true)
		   {
              //app_ibrt_ui_ware_up_event_test();
              if(apps_data.earineventflg == true)
			  {
			     //apps_data.earineventflg = false;
				 //osDelay(100);
                 //app_ibrt_if_event_entry(IBRT_WEAR_UP_EVENT);
                 fjh_thread_msg_send(THREAD_MSG_APPS_UISWITCHERROR);
			  }
		   }
		   #endif
           //?¡À?¨²¦Ì??¡¥¡Á¡Â
		   if(p_buff[2] == 1)
		   {
		       apps_data.otherearstatus = true;
			   #ifdef __HFPAUTOTOPHONE__
			   earinout_delay_set_time(3000);
			   #endif
		   }
		   else
		   {
		       apps_data.otherearstatus = false;
			   #ifdef __HFPAUTOTOPHONE__
               earinout_delay_set_time(3000);
			   #endif
		   }
		}
	  	break;


   
      case TWSSYNCCMD_POWEROFF:
	  	app_shutdown();
	  	break;
		

	  case TWSSYNCCMD_CONNECTEDLED:
	  	app_status_indication_set(APP_STATUS_INDICATION_CONNECTED);
	  	break;
		

	  case TWSSYNCCMD_INCOMCALLLED:
	  	app_status_indication_set(APP_STATUS_INDICATION_INCOMINGCALL);
	  	break;
		

	  case TWSSYNCCMD_TWSSLAVERCONNECTTONE:
	  	 //if(apps_data.leftrighttone_flg == true)
		 //{
		 //	 twscmdsync[0] = TWSSYNCCMD_TWSMASTERCONNECTTONE;
		 //	 app_ibrt_customif_test1_cmd_send(twscmdsync,10);
		 //}
		 apps_data.leftrighttone_flg = true;
	  	break;
		

	  case TWSSYNCCMD_TWSMASTERCONNECTTONE:
	  //	app_voice_report(APP_STATUS_INDICATION_ALEXA_STOP,0);
	  	break;
		
#ifdef __GAMMODE__
	  case TWSSYNCCMD_GAMEMUSICTEST:
	  	
	  	if(p_buff[1]==0)
		{
            musicmodeset();
		}
		else
		{
            gamemodeset();
		}
        #ifdef __POWERONAUTOOPENANC__
		TRACE(1,"p_buff[2] = %d  apps_data.poweronautoancflg = %d", p_buff[3],apps_data.poweronautoancflg);
		if(apps_data.poweronautoancflg == true)
		{
			if(p_buff[2] == true)
			{
               //¡ä¨°?aANC.........
               //app_anc_switch(APP_ANC_MODE1);  
			   //fjh_thread_msg_send(THREAD_MSG_APPS_EVENT_ANCSWITCH);
			   twscmdsync[0] = TWSSYNCCMD_ANCON_MASTER_FLG;
			   app_ibrt_customif_test1_cmd_send(twscmdsync,1);
			}
			apps_data.poweronautoancflg = false;
		}
        #endif
	  	break;
#endif		
		

	  case TWSSYNCCMD_DISCONNECTERROR:
	  	app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);
	  	app_voice_report(APP_STATUS_INDICATION_DISCONNECTED,0);
		app_voice_report(APP_STATUS_INDICATION_BOTHSCAN,0);
		//app_start_10_second_timer(APP_POWEROFF_TIMER_ID); 
	  	break;


	  case TWSSYNCCMD_EARINSTATUS:
		  if(p_buff[1] == true)
		  {
			  leftringear_set_com();
		  }
		  else
		  {
			 leftringear_claer_com();
		  }
	  	break;


	  case TWSSYNCCMD_TWSCONNECTTONE:
	  	if(apps_data.poweron_leftright_flg == true)
		{
           app_voice_report(APP_STATUS_INDICATION_ALEXA_STOP,0);
		}
		else
		{
            if(p_buff[2] == true)
			{
               app_voice_report(APP_STATUS_INDICATION_ALEXA_STOP,0);
			}
			else if(apps_data.leftrighttone_flg == true)
			{
                if(p_buff[1] == true)
				{
                   app_voice_report(APP_STATUS_INDICATION_ALEXA_STOP,0);
				}
			}	
		}
		/*
		#ifdef __POWERONAUTOOPENANC__
		TRACE(1,"p_buff[3] = %d  apps_data.poweronautoancflg = %d", p_buff[3],apps_data.poweronautoancflg);
		if(apps_data.poweronautoancflg == true)
		{
			if(p_buff[3] == true)
			{
               //¡ä¨°?aANC.........
               //app_anc_switch(APP_ANC_MODE1);  
			   fjh_thread_msg_send(THREAD_MSG_APPS_EVENT_ANCSWITCH);
			}
			apps_data.poweronautoancflg = false;
		}
		#endif
		*/
	  	break;


		case TWSSYNCCMD_DISCONNECTERROR03:
		  TRACE(1,"set apps_data.disconnet03errorflg...............");
		  #ifdef __HFPAUTOTOPHONE__
		  earinout_delay_close_time();
		  #endif
		  apps_data.leftrighttone_flg = true;
		  apps_data.disconnet03errorflg = false;
		  #ifdef __POWERONAUTOOPENANC__
		  apps_data.poweronautoancflg = true;
		  #endif
          //1?¡À?anc............................................
		  fjh_thread_msg_send(THREAD_MSG_APPS_ANCLOCALOFF);
          if(p_buff[1] == 1)
		  {
			  apps_data.twsdisconnctedenterpairing = true;
		  }
		  break;
		  

		 case TWSSYNCCMD_TWSVOLUMEFLG:
			repeortlongleftflg = p_buff[1];
			repeortlongrightflg = p_buff[2];		 	
		 	break;


        #ifdef __HFPAUTOTOPHONE__
		case TWSSYNCCMD_TWSHFP_WEARDOWN_FLG:
		if(earinflg == false)
		{
            //?D??¦Ì?¨º??¨²..............................................
            send_key_event(HAL_KEY_CODE_PWR,HAL_KEY_EVENT_HFP_EAROUT);	
		}
	    break;
		#endif	



		case TWSSYNCCMD_ANCON_MASTER_FLG:
			fjh_thread_msg_send(THREAD_MSG_APPS_EVENT_ANCSWITCH);
			break;


		default:break;
   }
   TRACE(1,"%s", __func__);
}

static void app_ibrt_customif_test2_cmd_send(uint8_t *p_buff, uint16_t length)
{
    TRACE(1,"%s", __func__);
}

static void app_ibrt_customif_test2_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    tws_ctrl_send_rsp(APP_IBRT_CUSTOM_CMD_TEST2, rsp_seq, NULL, 0);
    TRACE(1,"%s", __func__);

}

static void app_ibrt_customif_test2_cmd_send_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    TRACE(1,"%s", __func__);

}

static void app_ibrt_customif_test2_cmd_send_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    TRACE(1,"%s", __func__);
}
#endif
#if defined(USE_LOWLATENCY_LIB)
extern void app_low_latency_sync_status_receive(void *info, uint16_t length);

void app_ibrt_lly_cmd(uint8_t *p_buff, uint16_t length)
{

    if(length&&p_buff){
        tws_ctrl_send_cmd(APP_IBRT_CUSTOM_CMD_LLY_SYNC, (uint8_t*)p_buff, length);
    }

}

void app_ibrt_lly_sync_receive_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    app_low_latency_sync_status_receive(p_buff, length);
}

void app_ibrt_lly_sync_receive_handler_rsp(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    TRACE(1,"%s", __func__);
}

void app_ibrt_lly_cmd_send(uint8_t *p_buff, uint16_t length)
{
    TRACE(3,"%s  %x %d", __func__, p_buff, length);
    if(length&&p_buff){
        app_ibrt_send_cmd_without_rsp(APP_IBRT_CUSTOM_CMD_LLY_SYNC, p_buff, length);
    }
}
#endif
