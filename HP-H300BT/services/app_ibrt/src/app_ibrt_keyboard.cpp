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
#include "cmsis_os.h"
#include <string.h>
#include "app_ibrt_keyboard.h"
#include "app_tws_ibrt_trace.h"
#include "app_ibrt_if.h"
#include "app_ibrt_ui_test.h"
#include "app_tws_ibrt_cmd_handler.h"
#include "app_tws_ctrl_thread.h"
#include "btapp.h"
#include "apps.h"
#ifdef __FJH_APPS__
#include "fjh_apps.h"
#include "fjh_apps_thread.h"
#endif
#include "app_anc.h"
#include "app_tws_if.h"
#include "app_ibrt_ui_test_cmd_if.h"

#include "bleapp_section.h"



#if defined(IBRT)
#ifdef SUPPORT_SIRI
//extern int app_hfp_siri_report();
extern int app_hfp_siri_voice(bool en);
extern int open_siri_flag;
#endif

#ifdef IBRT_SEARCH_UI
bool leftdownflg = false;
bool rightdownflg = false;
bool leftlongflg = false;
bool righlongflg = false;

bool repeortlongleftflg = false;
bool repeortlongrightflg = false;

extern struct BT_DEVICE_T  app_bt_device;
extern void app_otaMode_enter(APP_KEY_STATUS *status, void *param);
extern int app_nvrecord_rebuild(void);
bool siri_open_left_flg;
bool siri_open_right_flg;
bool earleftinflg = false;
bool earrightinflg = false;
extern void app_ibrt_ui_local_volume_down_test(void);
extern void app_ibrt_ui_local_volume_up_test(void);
extern void app_ibrt_customif_test1_cmd_send(uint8_t *p_buff, uint16_t length);



bool key_set_com(uint8_t data)
{
	if(data == 0x06)
	{
	   bt_key_handle_func_ans();
	}
	else if(data == 0x07)
	{
       bt_key_handle_func_up();
	}
	else if(data == 0x05)
	{
	   //if((bleappdata.leftclicldata == 0x06)||(bleappdata.rightclicldata == 0x06)||(bleappdata.leftdoubleclicldata == 0x06)||(bleappdata.rightdoubleclicldata == 0x06)||(bleappdata.lefttrubleclicldata == 0x06)||(bleappdata.righttrubleclicldata == 0x06)||(bleappdata.leftlongdata == 0x06)||(bleappdata.rightlongdata == 0x06)||(bleappdata.leftfourclicldata == 0x06)||(bleappdata.rightfourclicldata == 0x06))
	   {
	         bt_key_handle_func_play_stop();
	   }
	   //else
	   {
            // bt_key_handle_func_click_left(); 
	   }  
	}
	else if(data == 0x03)
	{
	   bt_key_handle_func_backward();
	}
	else if(data == 0x04)
	{
	   bt_key_handle_func_forward();
	}
	else if(data == 0x02)
	{
		bt_key_handle_func_siri();
	}	
	else if(data == 0x08)
	{
		bt_key_handle_func_lastcall();
	}			
	else if(data == 0x09)
	{
	    if(bt_key_handle_func_long_left())
	    {
		    app_ibrt_ui_local_volume_up_test(); 
	    }
	}
	else if(data == 0x0A)
	{
	    if(bt_key_handle_func_long_left())
	    {
		    app_ibrt_ui_local_volume_down_test();
	    }
	}	
	else if(data == 0x0B)
	{
		bt_key_handle_func_reject();
	}	
	else if(data == 0x0D)
	{
         #ifdef __GAMMODE__
	     gamemusicmode_switch();  
         #endif

	}
	else if(data == 0x00)
	{

	}
	else
	{
           return true;
	}
    return false;
}


extern uint8_t  app_poweroff_flag;
extern bool poweroff_error_flg;
void app_ibrt_search_ui_handle_key(APP_KEY_STATUS *status, void *param)
{
   // ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    TRACE(3,"%s %d,%d",__func__, status->code, status->event);
    switch(status->event)
    {


	   case APP_KEY_EVENT_LONGLONGPRESS:
		 if(!app_tws_ibrt_mobile_link_connected())
		 {
			app_poweroff_flag = true;
	        #ifdef __FJH_APPS__
			fjh_thread_msg_send(THREAD_MSG_APPS_EVENT_POWEROFF);
	        #endif    
		 }
		 else if(poweroff_read())
		 {
			app_poweroff_flag = true;
	        #ifdef __FJH_APPS__
			fjh_thread_msg_send(THREAD_MSG_APPS_EVENT_POWEROFF);
	        #endif   
		 }
	   break;


	
       case APP_KEY_EVENT_REPEAT_LEFT:
	   if(app_tws_ibrt_mobile_link_connected())
	   {	
          if(poweroff_error_flg == true)
          {
             if(app_poweroff_flag == false)
             {
				 if(poweroff_read())
				 {
				    poweroff_error_flg = false;
					app_poweroff_flag = true;
                    #ifdef __FJH_APPS__
					fjh_thread_msg_send(THREAD_MSG_APPS_EVENT_POWEROFF);
                    #endif	 
					return;
				 }

			 }
		  }

	   
	      if(repeortlongleftflg == true)
	      {
	         if(volume_cmd_en())
	         {
	           //if(bleappdata.leftlongdata == 0x0a)
	           {
                    //app_ibrt_ui_local_volume_down_test();
                 //   app_bt_volumedown();
	           }
			   //else if(bleappdata.leftlongdata == 0x09)
			   {
                   // app_ibrt_ui_local_volume_up_test(); 
			   }
			   fjh_thread_msg_send(THREAD_MSG_BT_APPS_VOLUEDOWN_TWS_TEST);
	         }
			 
	      }
	   }
	   break;

        case APP_KEY_EVENT_REPEAT_RIGHT:
	    if(app_tws_ibrt_mobile_link_connected())
	    {	
		   if(poweroff_error_flg == true)
		   {
			  if(app_poweroff_flag == false)
			  {
				  if(poweroff_read())
				  {
					 poweroff_error_flg = false;
					 app_poweroff_flag = true;
		             #ifdef __FJH_APPS__
					 fjh_thread_msg_send(THREAD_MSG_APPS_EVENT_POWEROFF);
		             #endif   
					 return;
				  }
			  }
		   }

		
	       if(repeortlongrightflg == true)
	       {
	          if(volume_cmd_en())
	          {
				//if(bleappdata.rightlongdata == 0x0a)
				//{
				//	 app_ibrt_ui_local_volume_down_test();
				//}
				//else if(bleappdata.rightlongdata == 0x09)
				{
					 //app_ibrt_ui_local_volume_up_test(); 
					//  app_bt_volumeup();
				}
				fjh_thread_msg_send(THREAD_MSG_BT_APPS_VOLUEUP_TWS_TEST);
	          }
			  
	       }	
	    }
	    break;


		case APP_KEY_EVENT_CLICK_LEFT:   
			#if 0
			if(app_tws_ibrt_mobile_link_connected())
			{
                 if(key_set_com(bleappdata.leftclicldata)== true)
			     {
                     // bt_key_handle_func_click_left();
			     }
				 
			     if((bleappdata.leftclicldata == 0x06)||(bleappdata.rightclicldata == 0x06)||(bleappdata.leftdoubleclicldata == 0x06)||(bleappdata.rightdoubleclicldata == 0x06)||(bleappdata.lefttrubleclicldata == 0x06)||(bleappdata.righttrubleclicldata == 0x06)||(bleappdata.leftlongdata == 0x06)||(bleappdata.rightlongdata == 0x06)||(bleappdata.leftfourclicldata == 0x06)||(bleappdata.rightfourclicldata == 0x06))
			     {

				 }
				 else
				 {
                     bt_key_handle_func_click_left();
				 }
			}
			#endif
			  bt_key_handle_func_click_left();
			break;
		
		case APP_KEY_EVENT_CLICK_RIGHT:
			#if 0
			if(app_tws_ibrt_mobile_link_connected())
			{
                 if(key_set_com(bleappdata.rightclicldata)== true)
			     {
                     // bt_key_handle_func_click_right();
			     }
				 
			     if((bleappdata.leftclicldata == 0x06)||(bleappdata.rightclicldata == 0x06)||(bleappdata.leftdoubleclicldata == 0x06)||(bleappdata.rightdoubleclicldata == 0x06)||(bleappdata.lefttrubleclicldata == 0x06)||(bleappdata.righttrubleclicldata == 0x06)||(bleappdata.leftlongdata == 0x06)||(bleappdata.rightlongdata == 0x06)||(bleappdata.leftfourclicldata == 0x06)||(bleappdata.rightfourclicldata == 0x06))
			     {

				 }
				 else
				 {
                     bt_key_handle_func_click_left();
				 }	 
			}
			#endif
			bt_key_handle_func_click_right();
			break;
		
		case APP_KEY_EVENT_DOUBLECLICK_LEFT:
			#if 0
			if(app_tws_ibrt_mobile_link_connected())
			{
			    if(key_set_com(bleappdata.leftdoubleclicldata)== true)
			   	{
                     // bt_key_handle_func_double_left();
			   	}

			if((bleappdata.leftclicldata == 0x07)||(bleappdata.rightclicldata == 0x07)||(bleappdata.leftdoubleclicldata == 0x07)||(bleappdata.rightdoubleclicldata == 0x07)||(bleappdata.lefttrubleclicldata == 0x07)||(bleappdata.righttrubleclicldata == 0x07)||(bleappdata.leftlongdata == 0x07)||(bleappdata.rightlongdata == 0x07)||(bleappdata.leftfourclicldata == 0x07)||(bleappdata.rightfourclicldata == 0x07))
			{
			
			}
			else
			{
				bt_key_handle_func_double_left();
			}		
			}
			#endif

			bt_key_handle_func_double_left();
			break;
		
		case APP_KEY_EVENT_DOUBLECLICK_RIGHT:
			#if 0
			if(app_tws_ibrt_mobile_link_connected())
			{
			    if(key_set_com(bleappdata.rightdoubleclicldata)== true)
			    {
                     //  bt_key_handle_func_double_right();
			    }
			if((bleappdata.leftclicldata == 0x07)||(bleappdata.rightclicldata == 0x07)||(bleappdata.leftdoubleclicldata == 0x07)||(bleappdata.rightdoubleclicldata == 0x07)||(bleappdata.lefttrubleclicldata == 0x07)||(bleappdata.righttrubleclicldata == 0x07)||(bleappdata.leftlongdata == 0x07)||(bleappdata.rightlongdata == 0x07)||(bleappdata.leftfourclicldata == 0x07)||(bleappdata.rightfourclicldata == 0x07))
			{
			
			}
			else
			{
				bt_key_handle_func_double_left();
			}					
			}
			#endif

			bt_key_handle_func_double_right();
			break;
		
		case APP_KEY_EVENT_TRIPLECLICK_LEFT:
			#if 0
			if(app_tws_ibrt_mobile_link_connected())
			{
			    if(key_set_com(bleappdata.lefttrubleclicldata)== true)
			    {
                       bt_key_handle_func_truble_left();
			    }
			}
			#endif
#ifdef __GAMMODE__
			gamemusicmode_switch();	
#endif	

			//bt_key_handle_func_truble_left();
		   break;
			
		
		case APP_KEY_EVENT_TRIPLECLICK_RIGHT:
			#if 0
			if(app_tws_ibrt_mobile_link_connected())
			{
			    if(key_set_com(bleappdata.righttrubleclicldata)== true)
			    {
                       bt_key_handle_func_truble_right();	
			    }
			}
			#endif
			bt_key_handle_func_truble_right();	
			break;
			

		case APP_KEY_EVENT_ULTRACLICK_LEFT:
			#if 0
			if(app_tws_ibrt_mobile_link_connected())
			{
			    if(key_set_com(bleappdata.leftfourclicldata)== true)
			    {
                       #ifdef __GAMMODE__
					   gamemusicmode_switch();	
                       #endif
			    }
			}	
			#endif
            #ifdef __GAMMODE__
			gamemusicmode_switch();	
            #endif			
			break;
			

		case APP_KEY_EVENT_ULTRACLICK_RIGHT:
			if(app_tws_ibrt_mobile_link_connected())
			{
			    if(key_set_com(bleappdata.rightfourclicldata)== true)
			    {
                       #ifdef __GAMMODE__
					   gamemusicmode_switch();	
                       #endif
			    }
			}			
			break;


        case APP_KEY_EVENT_LONGPRESS_LEFT:
			if(app_tws_ibrt_mobile_link_connected())
			{
                repeortlongleftflg = bt_key_handle_func_long_left();
				/*
                if((bleappdata.leftlongdata == 0x09)||(bleappdata.leftlongdata == 0x0a))
                {

				}
				else
				{
                    repeortlongleftflg = false;
				}
				
                if(key_set_com(bleappdata.leftlongdata)== true)
                {

				}
				*/
			}
	    break;
			

        case APP_KEY_EVENT_LONGPRESS_RIGHT:
			if(app_tws_ibrt_mobile_link_connected())
			{
			    repeortlongrightflg = bt_key_handle_func_long_right();
				/*
                if((bleappdata.rightlongdata == 0x09)||(bleappdata.rightlongdata == 0x0a))
                {

				}
				else
				{
                    repeortlongrightflg = false;
				}			
                if(key_set_com(bleappdata.rightlongdata)== true)
                {
                
				}	*/		   
			}
	    break;
			

        case APP_KEY_EVENT_UP_LEFT:
             poweroff_error_flg = false;
		
            repeortlongleftflg = false;
            if(app_tws_ibrt_tws_link_connected())
	        {
		        twscmdsync[0] = TWSSYNCCMD_TWSVOLUMEFLG;
		        twscmdsync[1] = repeortlongleftflg;
		        twscmdsync[2] = repeortlongrightflg;
		        app_ibrt_customif_test1_cmd_send(twscmdsync,3);
	        }	
	    break;
			

        case APP_KEY_EVENT_UP_RIGHT:

              poweroff_error_flg = false;
		
             repeortlongrightflg = false;
             if(app_tws_ibrt_tws_link_connected())
	         {
		        twscmdsync[0] = TWSSYNCCMD_TWSVOLUMEFLG;
		        twscmdsync[1] = repeortlongleftflg;
		        twscmdsync[2] = repeortlongrightflg;
		        app_ibrt_customif_test1_cmd_send(twscmdsync,3);
	         }	
	    break;


        default:break;	



    }

#ifdef TILE_DATAPATH
    if(APP_KEY_CODE_TILE == status->code)
        app_tile_key_handler(status,NULL);
#endif

}
#endif


void leftringear_set_com(void)
{
   if(app_tws_is_left_side())
   {
	   earrightinflg = true;
   }
   else
   {
        earleftinflg = true;
   }
}


void leftringear_claer_com(void)
{
   if(app_tws_is_left_side())
   {
	   earrightinflg = false;
   }
   else
   {
        earleftinflg = false;
   }
}




void app_ibrt_normal_ui_handle_key(APP_KEY_STATUS *status, void *param)
{
/*
    switch(status->event)
    {
        case APP_KEY_EVENT_CLICK:
#if 0
            app_ibrt_ui_tws_switch();
#else
            bt_key_handle_func_click();
#endif
            break;

        case APP_KEY_EVENT_DOUBLECLICK:
            TRACE(0,"double kill");
            app_ibrt_if_enter_freeman_pairing();
            break;

        case APP_KEY_EVENT_LONGPRESS:
            app_ibrt_if_enter_pairing_after_tws_connected();
            break;

        case APP_KEY_EVENT_TRIPLECLICK:
              TRACE(0,"triple press");
#ifdef TILE_DATAPATH
            app_tile_key_handler(status,NULL);
#endif
            break;

        case HAL_KEY_EVENT_LONGLONGPRESS:
            TRACE(0,"long long press");
            app_shutdown();
            break;

        case APP_KEY_EVENT_ULTRACLICK:
            TRACE(0,"ultra kill");
            break;

        case APP_KEY_EVENT_RAMPAGECLICK:
            TRACE(0,"rampage kill!you are crazy!");
            break;

        case APP_KEY_EVENT_UP:
            break;
    }
#ifdef TILE_DATAPATH
    if(APP_KEY_CODE_TILE == status->code)
        app_tile_key_handler(status,NULL);
#endif
*/
}

int app_ibrt_if_keyboard_notify(APP_KEY_STATUS *status, void *param)
{
    if (app_tws_ibrt_tws_link_connected())
    {
        tws_ctrl_send_cmd(APP_TWS_CMD_KEYBOARD_REQUEST, (uint8_t *)status, sizeof(APP_KEY_STATUS));
    }
    return 0;
}

void app_ibrt_send_keyboard_request(uint8_t *p_buff, uint16_t length)
{
    if (app_tws_ibrt_tws_link_connected())
    {
        app_ibrt_send_cmd_without_rsp(APP_TWS_CMD_KEYBOARD_REQUEST, p_buff, length);
    }
}

void app_ibrt_keyboard_request_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    if (app_tws_ibrt_tws_link_connected())
    {
#ifdef IBRT_SEARCH_UI
        app_ibrt_search_ui_handle_key((APP_KEY_STATUS *)p_buff, NULL);
#else
        app_ibrt_normal_ui_handle_key((APP_KEY_STATUS *)p_buff, NULL);
#endif
#ifdef __AI_VOICE__
        app_ibrt_ui_test_voice_assistant_key((APP_KEY_STATUS *)p_buff, NULL);
#endif
    }
}

void app_ibrt_if_start_user_action(uint8_t *p_buff, uint16_t length)
{
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    TRACE(3,"%s role %d code %d",__func__, p_ibrt_ctrl->current_role, p_buff[0]);

    if (IBRT_SLAVE == p_ibrt_ctrl->current_role)
    {
        app_ibrt_ui_send_user_action(p_buff, length);
    }
    else
    {
        app_ibrt_ui_perform_user_action(p_buff, length);
    }
}


void app_ibrt_ui_perform_user_action(uint8_t *p_buff, uint16_t length)
{
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    if (IBRT_MASTER != p_ibrt_ctrl->current_role)
    {
        TRACE(2,"%s not ibrt master, skip cmd %02x\n", __func__, p_buff[0]);
        return;
    }

    switch (p_buff[0])
    {
        case IBRT_ACTION_PLAY:
            a2dp_handleKey(AVRCP_KEY_PLAY_INTERNAL);
            break;
        case IBRT_ACTION_PAUSE:
            a2dp_handleKey(AVRCP_KEY_PAUSE_INTERNAL);
            break;
        case IBRT_ACTION_SIRI:
        	if(open_siri_flag == 1)
            {
                TRACE(0,"open siri");
                app_hfp_siri_voice(true);
                open_siri_flag = 0;
            }
            else
            {
                TRACE(0,"evnet none close siri");
                app_hfp_siri_voice(false);
            }
            break;

        case IBRT_ACTION_LOCAL_VOLUP:
            app_bt_volumeup();
           // app_ibrt_sync_volume_info();
            break;
        case IBRT_ACTION_LOCAL_VOLDN:
            app_bt_volumedown();
           // app_ibrt_sync_volume_info();
            break;			
        default:
            TRACE(2,"%s unknown user action %d\n", __func__, p_buff[0]);
            break;
    }
}

#endif

