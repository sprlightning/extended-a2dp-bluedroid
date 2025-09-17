
#ifndef __APP_BOX_H__
#define __APP_BOX_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_key.h"

#define POWERON_HEADSET         0x00
#define CLOSEBOX_HEADSET        0x02
#define POWEROFF_HEADSET        0x06
#define ENTER_BT_PAIR_CLR       0x04
#define READBTADD               0x20
#define WRITEBTADD              0x21
#define STATUS_HEADSET          0x07
#define STATUS_HEADSET2          0x05

#define BT_PAIRNG               0x0e
#define BT_PAIRNG2              0x01


#define BOX_IN_CMD              0x22
#define BTCONNECTED             0x08
#define TWS_PAIR                0xE1
#define ENTER_DUT               0xE2
#define ENTER_OTA               0XD1
#define ENTER_TEST              0XC7
#define POWEROFF_HEADSET_TEST   0XC1
#define ENTER_ONE_BT_PAIR_TEST  0xA2
#define POWERON_NOBOX_HEADSET   0xAB
#define CLOSEBOX_NOBOX_HEADSET  0xAC

#define JS1228_CALIBRATION_HEADSET  0x37

#define STANDY_INSTRUCTION01_HEADSET  0x26
#define STANDY_INSTRUCTION02_HEADSET  0x18



#define INOUTBOXDELAY               0X11




enum
{
    box_status_unknow = 0,
    box_poweron,
    box_ota_fjh,
    box_btclear,
    box_status_open,
    box_status_close,
    box_status_poweroff,
    box_delay_test,
    box_cmd,
};



enum
{
    det_status_unknow = 0,
    det_boxoff,
    det_twspairing,
};


int app_box_uart_open(void);

//int app_uart_send2box(uint8_t *buf,uint8_t len);
int app_uart_send2box(uint8_t * buf, uint8_t len);
void init_delay_thread(void);
uint8_t box_status_test2(void);
void closebox_app(void);
extern uint8_t buf_powerof[10];
//extern uint8_t box_leftright_flg;
extern bool uart_cmd_enable;
void communication_receive__callback(uint8_t *buf, uint8_t len);
void c_Send_Cmd(uint32_t idtest);
void set_inoutbox_status(bool onoff);
bool get_inoutbox_status(void);
void delay_close_time(void);
void inoutbox_set_time(uint32_t millisec);

#ifdef __INBOX_STATUS_INT_PATCH__
void inbox_gpiote_init(void);
void inbox_gpiote_close(void);
#endif





extern bool btpairing_testflg;
extern bool cmdflg;





#ifdef __cplusplus
}
#endif

#endif
