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
#include "stdbool.h"
#include "hal_trace.h"
#include "app_pwl.h"
#include "app_status_ind.h"
#include "string.h"
#include "pmu.h"
#include "hal_timer.h"
#ifdef __FJH_BOX_MODULE__
#include "app_box2.h"
#endif
#include "app_thread.h"
#include "app_tws_ibrt.h"
//#include "app_ibrt_customif_cmd.h"
//#include "../../app_ibrt/inc/app_ibrt_customif_cmd.h"
#include "apps.h"
#ifdef __FJH_APPS__
#include "fjh_apps.h"
#include "fjh_apps_thread.h"
#endif


static APP_STATUS_INDICATION_T app_status_call = APP_STATUS_INDICATION_NUM;
static APP_STATUS_INDICATION_T app_status_delay_old = APP_STATUS_INDICATION_NUM;


static APP_STATUS_INDICATION_T app_status = APP_STATUS_INDICATION_NUM;
static APP_STATUS_INDICATION_T app_status_fjh = APP_STATUS_INDICATION_NUM;

static APP_STATUS_INDICATION_T app_status_ind_filter = APP_STATUS_INDICATION_NUM;

#if 0
static const char * const app_status_indication_str[] =
{
    "[POWERON]",
    "[INITIAL]",
    "[PAGESCAN]",
    "[POWEROFF]",
    "[CHARGENEED]",
    "[CHARGING]",
    "[FULLCHARGE]",
    /* repeatable status: */
    "[BOTHSCAN]",
    "[CONNECTING]",
    "[CONNECTED]",
    "[DISCONNECTED]",
    "[CALLNUMBER]",
    "[INCOMINGCALL]",
    "[PAIRSUCCEED]",
    "[PAIRFAIL]",
    "[HANGUPCALL]",
    "[REFUSECALL]",
    "[ANSWERCALL]",
    "[CLEARSUCCEED]",
    "[CLEARFAIL]",
    "[WARNING]",
    "[ALEXA_START]",
    "[ALEXA_STOP]",
    "[GSOUND_MIC_OPEN]",
    "[GSOUND_MIC_CLOSE]",
    "[GSOUND_NC]",
    "[INVALID]",
    "[MUTE]",
    "[TESTMODE]",
    "[TESTMODE1]",
    "[RING_WARNING]",
#ifdef __INTERACTION__	
    "[FINDME]",
#endif	
    "[MY_BUDS_FIND]",
    "[TILE_FIND]",
    "[APP_STATUS_INDICATION_ANC_ON]",
    "[APP_STATUS_INDICATION_AMB_ON]",
    "[APP_STATUS_INDICATION_ANC_OFF]",
    "[GAME_MODE]",  
    "[MUSIC_MODE]", 
    "[TWS_PAIRIRNG]"    



};
#endif

enum
{
    LED_DELAY_NULL = 0,
    LED_BATTERY_LOW,
    DUT_TEST_MODE,
    BOX_IN_MODE,
    DUT_CLEAR_MODE,
    LED_POWER_ON_DELAY,
    LED_CONNECT_DELAY,
    LED_INCOMING_DELAY,
    LED_CONNECTHUXI,
};
uint8_t led_delay_msg = LED_DELAY_NULL;



static void led_delay_timehandler(void const * param);
osTimerDef(LED_GENSORDELAY, led_delay_timehandler);
static osTimerId       led_delay_timer = NULL;

void led_delay_Send_Cmd(uint32_t modeid);

extern void app_ibrt_customif_test1_cmd_send(uint8_t *p_buff, uint16_t length);
static void led_delay_timehandler(void const * param)
{
    TRACE(2,"%s app_status_delay_old = %d",__func__, app_status_delay_old);
//	struct PMU_LED_BR_CFG_T breath_led;
	if(led_delay_msg == LED_POWER_ON_DELAY)
	{
	    led_delay_msg = LED_DELAY_NULL;
		if(app_status_fjh != APP_STATUS_INDICATION_NUM)
		{
          app_status_indication_set(app_status_fjh);
		  app_status_fjh = APP_STATUS_INDICATION_NUM;
		}
	}
}

void led_delay_set_time(uint32_t millisec)
{
  if (led_delay_timer == NULL)
  {
	 led_delay_timer	= osTimerCreate(osTimer(LED_GENSORDELAY), osTimerOnce, NULL);
	 osTimerStop(led_delay_timer);
  } 
     osTimerStart(led_delay_timer, millisec);
}

void led_delay_close(void)
{
   if (led_delay_timer != NULL)
   {
       osTimerStop(led_delay_timer);
   }
}


void led_delay_Send_Cmd(uint32_t modeid)
{
  APP_MESSAGE_BLOCK msg;
  msg.mod_id			= APP_MODUAL_LED_STATIC_DELAY;
  msg.msg_body.message_id = modeid;
  msg.msg_body.message_ptr = (uint32_t) NULL;
  app_mailbox_put(&msg);
}


int app_led_process(APP_MESSAGE_BODY * msg)
{
   led_delay_set_time(2000);
   return 0;
}

void init_leddelay_thread(void)
{
	app_set_threadhandle(APP_MODUAL_LED_STATIC_DELAY, app_led_process);
}




/*
const char *status2str(uint16_t status)
{
    const char *str = NULL;

    if (status >= 0 && status < APP_STATUS_INDICATION_NUM)
    {
        str = app_status_indication_str[status];
    }
    else
    {
        str = "[UNKNOWN]";
    }

    return str;
}
*/


int app_status_indication_filter_set(APP_STATUS_INDICATION_T status)
{
    app_status_ind_filter = status;
    return 0;
}

APP_STATUS_INDICATION_T app_status_indication_get(void)
{
	if(led_delay_msg != LED_DELAY_NULL)
	{
		 return app_status_fjh;
	}	 
    return app_status;
}


APP_STATUS_INDICATION_T app_status_call_indication_get(void)
{
    return app_status_call;
}



//struct PMU_LED_BR_CFG_T breath_led;




extern uint8_t box_status;
extern uint8_t  app_poweroff_flag;
int app_status_indication_set(APP_STATUS_INDICATION_T status)
{
    struct APP_PWL_CFG_T cfg0;
    struct APP_PWL_CFG_T cfg1;


    TRACE(2,"%s %d",__func__, status);


    if (app_status == status)
    {
         return 0;
    }


	if(app_poweroff_flag == true)
	{
        if(status!=APP_STATUS_INDICATION_POWEROFF)
        {
           return 0;
		}
	}

	if((status==APP_STATUS_INDICATION_CHARGING)||(status==APP_STATUS_INDICATION_POWERON)||(status==APP_STATUS_INDICATION_BOTHSCAN)||(status==APP_STATUS_INDICATION_CONNECTED)||(status==APP_STATUS_INDICATION_TESTMODE)||(status==APP_STATUS_INDICATION_TESTMODE1)||(status==APP_STATUS_BT_PAIRING_TEST_MODE)||(status==APP_STATUS_TWS_PAIRING)||(status==APP_STATUS_INDICATION_POWEROFF)||(status==APP_STATUS_INDICATION_PAGESCAN)||(status==APP_STATUS_INDICATION_CLEARSUCCEED)||(status==APP_STATUS_INDICATION_CHARGENEED))
	{
		if(led_delay_msg != LED_DELAY_NULL)
		{
			 app_status_fjh = status;
			 return 0;
		}
	}
	else
	{
          return 0;
	}
	
    if(app_status == APP_STATUS_INDICATION_CHARGING)
    {
       if((status != APP_STATUS_INDICATION_POWERON))
       {
           return 0;
	   }
	}


	if(app_status == APP_STATUS_INDICATION_CLEARSUCCEED)
	{
          return 0;
	}


    if(app_status == APP_STATUS_BT_PAIRING_TEST_MODE)
    {
       if((status != APP_STATUS_INDICATION_POWEROFF))
       {
           return 0;
	   }
	}


   if(app_status == APP_STATUS_INDICATION_CHARGENEED)
   {
        if((status==APP_STATUS_INDICATION_CHARGING)||(status==APP_STATUS_INDICATION_POWEROFF)||(status==APP_STATUS_INDICATION_CLEARSUCCEED))
        {

		}
		else
		{
            return 0;
		}
   }

	
	
/*
	if(status == APP_STATUS_INDICATION_CHARGING)
	{
	  //¿ªÆôºôÎüµÆ.....
	  app_pwl_stop(APP_PWL_ID_0);
	  app_pwl_stop(APP_PWL_ID_1);
	  breath_led.fade_time_ms = 2000;
	  breath_led.off_time_ms = 1000;
	  breath_led.on_time_ms = 1000;
	  pmu_led_breathing_enable(HAL_IOMUX_PIN_LED1,&breath_led);	  
	  app_status = status;
	  return 0;
	}
	if(app_status == APP_STATUS_INDICATION_CHARGING)
	{
	   if(status != APP_STATUS_INDICATION_CHARGING)
	   {
             pmu_led_breathing_disable(HAL_IOMUX_PIN_LED1);
	   }
	}
*/


	app_status = status;


#if 0
    if(status == APP_STATUS_INDICATION_CONNECTING)
    {
		   app_pwl_stop(APP_PWL_ID_0);
		   app_pwl_stop(APP_PWL_ID_1);
		   breath_led.fade_time_ms = 1000;
		   breath_led.off_time_ms = 500;
		   breath_led.on_time_ms = 500;
		   pmu_led_breathing_enable(HAL_IOMUX_PIN_LED1,&breath_led);	 
		   return 0;
	}
	else
	{
           pmu_led_breathing_disable(HAL_IOMUX_PIN_LED1);
	}
#endif

	



    memset(&cfg0, 0, sizeof(struct APP_PWL_CFG_T));
    memset(&cfg1, 0, sizeof(struct APP_PWL_CFG_T));
    app_pwl_stop(APP_PWL_ID_0);
    app_pwl_stop(APP_PWL_ID_1);


/*
	if(eqmode == 0)
	{
	  cfg0.part[0].level = 1;
	  cfg0.part[0].time = (100);
	  cfg0.part[1].level = 1;
	  cfg0.part[1].time = (1000);
	  cfg0.part[2].level = 1;
	  cfg0.part[2].time = (100);			
	  cfg0.parttotal = 3;
	  cfg0.startlevel = 1;
	  cfg0.periodic = true;
	  app_pwl_setup(APP_PWL_ID_0, &cfg0);
	  app_pwl_start(APP_PWL_ID_0);
      return 0;
	}
*/
	
    switch (status) {

        case APP_STATUS_INDICATION_CLEARSUCCEED:
            cfg0.part[0].level = 1;
            cfg0.part[0].time = (300);
            cfg0.part[1].level = 0;
            cfg0.part[1].time = (200);
            cfg0.part[2].level = 1;
            cfg0.part[2].time = (300);
            cfg0.part[3].level = 0;
            cfg0.part[3].time = (200);
            cfg0.part[4].level = 1;
            cfg0.part[4].time = (300);
            cfg0.part[5].level = 0;
            cfg0.part[5].time = (200);			
            cfg0.parttotal = 6;
            cfg0.startlevel = 1;
            cfg0.periodic = false;
            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);		
			break;
			
		
        case APP_STATUS_TWS_PAIRING:
            cfg0.part[0].level = 1;
            cfg0.part[0].time = (250);
            cfg0.part[1].level = 0;
            cfg0.part[1].time = (250);

            cfg0.part[2].level = 1;
            cfg0.part[2].time = (250);
            cfg0.part[3].level = 0;
            cfg0.part[3].time = (1000);

            cfg0.parttotal = 4;
            cfg0.startlevel = 1;
            cfg0.periodic = true;
            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);				
			break;

		

		
        case APP_STATUS_INDICATION_POWERON:
            cfg0.part[0].level = 0;
            cfg0.part[0].time = (200);
            cfg0.part[1].level = 1;
            cfg0.part[1].time = (1000);
            cfg0.part[2].level = 0;
            cfg0.part[2].time = (100);			
            cfg0.parttotal = 3;
            cfg0.startlevel = 0;
            cfg0.periodic = false;
            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
			led_delay_msg = LED_POWER_ON_DELAY;
			led_delay_Send_Cmd(LED_POWER_ON_DELAY);
            break;

			

        case APP_STATUS_INDICATION_POWEROFF:
            cfg0.part[0].level = 1;
            cfg0.part[0].time = (300);
            cfg0.part[1].level = 0;
            cfg0.part[1].time = (200);
            cfg0.part[2].level = 1;
            cfg0.part[2].time = (300);
            cfg0.part[3].level = 0;
            cfg0.part[3].time = (200);			
            cfg0.parttotal = 4;
            cfg0.startlevel = 1;
            cfg0.periodic = false;
            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            break;

			

			
        case APP_STATUS_INDICATION_PAGESCAN:
            cfg0.part[0].level = 0;
            cfg0.part[0].time = (1000);					
            cfg0.part[1].level = 1;
            cfg0.part[1].time = (250);
            cfg0.part[2].level = 0;
            cfg0.part[2].time = (250);
            cfg0.part[3].level = 1;
            cfg0.part[3].time = (250);


            cfg0.parttotal = 4;
            cfg0.startlevel = 0;
            cfg0.periodic = true;
            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);	
            break;


			
        case APP_STATUS_INDICATION_BOTHSCAN:
            cfg0.part[0].level = 0;
            cfg0.part[0].time = (1000);					
            cfg0.part[1].level = 1;
            cfg0.part[1].time = (250);
            cfg0.part[2].level = 0;
            cfg0.part[2].time = (250);
            cfg0.part[3].level = 1;
            cfg0.part[3].time = (250);


            cfg0.parttotal = 4;
            cfg0.startlevel = 0;
            cfg0.periodic = true;
            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);			
            break;

			
        case APP_STATUS_INDICATION_CHARGING:
            cfg0.part[0].level = 1;
            cfg0.part[0].time = (100);
            cfg0.part[1].level = 1;
            cfg0.part[1].time = (1000);
            cfg0.parttotal = 2;
            cfg0.startlevel = 1;
            cfg0.periodic = true;
            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            break;


        case APP_STATUS_INDICATION_CHARGENEED:
            cfg0.part[0].level = 1;
            cfg0.part[0].time = (300);
            cfg0.part[1].level = 0;
            cfg0.part[1].time = (5000);
            cfg0.parttotal = 2;
            cfg0.startlevel = 1;
            cfg0.periodic = true;
            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            break;



			
		//ANC TEST LED	
		case APP_STATUS_INDICATION_TESTMODE:
        case APP_STATUS_INDICATION_TESTMODE1:
            cfg0.part[0].level = 1;
            cfg0.part[0].time = (300);
            cfg0.part[1].level = 1;
            cfg0.part[1].time = (300);
            cfg0.parttotal = 2;
            cfg0.startlevel = 1;
            cfg0.periodic = true;
            cfg1.part[0].level = 1;
            cfg1.part[0].time = (300);
            cfg1.part[1].level = 1;
            cfg1.part[1].time = (300);
            cfg1.parttotal = 2;
            cfg1.startlevel = 1;
            cfg1.periodic = true;
            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);
            break;
        default:
            break;
    }
    return 0;
}
