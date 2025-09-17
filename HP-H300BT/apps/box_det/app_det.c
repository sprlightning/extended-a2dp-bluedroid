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
#include "app_thread.h"
#ifdef __JS1228_SENSOR__
#include "jsa1228.h"
#endif

#ifdef __EAR_SENSOR__
#include "earsensor.h"
#endif
#include "app_box2.h"

void boxdet_event_enableint(enum HAL_GPIO_PIN_T pin, enum HAL_GPIO_IRQ_POLARITY_T polarity);
void boxdet_event_disint(enum HAL_GPIO_PIN_T pin, enum HAL_GPIO_IRQ_POLARITY_T polarity);
extern void analog_aud_codec_mute(void);
extern void analog_aud_codec_nomute(void);
//void boxdet_Send_Cmd(uint32_t idtest);
//void lineinbug_delay_set_time(uint32_t millisec);
void boxdet_Send_Cmd(uint32_t idtest);

bool earinboxflg = false;


static void boxdet_event_irqhandler(enum HAL_GPIO_PIN_T pin)
{
    TRACE(9,"boxdet_event_irqhandler>>>>>>>>>>>>>>>> \n");
	boxdet_event_disint(BOXIN_DET_IO, HAL_GPIO_IRQ_POLARITY_LOW_FALLING);
    if(hal_gpio_pin_get_val(BOXIN_DET_IO))
    {
            if(earinboxflg == false)
            {
               boxdet_event_enableint(BOXIN_DET_IO, HAL_GPIO_IRQ_POLARITY_LOW_FALLING);
            }
	}
	else
	{      
            if(earinboxflg == false)
            {
                //boxdet_event_enableint(BOXIN_DET_IO, HAL_GPIO_IRQ_POLARITY_HIGH_RISING);
                //earinboxflg = true;
                c_Send_Cmd(BOX_IN_CMD);
				TRACE(9,"box in 01........................... \n");
            }				
	}
}


void boxdet_event_enableint(enum HAL_GPIO_PIN_T pin, enum HAL_GPIO_IRQ_POLARITY_T polarity)
{
	struct HAL_GPIO_IRQ_CFG_T gpiocfg;
    gpiocfg.irq_enable = true;
    gpiocfg.irq_debounce = true;
    gpiocfg.irq_polarity = polarity;
    gpiocfg.irq_handler = boxdet_event_irqhandler;
    gpiocfg.irq_type = HAL_GPIO_IRQ_TYPE_LEVEL_SENSITIVE;
    hal_gpio_setup_irq(pin, &gpiocfg);
}


void boxdet_event_disint(enum HAL_GPIO_PIN_T pin, enum HAL_GPIO_IRQ_POLARITY_T polarity)
{
	struct HAL_GPIO_IRQ_CFG_T gpiocfg;
	gpiocfg.irq_enable = false;
	gpiocfg.irq_debounce = false;
	gpiocfg.irq_polarity = polarity;
	gpiocfg.irq_handler = NULL;
	gpiocfg.irq_type = HAL_GPIO_IRQ_TYPE_LEVEL_SENSITIVE;
    hal_gpio_setup_irq(pin, &gpiocfg);
}





#if 1
void boxdet_Send_Cmd(uint32_t idtest)
{
  APP_MESSAGE_BLOCK msg;
  msg.mod_id			= APP_MODUAL_BOX_DET;
  msg.msg_body.message_id = idtest;
  msg.msg_body.message_ptr = (uint32_t) NULL;
  app_mailbox_put(&msg);
}

int box_det_process(APP_MESSAGE_BODY * msg)
{
   TRACE(9,"box_det_process........................... \n");
   //lineinbug_delay_set_time(1000);
   return 0;
}


void boxdet_init_delay_thread(void)
{
	app_set_threadhandle(APP_MODUAL_BOX_DET, box_det_process);
}
#endif



void close_box_det(void)
{
//   earinboxflg = true;
   boxdet_event_disint(BOXIN_DET_IO, HAL_GPIO_IRQ_POLARITY_LOW_FALLING);
}


void out_box_det(void)
{
   boxdet_event_enableint(BOXIN_DET_IO, HAL_GPIO_IRQ_POLARITY_LOW_FALLING);
}


void in_box_det(void)
{
    boxdet_event_disint(BOXIN_DET_IO, HAL_GPIO_IRQ_POLARITY_HIGH_RISING);
}



void box_det_init(void)
{
	//osDelay(20);
	/*
    if(hal_gpio_pin_get_val(BOXIN_DET_IO))
    {
            boxdet_event_enableint(BOXIN_DET_IO, HAL_GPIO_IRQ_POLARITY_LOW_FALLING);
	}
	else
	{
            boxdet_event_enableint(BOXIN_DET_IO, HAL_GPIO_IRQ_POLARITY_HIGH_RISING);
	}
	*/

	boxdet_event_enableint(BOXIN_DET_IO, HAL_GPIO_IRQ_POLARITY_LOW_FALLING);
}


