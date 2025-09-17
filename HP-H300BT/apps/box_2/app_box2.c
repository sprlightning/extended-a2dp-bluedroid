/******************************************************************************
 *
 * Filename:       app_box.cpp
 *
 * Description:    Source file for communication with the charging box. 
 *
 * date_created:  2019-08-23
 * version:       1.0
 * authors:       fanjianghua
 *
 *****************************************************************************/
#include "cmsis_os.h"
#include "tgt_hardware.h"
#include "pmu.h"
#include "hal_timer.h"
#include "hal_gpadc.h"
#include "hal_trace.h"
#include "hal_gpio.h"
#include "hal_iomux.h"
#include "hal_chipid.h"
#include "app_thread.h"
#include "apps.h"
#include "app_status_ind.h"
#ifdef BT_USB_AUDIO_DUAL_MODE
#include "btusb_audio.h"
#endif
#include <stdlib.h>
	 

#include "communication_svr.h"
//#include "app_ibrt_ui.h"
#include "app_thread.h"
#include "app_battery.h"
#include "app_box2.h"
#include "app_tws_if.h"
//#include "app_ibrt_ui.h"
#ifdef __JS1228_SENSOR__
#include "jsa1228.h"
#endif
#include "app_det.h"
#include "fjh_apps.h"
#include "fjh_apps_thread.h"
#ifdef __EAR_SENSOR__
#include "earsensor.h"
#endif
#include "app_factory.h"

#ifdef __BLE_S80__
#include "ble_cmd.h"
#endif



bool poweron_err_flg = false;
bool btpairing_testflg = false;
bool cmdflg = false;




#define BOX_DEBUG
#ifdef BOX_DEBUG
#define BOX_TRACE TRACE
#else
#define BOX_TRACE(...)
#endif
uint8_t box_old_cmd = 0xff;
uint8_t cmd_data = 0xff;
uint8_t buf_powerof[10]={0};



uint8_t box_status_fjh = box_status_unknow;
bool boxin_flg = true;
bool uart_cmd_enable;
//uint8_t box_leftright_flg = 0;



uint8_t detmode = det_status_unknow;

int app_uart_send2box(uint8_t * buf, uint8_t len);
void c_Send_Cmd(uint32_t idtest);



extern void analog_aud_codec_nomute(void);
static void inoutbox_det_timehandler(void const * param);
osTimerDef(INOUTBOX_GENSORDELAY,inoutbox_det_timehandler);
static osTimerId inoutboxdet_timer = NULL;
static void inoutbox_det_timehandler(void const * param)
{
	TRACE(0,"..................earinboxflg = %d box_status_fjh = %d",earinboxflg,box_status_fjh);


    #ifdef __BOX3_PATCH__
    box_old_cmd = 0xff;
    #endif

	if((get_inoutbox_status() == true)&(box_status_fjh == box_status_open))
	{
	 //   if(btpairing_testflg == false)
	    {
            set_inoutbox_status(false);	
	    }
	}
}



void inoutbox_set_time(uint32_t millisec)
{
  if (inoutboxdet_timer == NULL)
  {
     TRACE(0,"...........inoutbox_set_time = %d",millisec);
	 inoutboxdet_timer	= osTimerCreate(osTimer(INOUTBOX_GENSORDELAY), osTimerOnce, NULL);
	 osTimerStop(inoutboxdet_timer);
  } 
     osTimerStart(inoutboxdet_timer, millisec);


	 TRACE(0,"inoutbox_set_time = %d",millisec);
}


void inoutbox_close_time(void)
{
  if (inoutboxdet_timer != NULL)
  {
     osTimerStop(inoutboxdet_timer);
  } 
  TRACE(0,"inoutbox_close_time...................");
}

/*
uint8_t crc8_maxim(uint8_t *buff, uint8_t length)
{
uint8_t i;
uint8_t crc = 0; //initial value
   while(length--){
     crc ^= *buff++; //crc ^= *buff; buff++;
     for(i = 0; i < 8; i++){
     if(crc & 1)
     crc = (crc >> 1) ^ 0x8C; //0x8C = reverse 0x31
     else
     crc >>= 1;
   }
   }
   return crc;
}
*/


static void delay_timehandler(void const * param);
osTimerDef(DELAY_GENSORDELAY,delay_timehandler);
static osTimerId       delay_timer = NULL;
void c_Send_Cmd(uint32_t idtest);
static void delay_timehandler(void const * param)
{
   /*
   if(box_status_fjh == box_status_close)
   {
      app_poweroff_box();
   }*/
   //ºÏ¸Ç×´Ì¬ÏÂ3ÃëÃ»³äµç... ÔòÅÐ¶ÏÎª¿ìËÙ³ö²ÖÁË
   if(box_status_test2()==box_status_close)
   {
       if(!app_battery_is_charging())
       {
	   TRACE(0,"open box------------");
	   buf_powerof[0] = 0xf5;
	   buf_powerof[1] = 0x01;
	   buf_powerof[2] = 0x00;
	   buf_powerof[3] = POWERON_NOBOX_HEADSET;
	   buf_powerof[4] = 0x00;
	   buf_powerof[5] = 0x01;  
	   buf_powerof[6] = 0xf5;
	   communication_receive__callback(buf_powerof,7);

       }
   }
}


void delay_set_time(uint32_t millisec)
{
 osStatus tt;
  if (delay_timer == NULL)
  {
	 delay_timer	= osTimerCreate(osTimer(DELAY_GENSORDELAY), osTimerOnce, NULL);
	 osTimerStop(delay_timer);
  } 
      tt = osTimerStart(delay_timer, millisec);
	  TRACE(9,"osTimerStart:= %08x \n",tt);
}

void delay_close_time(void)
{
	if (delay_timer != NULL)
	{
	   osTimerStop(delay_timer);
	} 
}


/**********************************************************************************************************************/
void c_Send_Cmd(uint32_t idtest)
{
  APP_MESSAGE_BLOCK msg;
  msg.mod_id			= APP_MODUAL_DELAY;
  msg.msg_body.message_id = idtest;
  msg.msg_body.message_ptr = (uint32_t) NULL;
  app_mailbox_put(&msg);
}


int app_delay_process(APP_MESSAGE_BODY * msg)
{
   uint8_t cmd_data = msg->message_id;
   //if(msg->message_id == box_cmd)
   {
      app_uart_send2box(&cmd_data,1);
   }
   return 0;
}


void init_delay_thread(void)
{
	app_set_threadhandle(APP_MODUAL_DELAY, app_delay_process);
}
/**********************************************************************************************************************/



int app_box_uart_open(void)
{
	BOX_TRACE(8,"---------------------------app_box_uart_open-----------------------------------");

	init_delay_thread();
    return 0;
}


extern void closeboxhandletime(void);
extern void analog_aud_codec_mute(void);
extern void analog_aud_codec_nomute(void);
uint8_t uart_sendbuf[7]={0x55,0xAA,0,0,0,0xAA};
int app_uart_send2box(uint8_t * buf, uint8_t len)
{
/*
	if(((* buf)==POWERON_NOBOX_HEADSET)||((* buf)==CLOSEBOX_NOBOX_HEADSET))
	{
	
	}
	else
	{
    if((* buf) == BOX_IN_CMD)
    {
		uart_sendbuf[2] = BOX_IN_CMD;
		uart_sendbuf[3] = box_leftright_flg;
		uart_sendbuf[4] = 0x01;
		uart_sendbuf[5] = 100;
		uart_sendbuf[6] = crc8_maxim(uart_sendbuf,6);
		communication_send_buf(uart_sendbuf,7);
		osDelay(10);
		communication_send_buf(uart_sendbuf,7);
	}
	else if((* buf) == STATUS_HEADSET)
	{
		    uart_sendbuf[2] = STATUS_HEADSET;
		    uart_sendbuf[3] = box_leftright_flg;
		    uart_sendbuf[4] = 0x01;
		    uart_sendbuf[5] = 100;
		    uart_sendbuf[6] = crc8_maxim(uart_sendbuf,6);
		    communication_send_buf(uart_sendbuf,7);
	}
	else if((* buf) == BTCONNECTED)
	{
	    if(apps_data.btconnectdtoboxcmdflg == true)
	    {
           uart_sendbuf[5] = 1; 
		}
		else
		{
           uart_sendbuf[5] = 0;
		}
		uart_sendbuf[2] = BTCONNECTED;
		uart_sendbuf[3] = box_leftright_flg;
		uart_sendbuf[4] = 0x01;
		//uart_sendbuf[5] = 1;
		uart_sendbuf[6] = crc8_maxim(uart_sendbuf,6);
		communication_send_buf(uart_sendbuf,7);
		BOX_TRACE(8,"---------------------------BTCONNECTED-----------------------------------");
	}
	else
	{
        uart_sendbuf[2] = (* buf);
        uart_sendbuf[3] = box_leftright_flg;
		uart_sendbuf[4] = 0x00;
	    uart_sendbuf[5] = crc8_maxim(uart_sendbuf,5);
        communication_send_buf(uart_sendbuf,6);
	}
	}
*/
switch(* buf)
{

    case INOUTBOXDELAY:
		inoutbox_set_time(1000);
		//set_inoutbox_status(true);
		break;

    case STATUS_HEADSET:	
	break;

	case POWEROFF_HEADSET:
		//communication_send_buf(uart_sendbuf,6);
		app_poweroff_box();
	break;

    case POWERON_NOBOX_HEADSET:
	case POWERON_HEADSET:
		closeboxhandletime();
		box_det_init();
		#ifdef __POWERONAUTOOPENANC__
	    apps_data.poweronautoancflg = true;
	    #endif
		//analog_aud_codec_nomute();
		//app_box_poweron_fjh();
		fjh_thread_msg_send(THREAD_MSG_APPS_EVENT_BOXPOWERON);
	break;

    case CLOSEBOX_NOBOX_HEADSET:
	case CLOSEBOX_HEADSET:
		btpairing_testflg = false;
#ifdef __FJH_SLAVER_BUG__
	    slaverbug_delay_close_time();
#endif		
		close_box_det();
		app_stop_10_second_timer(APP_PAIR_TIMER_ID);
		app_stop_10_second_timer(APP_POWEROFF_TIMER_ID);	
		//#ifdef __TPOETS_TEST_PATCH__
		analog_aud_codec_mute();
		//#endif
		//app_box_close_fjh();
		fjh_thread_msg_send(THREAD_MSG_APPS_EVENT_BOXCLOSE);
	break;

	case TWS_PAIR:
    //app_enter_bttest_fjh();
    app_enter_twspairing_fjh();
	break;

	case ENTER_BT_PAIR_CLR:
		//app_ibrt_ui_event_entry(IBRT_CLOSE_BOX_EVENT);
		//app_enter_twspairing_fjh();
		//close_box_test();
		//inoutbox_set_time(2000);
		//app_enter_bttest_fjh();
		app_enter_twspairing_fjh();
		//app_clear_btpailist_fjh();
		break;

    case BT_PAIRNG2:
	case BT_PAIRNG:
		BT_PAIRING();
		break;

	case ENTER_TEST:
		app_enter_bttest_fjh();
		break;

	case ENTER_DUT:
		app_enter_dut_fjh();
		break;

	case ENTER_OTA:
		//app_enter_ota_fjh();
		app_enter_ota_box();
		break;
		
	case POWEROFF_HEADSET_TEST:
		app_poweroff_box();
		break;

	case BOX_IN_CMD:
        set_inoutbox_status(true);					
	break;




	default:break;	
}	
    return 0;
}







//extern bool poweroncharingflg;
void communication_receive__callback(uint8_t *buf, uint8_t len)
{
	//uint8_t uartrxcmd = 0;
    //ÃüÁî½âÎö...
	//BOX_TRACE(8,"......communication_receive__callback");
    //DUMP8("%02x ", buf, len);

	uint8_t ii=0;
	 if(len>=7)
	 {
         while(1)
       	 {
             if((buf[ii] == 0xf5)&(buf[ii+1] == 0x01))
             {
				 BOX_TRACE(8,"cmd sucefull--------- = %x",ii);
				 break;
		     }
			 ii++;
			 if(ii>=(len-1))
			 {
			     BOX_TRACE(8,"cmd error--------- = %x",ii);
                 return;
			 }
	     }
         cmd_data = (buf[ii+3]);


		 //if((cmd_data == CLOSEBOX_HEADSET)||(cmd_data == POWERON_HEADSET))
		 //{

		 //}
		 //else if(cmd_data !=CLOSEBOX_HEADSET)
		 //{
		   //if((buf[ii+3]) != box_leftright_flg)
		   //{
           //    BOX_TRACE(8,"box_leftright_flg--------- = %x",box_leftright_flg);
		 	//   return;
		   //}
		 //}
	  }
	  else
	  { 
	     BOX_TRACE(8,"cmd len error--------- = %x",ii);
		 return;
	  }

#if 0
	if(uart_cmd_enable == false)
	{
	   BOX_TRACE(8,"---->uart_cmd_enable error = %d",uart_cmd_enable);  
	   if((*buf) ==ENTER_TEST)
	   {
	         enter_test_mode = true;
             return;
	   }
	}
	else
	{
       BOX_TRACE(8,"---->uart_cmd_enable sucefull = %d",uart_cmd_enable);   
	}
#endif	

      #ifdef __BLE_S80__
      if((cmd_data ==STATUS_HEADSET)||(cmd_data ==POWERON_HEADSET))
      {
	       box_battery = (buf[ii+4] &0x7f); 
      }
	  #endif



	  if(cmd_data !=STATUS_HEADSET)
	  {
	    if(uart_cmd_enable == false)
	    {
          buf_powerof[0] = (buf[ii]);
		  buf_powerof[1] = (buf[ii+1]);
		  buf_powerof[2] = (buf[ii+2]);
		  buf_powerof[3] = (buf[ii+3]);
		  buf_powerof[4] = (buf[ii+4]);	  
		  buf_powerof[5] = (buf[ii+5]);	 
		  buf_powerof[6] = (buf[ii+6]);	
		  cmdflg = true;
	    }
	  }

	  if(uart_cmd_enable == false)
	  {
	       if(cmd_data != STATUS_HEADSET)
	       {
               return;
		   }
           
	  }

	  inoutbox_set_time(2000);

      if(box_old_cmd!=cmd_data)
      {
           box_old_cmd = cmd_data;
           #ifdef __BOX3_PATCH__
           if((cmd_data == ENTER_ONE_BT_PAIR_TEST)||(cmd_data == TWS_PAIR)||(cmd_data == ENTER_DUT)||(cmd_data == ENTER_OTA)||(cmd_data == ENTER_BT_PAIR_CLR)||(cmd_data == ENTER_TEST))
           {
                  return;
		   }
		   #endif
	  }
	  else
	  {
	       if((cmd_data ==STATUS_HEADSET)||(cmd_data ==BTCONNECTED))
	       {
              
	       }
		   else
		   {

#ifdef __BOX3_PATCH__
			  if((cmd_data == ENTER_ONE_BT_PAIR_TEST)||(cmd_data == TWS_PAIR)||(cmd_data == ENTER_DUT)||(cmd_data == ENTER_OTA)||(cmd_data == ENTER_BT_PAIR_CLR)||(cmd_data == ENTER_TEST))
			  {
			         box_old_cmd = 0xff;
					 //return;
			  }
			  else
#endif
              {
                     return;
              }
		   }
	  }


	  

	  switch(cmd_data)
	  {

                 case POWERON_NOBOX_HEADSET:
					  //set_inoutbox_status(true);
					  
					  delay_close_time();
					  if(apps_data.fjhpatch003flg == true)
					  {
                         apps_data.fjhpatch003okflg = true;
					  }
					  
				      //delay_close_time();
					  if(box_status_fjh != box_status_open)
					  {				  	
					     BOX_TRACE(8,"---------------------------POWERON_NOBOX_HEADSET-----------------------------------");
					     c_Send_Cmd(cmd_data);
					     box_status_fjh = box_status_open;
					  }				 	
				 	break;


				 case CLOSEBOX_NOBOX_HEADSET:
					  close_box_det();
					  inoutbox_close_time();
					  set_inoutbox_status(true);
					  delay_set_time(3000);
					  if(apps_data.fjhpatch003flg == true)
					  {
                           //apps_data.fjhpatch003okflg = true;
					  }
					  if(box_status_fjh != box_status_close)
					  {
					     BOX_TRACE(8,"---------------------------CLOSEBOX_NOBOX_HEADSET-----------------------------------");
					     c_Send_Cmd(cmd_data);
					     box_status_fjh = box_status_close;
					  }		

					if(app_factorymode_get())
					{
						//ÍË³öDUT Ä£Ê½.........
						//hal_sw_bootmode_clear(HAL_SW_BOOTMODE_REBOOT);
						app_reset(); 
					} 
				 	break;


	  

                  case BTCONNECTED:
				  //	inoutbox_set_time(2000);
				  	BOX_TRACE(8,"---------------------------BTCONNECTED-----------------------------------");
				  	c_Send_Cmd(cmd_data);
				  	break;


                  case ENTER_ONE_BT_PAIR_TEST:
				  	BOX_TRACE(8,"---------------------------ENTER_ONE_BT_PAIR_TEST-----------------------------------");
					c_Send_Cmd(cmd_data);
				  	break;
					


                  case CLOSEBOX_HEADSET:
				  	  BOX_TRACE(8,"---------------------------CLOSEBOX_HEADSET-----------------------------------");
					  close_box_det();
					  inoutbox_close_time();
					  set_inoutbox_status(true);
					  delay_set_time(3000);
					  if(apps_data.fjhpatch003flg == true)
					  {
                           //apps_data.fjhpatch003okflg = true;
					  }

					  if(app_factorymode_get())
					  {
						  //ÍË³öDUT Ä£Ê½.........
						  //hal_sw_bootmode_clear(HAL_SW_BOOTMODE_REBOOT);
						  app_reset(); 

					  }

					  
					  if(box_status_fjh != box_status_close)
					  {
					     c_Send_Cmd(cmd_data);
					     box_status_fjh = box_status_close;
					  }
				  	 break;
					 


				  case POWEROFF_HEADSET:					  
					  BOX_TRACE(8,"---------------------------POWEROFF_HEADSET-----------------------------------");
					  delay_close_time();
					  if(box_status_fjh != box_status_poweroff)
					  {					  
					     c_Send_Cmd(cmd_data);
					     box_status_fjh = box_status_poweroff;
					  }
					  break;

					  
		  
				  case POWERON_HEADSET:
				  	  //inoutbox_set_time(2000);
					  //set_inoutbox_status(true);
					  delay_close_time();
					  if(apps_data.fjhpatch003flg == true)
					  {
                         apps_data.fjhpatch003okflg = true;
					  }
					  
					  
				      //delay_close_time();
					  if(box_status_fjh != box_status_open)
					  {				  	
					  BOX_TRACE(8,"---------------------------POWERON_HEADSET-----------------------------------");
					  c_Send_Cmd(cmd_data);
					  box_status_fjh = box_status_open;
					  }
					  break;	  


					  
				  case TWS_PAIR:
					  BOX_TRACE(8,"---------------------------TWS_PAIR-----------------------------------");
					  //if(box_status_fjh != box_status_close)
					  {					  
					       c_Send_Cmd(cmd_data);
						   
					  }
					  break;
	  
					  
				  case ENTER_DUT:
					  BOX_TRACE(8,"---------------------------ENTER_DUT-----------------------------------");
					  //if(box_status_fjh != box_status_close)
					  {					  
					       c_Send_Cmd(cmd_data);
					  }
					  break;  
					  

		  
				 case ENTER_OTA:
					  BOX_TRACE(8,"---------------------------ENTER_OTA-----------------------------------");
					  //if(box_status_fjh != box_status_close)
					  {					  
					       c_Send_Cmd(cmd_data);
					  }
					  break;
					  
					  
				  case ENTER_BT_PAIR_CLR:
					  BOX_TRACE(8,"---------------------------ENTER_BT_PAIR_CLR-----------------------------------");
					  //if(box_status_fjh != box_status_close)
					  {		
					       //closebox_app();
					       c_Send_Cmd(cmd_data);
					  }
				  break;

                  case BT_PAIRNG2:
				  case BT_PAIRNG:
				  	BOX_TRACE(8,"---------------------------BT_PAIRNG-----------------------------------");
					btpairing_testflg = true;
					apps_data.btconnectdtoboxcmdflg = false;
					//inoutbox_set_time(3000);
					c_Send_Cmd(cmd_data);
				  	break;


				  case ENTER_TEST:
				  	  BOX_TRACE(8,"---------------------------ENTER_BT_TEST-----------------------------------");
					  if(box_status_fjh != box_status_close)
					  {					  
					       c_Send_Cmd(cmd_data);
					  }				  
				  break;


		          case POWEROFF_HEADSET_TEST:
			      BOX_TRACE(8,"---------------------------POWEROFF_HEADSET_TEST-----------------------------------");
				  if(box_status_fjh != box_status_close)
				  {				 
				       c_Send_Cmd(cmd_data);
				  }
		          break;

                 case STATUS_HEADSET2:
		         case STATUS_HEADSET:
				 BOX_TRACE(8,"---------------------------STATUS_HEADSET-----------------------------------");
				// c_Send_Cmd(cmd_data);

				 if(uart_cmd_enable == true)
				 {
				    set_inoutbox_status(true);
				 }
		  	     break;		
	
		default:break;
	  }
}


extern bool EnterBTpairFlg;
extern bool EnterTWSpairFlg;
void set_inoutbox_status(bool onoff)
{

//#ifndef __TPOETS_TEST_PATCH__
//return;
//#endif


/*************************************************************************/
   if(box_status_fjh == box_status_poweroff)
   {
       return;
   }
/*************************************************************************/   	
   if(onoff)
   {
      //inoutbox_set_time(1000);
      c_Send_Cmd(INOUTBOXDELAY);
      if(earinboxflg == false)
      {
        BOX_TRACE(8,"---------------------------IN BOX-----------------------------------");

		if(btpairing_testflg == true)
		{
          // BOX_TRACE(8,"---------------------------IN BOX---------btpairing_testflg = %d",btpairing_testflg); 
		  // return;
		}

		
        earinboxflg = true;
	    analog_aud_codec_mute();
	    #ifdef __EAR_SENSOR__
        Earbuds_In_box();
	    #endif	
        #ifdef __JS1228_SENSOR__
		Earbuds_In_box();
		#else
		//hal_pwrkey_close_fjh();
	    //send_key_event(HAL_KEY_CODE_PWR,HAL_KEY_EVENT_BOXIN);	
	    fjh_thread_msg_send(THREAD_MSG_APPS_EVENT_INBOX);
	    #endif
	    if((app_factorymode_get())||(EnterBTpairFlg == true)||(EnterTWSpairFlg == true))
	    {
	        
	    }
		else
		{
		    #ifdef __FJH_APPS__
            //fjh_thread_msg_send(THREAD_MSG_APPS_EVENT_INBOX);
            #endif			
           //app_box_close_fjh();
		}
      }
   }
   else
   {
      if(hal_gpio_pin_get_val(HAL_GPIO_PIN_P0_1)==0)
      {
         inoutbox_set_time(2000);
		 BOX_TRACE(8,"---------------------------OUT BOX  error-----------------------------------");
         return;   
	  }
   
      if(earinboxflg == true)
      {   
         BOX_TRACE(8,"---------------------------OUT BOX-----------------------------------");
		 
         #ifdef __POWERONAUTOOPENANC__
		 apps_data.poweronautoancflg = true;
         #endif

		 
	     earinboxflg = false;
	     analog_aud_codec_nomute();
	     out_box_det();
         #ifdef __EAR_SENSOR__
	     Earbuds_Move_out();
         #endif		
         #ifdef __JS1228_SENSOR__
		 Earbuds_Move_out();
         #else
		 //hal_pwrkey_openfjh();
	     //send_key_event(HAL_KEY_CODE_PWR,HAL_KEY_EVENT_BOXOUT);	 
	     fjh_thread_msg_send(THREAD_MSG_APPS_EVENT_OUTBOX);
		 #endif	 	 
		 btpairing_testflg = false;
		 
	     if((app_factorymode_get())||(EnterBTpairFlg == true)||(EnterTWSpairFlg == true))
	     {
	         EnterBTpairFlg = false;
			 EnterTWSpairFlg = false;
	     }
		 else
		 {
		    #ifdef __FJH_APPS__
            //fjh_thread_msg_send(THREAD_MSG_APPS_EVENT_OUTBOX);
            #endif	
            //app_box_poweron_fjh();
		 }	
#ifdef __INBOX_STATUS_INT_PATCH__
		 inbox_gpiote_init();
#endif
         //³ö²Öbox_old_cmd ÇåÁã........
		 box_old_cmd = 0;
      }
   }
}

bool get_inoutbox_status(void)
{
   BOX_TRACE(8,"earinboxflg := %d",earinboxflg);


   //#ifndef __TPOETS_TEST_PATCH__
   //earinboxflg = false;
   //#endif
   return earinboxflg;
}

uint8_t box_status_test2(void)
{
   BOX_TRACE(8,"box_status_fjh := %d",box_status_fjh);
   return box_status_fjh;
}

#ifdef __INBOX_STATUS_INT_PATCH__

//åŠ å¿«å…¥ä»“æ£€æµ‹.....................................
void inbox_int_handler(enum HAL_GPIO_PIN_T pin)
{
	BOX_TRACE(8,"......inbox_int_handler");
	inbox_gpiote_close();
	if(hal_gpio_pin_get_val(HAL_GPIO_PIN_P0_1)==0)
	{
       set_inoutbox_status(true);
     // c_Send_Cmd(INOUTBOXDELAY);
	}
	else
	{
	    BOX_TRACE(8,"......release_int_handler");
        inbox_gpiote_init();
	}
}

void inbox_gpiote_init(void)
{
    struct HAL_GPIO_IRQ_CFG_T gpiocfg;
    gpiocfg.irq_enable = true;
    gpiocfg.irq_debounce = true;
    gpiocfg.irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE;
    gpiocfg.irq_polarity = HAL_GPIO_IRQ_POLARITY_LOW_FALLING;
    gpiocfg.irq_handler = inbox_int_handler;
    hal_gpio_setup_irq(app_boxin_detecter_cfg.pin, &gpiocfg);
}

void inbox_gpiote_close(void)
{
    struct HAL_GPIO_IRQ_CFG_T gpiocfg;
    gpiocfg.irq_enable = false;
    gpiocfg.irq_debounce = true;
    gpiocfg.irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE;
    gpiocfg.irq_polarity = HAL_GPIO_IRQ_POLARITY_LOW_FALLING;
    gpiocfg.irq_handler = inbox_int_handler;
    hal_gpio_setup_irq(app_boxin_detecter_cfg.pin, &gpiocfg);
}

#endif


