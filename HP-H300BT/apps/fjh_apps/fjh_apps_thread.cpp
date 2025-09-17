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
#include "stdio.h"
#include "cmsis_os.h"
#include "list.h"
#include "string.h"

#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_bootmode.h"

#include "audioflinger.h"
#include "fjh_apps.h"
#include "fjh_apps_thread.h"
#include "app_thread.h"

#include "app_utils.h"
#include "me_api.h"
#include "os_api.h"
#include "app_tws_ibrt.h"
#include "app_ibrt_customif_cmd.h"
#include "apps.h"
//#include "app_box.h"
#ifdef __FJH_BOX_MODULE__
#include "app_box2.h"
#endif
#include "app_tws_if.h"
#include "app_ibrt_ui.h"
#include "app_ibrt_if.h"
#include "app_anc.h"
#ifdef __BLE_S80__
#include "ble_cmd.h"
#endif
//#include "jsa1228.h"

#ifdef LONGWEI
#include "sp_audio.h"
#endif

#include "btapp.h"
#include "hal_key.h"

#include "app_det.h"

#ifdef __BLEAPPKEY__
#include "bleapp_section.h"
#endif
#include "app_ble_mode_switch.h"

//#include "app_battery.h"
//#include "ble_cmd.h"



uint8_t twscmdsync[34]={0};
uint8_t box_status = box_status_unknow;


void fjhhds_delay_close_time(void);
static void fjhhds_delay_timehandler(void const * param);
osTimerDef(FJHHDS_DELAY, fjhhds_delay_timehandler);
static osTimerId       fjhhds_delay_timer = NULL;

static void fjhhds_delay_timehandler(void const * param)
{
	//ibrt_ctrl_t *p_ibrt_ctrl = app_ibrt_if_get_bt_ctrl_ctx();

   //低电时强制关机.............
   if(apps_data.tws_switch_status_flg == true)
   {
	   app_ibrt_ui_event_entry(IBRT_CLOSE_BOX_EVENT);
	   osDelay(1000);
	   app_shutdown();
       return;
   }

   if(apps_data.twsslaverbug01flg == true)
   {
       apps_data.twsslaverbug01flg = false;
       #ifdef __POWERON_RECONNECT_PAIIRNG__
       fjh_thread_msg_send(THREAD_MSG_APPS_POWER_ON_10S_PAIRING);
       #endif  	   
       app_ibrt_ui_event_entry(IBRT_PEER_FETCH_OUT_EVENT);
   }

   #if 0
   else if(apps_data.errorpatch01flg == true)
   {
          apps_data.errorpatch01flg = false;
		  if (app_tws_ibrt_slave_ibrt_link_connected())
		  {
			  if((apps_data.earineventflg == true)&(earinflg == true))
			  {
			     app_ibrt_if_event_entry(IBRT_WEAR_UP_EVENT);
			  }
	      }
		  apps_data.earineventflg = false;
   }
   #endif

}


void fjhhds_delay_set_time(uint32_t millisec)
{
  if (fjhhds_delay_timer == NULL)
  {
	 fjhhds_delay_timer	= osTimerCreate(osTimer(FJHHDS_DELAY), osTimerOnce, NULL);
	 osTimerStop(fjhhds_delay_timer);
  } 
     osTimerStart(fjhhds_delay_timer, millisec);
}



void fjhhds_delay_close_time(void)
{
		if (fjhhds_delay_timer != NULL)
		{
		   osTimerStop(fjhhds_delay_timer);
		} 
}



#ifdef __POWERON_RECONNECT_PAIIRNG__

//bool linklosspatchflg = false;

void fj10s_delay_close_time(void);
static void fj10s_delay_timehandler(void const * param);
osTimerDef(FJH10S_DELAY, fj10s_delay_timehandler);
static osTimerId       fjh10s_delay_timer = NULL;
static void fj10s_delay_timehandler(void const * param)
{
TRACE(0,"fj10s_delay_timehandler = %d \n",apps_data.recnecterrorenterpairflg);

//if(apps_data.recnecterrorenterpairflg == true)
if(box_status_test2() == box_status_open)
{

	//  app_ibrt_ui_event_entry(IBRT_FETCH_OUT_EVENT);
    //  linklosspatchflg = false;
	//  app_ibrt_ui_event_entry(IBRT_AUDIO_PLAY);


	 apps_data.recnecterrorenterpairflg = false;

     if(!app_tws_ibrt_tws_link_connected())
     {
	    if(!app_tws_ibrt_mobile_link_connected())
	    {
	            //app_ibrt_ui_set_enter_pairing_mode(IBRT_MOBILE_CONNECTED);
	            //app_ibrt_if_pairing_mode_refresh();
                #if 1
	            app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);
                #ifdef MEDIA_PLAYER_SUPPORT
	            app_voice_report(APP_STATUS_INDICATION_BOTHSCAN,0);
                #endif	  
	            app_tws_ibrt_set_access_mode(BTIF_BAM_GENERAL_ACCESSIBLE);
				//app_bt_accessmode_set(BTIF_BAM_GENERAL_ACCESSIBLE);
                
	            app_ibrt_ui_set_enter_pairing_mode(IBRT_CONNECT_MOBILE_FAILED);
	            //app_ibrt_ui_event_entry(IBRT_FREEMAN_PAIRING_EVENT);
				#endif	
	    }
	 }
	 else
	 {
          if(app_tws_is_master_mode())
          {
            if(!app_tws_ibrt_mobile_link_connected())
            {
               //app_ibrt_if_pairing_mode_refresh();
	           //app_ibrt_ui_set_enter_pairing_mode(IBRT_MOBILE_CONNECTED);
               #if 1
		       app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);
               #ifdef MEDIA_PLAYER_SUPPORT
		       app_voice_report(APP_STATUS_INDICATION_BOTHSCAN,0);
               #endif	  
			   app_tws_ibrt_set_access_mode(BTIF_BAM_GENERAL_ACCESSIBLE);
			   //app_bt_accessmode_set(BTIF_BAM_GENERAL_ACCESSIBLE);
               
		       app_ibrt_ui_set_enter_pairing_mode(IBRT_CONNECT_MOBILE_FAILED);
		       //app_ibrt_ui_event_entry(IBRT_FREEMAN_PAIRING_EVENT);
			   #endif	
            }
		  } 
	 }
}
}


void fjh10S_delay_set_time(uint32_t millisec)
{
  if (fjh10s_delay_timer == NULL)
  {
	 fjh10s_delay_timer	= osTimerCreate(osTimer(FJH10S_DELAY), osTimerOnce, NULL);
	 osTimerStop(fjh10s_delay_timer);
  } 
     osTimerStart(fjh10s_delay_timer, millisec);
}



void fj10s_delay_close_time(void)
{
		if (fjh10s_delay_timer != NULL)
		{
		   osTimerStop(fjh10s_delay_timer);
		} 
		apps_data.recnecterrorenterpairflg = false;
}
#endif




#ifdef __CLOSEBOX_PATCH__
extern void app_set_disconnecting_all_bt_connections(bool isEnable);
void LinkDisconnectDirectly_test(void)
{
TRACE(0,"disconnectall......");
app_ibrt_ui_t *p_ibrt_ui = app_ibrt_ui_get_ctx();
TRACE(0,"p_ibrt_ui->active_event.event = %d ",p_ibrt_ui->active_event.event);

if((p_ibrt_ui->active_event.event == IBRT_RECONNECT_EVENT)||(p_ibrt_ui->active_event.event == IBRT_PEER_RECONNECT_EVENT))
{
	p_ibrt_ui->active_event.event = IBRT_CLOSE_BOX_EVENT;
}

 app_ibrt_ui_judge_scan_type(IBRT_OPEN_BOX_TRIGGER,NO_LINK_TYPE,IBRT_UI_NO_ERROR);
 app_ibrt_ui_event_entry(IBRT_OPEN_BOX_EVENT);
 osDelay(50);
 app_ibrt_ui_judge_scan_type(IBRT_CLOSE_BOX_TRIGGER,NO_LINK_TYPE,IBRT_UI_NO_ERROR);
 app_ibrt_ui_event_entry(IBRT_CLOSE_BOX_EVENT);   

return;


}


void boxclose_delay_set_time(uint32_t millisec);
static void boxclose_delay_timehandler(void const * param);
osTimerDef(BOXCLOSE_DELAY, boxclose_delay_timehandler);
static osTimerId       boxclose_delay_timer = NULL;
static void boxclose_delay_timehandler(void const * param)
{
	TRACE(0,"btpairing_testflg := %d",btpairing_testflg);

	if((box_status_test2()==box_status_close))
	{
          #ifdef __CLOSEBOX_TWS_SWITCH_PATCH__
		  if(twsswitchok_enterclosebox == true)
		  {
		           twsswitchok_enterclosebox = false;
                   app_ibrt_ui_event_entry(IBRT_CLOSE_BOX_EVENT);
                   boxclose_delay_set_time(3000);
				   return;
		  }
          #endif

	
		  if(app_tws_ibrt_tws_link_connected()||app_tws_ibrt_mobile_link_connected()||app_tws_ibrt_slave_ibrt_link_connected())
		  {
			 LinkDisconnectDirectly_test();
			 //app_ibrt_ui_event_entry(IBRT_CLOSE_BOX_EVENT);
			 boxclose_delay_set_time(3000);
		  }
		  else
		  {
#ifdef __CLOSEBOX_BLEDATA_SET__
			  bleapp_data_set();
#endif
#ifdef __BLE_S80__
			  app_ble_force_switch_adv(BLE_SWITCH_USER_BOX, false);
#endif


		  }
		  //app_ibrt_ui_event_entry(IBRT_CLOSE_BOX_EVENT);
	}
}




void boxclose_delay_set_time(uint32_t millisec)
{
  if (boxclose_delay_timer == NULL)
  {
	 boxclose_delay_timer	= osTimerCreate(osTimer(BOXCLOSE_DELAY), osTimerOnce, NULL);
	 osTimerStop(boxclose_delay_timer);
  } 
     osTimerStart(boxclose_delay_timer, millisec);
}



void boxclose_delay_close_time(void)
{
		if (boxclose_delay_timer != NULL)
		{
		   osTimerStop(boxclose_delay_timer);
		} 
}
#endif


/***********************************************************************************************************/

#ifdef __SCO_TWS_SWITCH_PATCH__
bool sco_mode_flg = false;
#endif

#ifdef __INOUTBOX_PATCH__
void inoutbox_delay_set_time(uint32_t millisec);
static void inoutbox_delay_timehandler(void const * param);
osTimerDef(INOUTBOX_DELAY, inoutbox_delay_timehandler);
static osTimerId       inoutboxclose_delay_timer = NULL;
static void inoutbox_delay_timehandler(void const * param)
{
	ibrt_ctrl_t *p_ibrt_ctrl = app_ibrt_if_get_bt_ctrl_ctx();
	TRACE(0,"apps_data.peerinboxstatus = %d  box_status.boxin_flg = %d",apps_data.peerinboxstatus,earinboxflg);
	
	if((box_status_test2()==box_status_open))
	{
	   if(p_ibrt_ctrl->current_role == IBRT_MASTER)
	   {
		   if(app_tws_ibrt_mobile_link_connected()&(app_tws_ibrt_tws_link_connected()))
		   {
			   if((apps_data.peerinboxstatus == false)&(earinboxflg == true))
			   {
                  #ifdef __SCO_TWS_SWITCH_PATCH__
				  if(sco_mode_flg == true)
                  #endif
                  {
				     if(app_tws_if_trigger_role_switch())
				     {
					   TRACE(0,"TWS SWITCH OK.........");
				     }	
				     else
				     {
                       inoutbox_delay_set_time(2000);
				     }
                  }
			   }
		   }
	   }	   
	}
}




void inoutbox_delay_set_time(uint32_t millisec)
{
  if (inoutboxclose_delay_timer == NULL)
  {
	 inoutboxclose_delay_timer	= osTimerCreate(osTimer(INOUTBOX_DELAY), osTimerOnce, NULL);
	 osTimerStop(inoutboxclose_delay_timer);
  } 
     osTimerStart(inoutboxclose_delay_timer, millisec);
}



void inoutbox_delay_close_time(void)
{
		if (inoutboxclose_delay_timer != NULL)
		{
		   osTimerStop(inoutboxclose_delay_timer);
		} 
}
#endif
/***********************************************************************************************************/


extern void app_bt_volumedown();
extern void app_bt_volumeup();



int fjh_thread_msg_handler(APP_MESSAGE_BODY * msg)
{
	ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();

	TRACE(0,"msg->message_id = %x \n",msg->message_id);
#ifdef __FJH_BOX_MODULE__
    if(box_status_test2()==box_status_close)  
	{
        if((msg->message_id ==THREAD_MSG_APPS_EVENT_BOXCLOSE)||(msg->message_id ==THREAD_MSG_APPS_EVENT_BOXPOWERON)||(msg->message_id ==THREAD_MSG_APPS_EVENT_POWEROFF)||(msg->message_id ==THREAD_MSG_APPS_EVENT_OUTBOX)||(msg->message_id ==THREAD_MSG_APPS_EVENT_CLOSE_BOX_PATCH_TEST)||(msg->message_id ==THREAD_MSG_APPS_EVENT_INOUT_BOX_PATCH_TEST))
		{

		}
		else
		{
           return 0;
		}
	}
#endif	






    switch(msg->message_id)
	{


#ifdef __POWERON_RECONNECT_PAIIRNG__
				case THREAD_MSG_APPS_POWER_ON_10S_PAIRING:
					fjh10S_delay_set_time(12000);
					break;
		
				case THREAD_MSG_APPS_POWER_ON_10S_STOP:
					fj10s_delay_close_time();
					break;
#endif		



	
		case THREAD_MSG_APPS_EVENT_CLOSE_BOX_PATCH2:
		slaverbug_delay_close_time();
		app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);
		app_tws_ibrt_set_access_mode(BTIF_BAM_GENERAL_ACCESSIBLE);
		app_ibrt_ui_set_enter_pairing_mode(IBRT_MOBILE_CONNECTED);
        #ifdef GFPS_ENABLED
		app_enter_fastpairing_mode();
        #endif
		app_start_10_second_timer(APP_PAIR_TIMER_ID);
		break;

	    case THREAD_MSG_BT_APPS_EVENT_BTTEST:
			app_enter_bttest_fjh();
		break;
		
        case THREAD_MSG_BT_APPS_EVENT_DUT:
			app_enter_dut_fjh();
		break;

		case THREAD_MSG_BT_APPS_EVENT_CLEANPAIRLIST:
			//app_nvrecord_rebuild_fjh();
			app_nvrecord_rebuild_app(true);
		break;

		case THREAD_MSG_APPS_EVENT_POWERONPAIRING:
			app_poweron_pairing_fjh();
		break;

		case THREAD_MSG_BT_APPS_EVENT_TWS_PAIRING:
			app_tws_pairing_fjh();
		break;


		case THREAD_MSG_APPS_EVENT_ANCSWITCH:
//			app_anc_loop_switch_fjh();
		break;

		case THREAD_MSG_APPS_EVENT_POWEROFF:
			if(app_tws_ibrt_tws_link_connected())
			{
			   twscmdsync[0] = TWSSYNCCMD_POWEROFF;
               app_ibrt_customif_test1_cmd_send(twscmdsync,10);
			   osDelay(50);
			}
			app_shutdown();
			break;

	   case THREAD_MSG_APPS_EVENT_CONNECTSYNCLED:
			if(app_tws_ibrt_tws_link_connected())
			{
			   twscmdsync[0] = TWSSYNCCMD_CONNECTEDLED;
               app_ibrt_customif_test1_cmd_send(twscmdsync,10);
			}	   	
	   	break;


		case THREAD_MSG_APPS_EVENT_INCOMCALLSYNCLED:
			if(app_tws_ibrt_tws_link_connected())
			{
			   twscmdsync[0] = TWSSYNCCMD_INCOMCALLLED;
               app_ibrt_customif_test1_cmd_send(twscmdsync,10);
			}			
			break;


		case THREAD_MSG_BT_APPS_EVENT_OTA:
			app_enter_ota_fjh();
			break;

		case THREAD_MSG_BT_APPS_EVENT_MOBILE_CONNECTED:
			app_mobile_connected_fjh();
			app_stop_10_second_timer(APP_POWEROFF_TIMER_ID);
			apps_data.moblieconnectedpatch03flg = true;
			break;


		case THREAD_MSG_BT_APPS_EVENT_MOBILE_DISCONNECTED:
			app_start_10_second_timer(APP_POWEROFF_TIMER_ID);
			apps_data.moblieconnectedpatch03flg = false; 
			break;

		case THREAD_MSG_APPS_EVENT_BOXPOWERON:
			//box_status = box_status_open;
			#ifdef __CLOSEBOX_PATCH__
			boxclose_delay_close_time();
			#endif
			app_box_poweron_fjh();	
			break;

		case THREAD_MSG_APPS_EVENT_BOXCLOSE:
			//box_status = box_status_off;
			fjhhds_delay_close_time();
			slaverbug_delay_close_time();
			app_box_close_fjh();
			break;	

		case THREAD_MSG_APPS_EVENT_BTDISCONNECT:
			//apps_data.errorpatch01flg = false;
            //fjhhds_delay_set_time(600);
			app_bt_disconnect_fjh();
			break;

		case THREAD_MSG_APPS_EVENT_BTDISCONNECTOK:
			app_bt_disconnect_fjh();
			break;
			
		case THREAD_MSG_APPS_EVENT_BTCONNECT:
			//fjhhds_delay_close_time();
			//apps_data.errorpatch01flg = true;
			app_bt_connect_fjh();
			//fjhhds_delay_set_time(3000);
			break;			

		case THREAD_MSG_APPS_EVENT_BTSCAN:
			app_bt_scan_fjh();
			break;

		case THREAD_MSG_BT_APPS_EVENT_TWS_CONNECTED:
			app_tws_connect_fjh();
			break;

		case THREAD_MSG_BT_APPS_EVENT_TWS_DISCONNECTED:
			app_tws_disconnect_fjh();
			break;

		case THREAD_MSG_APPS_BATTERYLOWPOWEROFF:
			   if(app_tws_ibrt_tws_link_connected())
			   {
				   set_disconnet03errorflg();

			   
				  if(p_ibrt_ctrl->current_role == IBRT_MASTER)
				  {
					 apps_data.tws_switch_status_flg = true;
					 
					 if(app_tws_if_trigger_role_switch())
					 {
						TRACE(0,"TWS SWITCH OK.........");
					 }
					 else
					 {
                        app_ibrt_ui_event_entry(IBRT_CLOSE_BOX_EVENT);
					 }
					 /*
					 else
					 {
						TRACE(0,"TWS SWITCH ERROR.........");
						apps_data.onepowerofftwsswitcherrortime++;
						//强制关机.........
						if(apps_data.onepowerofftwsswitcherrortime>2)
						{
							  app_ibrt_ui_event_entry(IBRT_CLOSE_BOX_EVENT);
							  osDelay(1000);
							  app_shutdown();
							  break;
						}
						osDelay(100);
						//重新进入关机流程...........
#ifdef __FJH_APPS__
						fjh_thread_msg_send(THREAD_MSG_APPS_BATTERYLOWPOWEROFF);
#endif				
					 }
					 */
					 fjhhds_delay_set_time(3000);
				  }
				  else
				  {
					  app_shutdown();
				  }
			   }
			   else
			   {
				   app_shutdown();
			   }					
			break;

		case THREAD_MSG_APPS_EVENT_SLAVERBUG:
			apps_data.twsslaverbug01flg = true;
			fjhhds_delay_set_time(1200);
			break;

		#ifdef __GAMMODE__
		case THREAD_MSG_BT_APPS_EVENT_MUSICGAME_MODE_SWITCH:
			gamemusicmode_switch();
			break;
		#endif

		case THREAD_MSG_APPS_EVENT_DISCONNECTERROR:
			   twscmdsync[0] = TWSSYNCCMD_DISCONNECTERROR;
               app_ibrt_customif_test1_cmd_send(twscmdsync,10);			
			break;


		case THREAD_MSG_APPS_EVENT_INBOX:
			app_box_in_fjh();
			break;

		case THREAD_MSG_APPS_EVENT_OUTBOX:
			app_box_out_fjh();
			break;
			

		case THREAD_MSG_APPS_ANCLOCALOFF:
		//	app_anc_switch_locally(APP_ANC_MODE_OFF);
			break;


        case THREAD_MSG_APPS_UISWITCHERROR:

		break;



#ifdef __CLOSEBOX_PATCH__
        case THREAD_MSG_APPS_EVENT_CLOSE_BOX_PATCH_TEST:
			if((box_status_test2()==box_status_close))
			{
               #ifdef __CLOSEBOX_TWS_SWITCH_PATCH__
			   //if(twsswitchok_enterclosebox == true)
			   {
                   boxclose_delay_set_time(3000);
			   }
			   #else
			       boxclose_delay_set_time(5000);
			   #endif

			
                //3S 鍚庢柇寮�杩炴帴.........
                
			}
		break;
#endif	



       case THREAD_MSG_APPS_EVENT_EQMODE_TEST:
		TRACE(1,".............eqmode is: 0xff");
		DUMP8("0x%02x ", bleappdata.eq_gain, 7);		   
		//set_app_eq_mode(0xff,(float *)eq_gain);
		#ifdef LONGWEI
		set_app_eq_mode(0xff,(char *)bleappdata.eq_gain);
		#endif	
		DUMP8("0x%02x ", (char *)bleappdata.eq_gain, 7);
		//send_key_event(HAL_KEY_CODE_PWR,HAL_KEY_EVENT_EQSWITCH);
	   	break;


	   case THREAD_MSG_APPS_EVENT_PRE_TEST:
	   	if(app_tws_ibrt_mobile_link_connected())
	   	{
              bt_key_handle_func_backward();
		}
		else
		{
             if(app_tws_ibrt_tws_link_connected())
             {
                 if(p_ibrt_ctrl->current_role == IBRT_SLAVE)  
                 { 
                     sync_pre_cmd();
				 }
			 }
		}
	   	break;


	   case THREAD_MSG_APPS_EVENT_NEX_TEST:
	   	if(app_tws_ibrt_mobile_link_connected())
	   	{
              bt_key_handle_func_forward();
		}
		else
		{
             if(app_tws_ibrt_tws_link_connected())
             {
                 if(p_ibrt_ctrl->current_role == IBRT_SLAVE)  
                 {
                     sync_nex_cmd();
				 }
			 }
		} 	
	   	break;


	   case THREAD_MSG_APPS_EVENT_PLAY_TEST:
	   	if(app_tws_ibrt_mobile_link_connected())
	   	{
             bt_key_handle_func_play();
		}
		else
		{
             if(app_tws_ibrt_tws_link_connected())
             {
                 if(p_ibrt_ctrl->current_role == IBRT_SLAVE)  
                 {
                     sync_paly_cmd();
				 }
			 }
		}   	
	   	break;

	   case THREAD_MSG_APPS_EVENT_STOP_TEST:
	   	if(app_tws_ibrt_mobile_link_connected())
	   	{
            bt_key_handle_func_stop();
		}
		else
		{
             if(app_tws_ibrt_tws_link_connected())
             {
                 if(p_ibrt_ctrl->current_role == IBRT_SLAVE)  
                 {
                    sync_stop_cmd();
				 }
			 }
		}   	
	   	break;
		
#ifdef __INOUTBOX_PATCH__
		case THREAD_MSG_APPS_EVENT_INOUT_BOX_PATCH_TEST:

#ifdef __INOUTBOX_STATUS_APP_PATCH__
        if(apps_data.bugflgtest == true)
        {
            apps_data.bugflgtest = false;
		    send_battery_status_to_app();
        }
#endif		
		inoutbox_delay_set_time(2000);
		break;
#endif



        case THREAD_MSG_BT_APPS_EVENT_CLEANPAIRLIST_PHONELIST:
			app_clear_btpailist_fjh(true);
		break;


		case THREAD_MSG_BT_APPS_EVENT_TWS_PAIRING2:
			app_tws_pairing_fjh2();
			break;


		case THREAD_MSG_BT_APPS_VOLUEUP_TWS_TEST:
			app_bt_volumeup();
			break;


		case THREAD_MSG_BT_APPS_VOLUEDOWN_TWS_TEST:
			app_bt_volumedown();
			break;

			

		default:break;
	}

	return 0;
}


void fjh_thread_msg_send(uint32_t msgid)
{
    APP_MESSAGE_BLOCK msg;
    msg.mod_id          = APP_MODUAL_FJHAPPS;
    msg.msg_body.message_id = msgid;
    msg.msg_body.message_ptr = (uint32_t) NULL;
    app_mailbox_put(&msg);
}



void fjh_thread_init(void)
{
  app_set_threadhandle(APP_MODUAL_FJHAPPS, fjh_thread_msg_handler);
}


