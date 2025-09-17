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
#ifndef __FJH_APPS_THREAD_H__
#define __FJH_APPS_THREAD_H__
#undef _EXT_

#ifdef __HDS_THREAD_C_
#define _EXT_ 
#else 
#define _EXT_ extern 
#endif 

#include "app_status_ind.h"


#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"

#if 0
enum
{
    box_status_unknow = 0,
    box_status_in,
    box_status_out,
    box_status_off,
    box_status_open,
    box_status_batter_power_off,
};
extern uint8_t box_status;
#endif

#define TWSSYNCCMD_POWEROFF 0x01
#define TWSSYNCCMD_CONNECTEDLED 0x02
#define TWSSYNCCMD_INCOMCALLLED 0x03
#define TWSSYNCCMD_TWSSLAVERCONNECTTONE 0x04
#define TWSSYNCCMD_TWSMASTERCONNECTTONE 0x05
#define TWSSYNCCMD_BTDISCONNECTEDTEST 0x06
#define TWSSYNCCMD_GAMEMUSICTEST 0x07
#define TWSSYNCCMD_DISCONNECTERROR 0x08
#define TWSSYNCCMD_EARINSTATUS 0x09
#define TWSSYNCCMD_TWSCONNECTTONE 0x0A



//#define TWSSYNCCMD_DISCONNECTERROR 0x0B
#define TWSSYNCCMD_DISCONNECTERROR03 0x0C

#define TWSSYNCCMD_TWSVOLUMEFLG 0x0D

#define TWSSYNCCMD_TWSHFP_WEARDOWN_FLG 0x0E

#define TWSSYNCCMD_ANCON_MASTER_FLG 0x0F

#define TWSSYNCCMD_AUTO_ANC_FLG 0x10


#define TWSSYNCCMD_TONEDELAY_FLG 0x11


#define TWSSYNCCMD_TWSBATTERYSTATUS 0x12


#define TWSSYNCCMD_BATTERYLOV 0x13

#define TWSSYNCCMD_APPKEY 0x14

#define TWSSYNCCMD_APPNMAE 0x15

#define TWSSYNCCMD_APPEQ 0x16

#define TWSSYNCCMD_APPRING 0x17

#define TWSSYNCCMD_APPFACTORYRESET 0x18



#define TWSSYNCCMD_APPPRE 0x19

#define TWSSYNCCMD_APPNEX 0x1A

#define TWSSYNCCMD_APPPLAY 0x1B

#define TWSSYNCCMD_APPSTOP 0x1C


#define TWSSYNCCMD_INOUTSTATUS 0x1D


#define TWSSYNCCMD_APPFACTORYRESET_PHONELOST 0x1E








#define TWSSYNCCMD_ENTERPAIRING 0x30


#define TWSSYNCCMD_SCOVOLUME 0x31









//define type of msg
//输入事件
#define THREAD_MSG_APPS_EVENT_INBOX     	0x00 //入仓
#define THREAD_MSG_APPS_EVENT_OUTBOX     	0x01 //出仓
#define THREAD_MSG_APPS_EVENT_WEARUP    	0x02 //入耳
#define THREAD_MSG_APPS_EVENT_WEARDOWN    	0x03 //出耳
#define THREAD_MSG_APPS_EVENT_POWERONPAIRING    0x04 
#define THREAD_MSG_APPS_EVENT_ANCSWITCH         0x05
#define THREAD_MSG_APPS_EVENT_POWEROFF          0x06
#define THREAD_MSG_APPS_EVENT_BOXPOWERON        0x07
#define THREAD_MSG_APPS_EVENT_BOXCLOSE          0x0a
#define THREAD_MSG_APPS_EVENT_BTDISCONNECT      0x0b
#define THREAD_MSG_APPS_EVENT_BTCONNECT         0x0c

#define THREAD_MSG_APPS_EVENT_BTSCAN            0x0d
#define THREAD_MSG_APPS_BATTERYLOWPOWEROFF      0x0e


#define THREAD_MSG_APPS_EVENT_BTDISCONNECTOK      0x0f



#define THREAD_MSG_APPS_EVENT_CONNECTSYNCLED        0x08
#define THREAD_MSG_APPS_EVENT_INCOMCALLSYNCLED        0x09





//蓝牙事件
#define THREAD_MSG_BT_APPS_EVENT_DUT				0x10 
#define THREAD_MSG_BT_APPS_EVENT_CLEANPAIRLIST		0x11
#define THREAD_MSG_BT_APPS_EVENT_BTTEST		        0x12
#define THREAD_MSG_BT_APPS_EVENT_OTA				0x13

#define THREAD_MSG_BT_APPS_EVENT_MUSICGAME_MODE_SWITCH				0x14




#define THREAD_MSG_BT_APPS_EVENT_TIMEOUT				0x20 //未连接蓝牙的超时处理
#define THREAD_MSG_BT_APPS_EVENT_FREEMAN				0x22 //耳机独立使用
#define THREAD_MSG_BT_APPS_EVENT_MOBILE_CONNECTED	    0x23 //连接上手机
#define THREAD_MSG_BT_APPS_EVENT_MOBILE_DISCONNECTED	0x24 //断开手机
#define THREAD_MSG_BT_APPS_EVENT_TWS_PAIRING 		    0x25 //主从配对
#define THREAD_MSG_BT_APPS_EVENT_TWS_DISCONNECTED 	    0x26 //断开主从
#define THREAD_MSG_BT_APPS_EVENT_TWS_CONNECTED 	        0x2B //

#define THREAD_MSG_BT_APPS_EVENT_POWERON_WITHROLE 	    0x27 //有主从角色开机
#define THREAD_MSG_BT_APPS_EVENT_POWERON_WITHOUTROLE    0x28 //无主从角色开机
#define THREAD_MSG_BT_APPS_EVENT_POWERON_TEST 		    0x29 //开机进测试
#define THREAD_MSG_BT_APPS_EVENT_POWEROFF			    0x2A


//电池事件
#define THREAD_MSG_BATTRY_APPS_EVENT_LOWPOWER       0x30
#define THREAD_MSG_BATTRY_APPS_EVENT_LOWPOWEROFF    0x31
#define THREAD_MSG_BATTRY_APPS_EVENT_FULLCHARGE     0x32
#define THREAD_MSG_BATTRY_APPS_EVENT_NTC            0x33
#define THREAD_MSG_BATTRY_APPS_EVENT_1205RESET      0x34



#define THREAD_MSG_APPS_EVENT_SLAVERBUG            0x41

#define THREAD_MSG_APPS_EVENT_DISCONNECTERROR      0x42

#define THREAD_MSG_APPS_ANCLOCALOFF      0x43

#define THREAD_MSG_APPS_UISWITCHERROR      0x44

#define THREAD_MSG_APPS_EVENT_CLOSE_BOX_PATCH2 0x45


#ifdef __CLOSEBOX_PATCH__
#define THREAD_MSG_APPS_EVENT_CLOSE_BOX_PATCH_TEST 0x50
#endif 

#define THREAD_MSG_APPS_EVENT_EQMODE_TEST 0x51

#define THREAD_MSG_APPS_EVENT_NEX_TEST 0x60
#define THREAD_MSG_APPS_EVENT_PRE_TEST 0x61
#define THREAD_MSG_APPS_EVENT_PLAY_TEST 0x62
#define THREAD_MSG_APPS_EVENT_STOP_TEST 0x63


#ifdef __INOUTBOX_PATCH__	
#define THREAD_MSG_APPS_EVENT_INOUT_BOX_PATCH_TEST 0x64
#endif



#define THREAD_MSG_BT_APPS_EVENT_CLEANPAIRLIST_PHONELIST 0x65


#define THREAD_MSG_BT_APPS_EVENT_TWS_PAIRING2 		    0x66 //主从配对


#define THREAD_MSG_BT_APPS_VOLUEUP_TWS_TEST 		    0x67
#define THREAD_MSG_BT_APPS_VOLUEDOWN_TWS_TEST 		    0x68

#define THREAD_MSG_APPS_POWER_ON_10S_PAIRING            0x69
#define THREAD_MSG_APPS_POWER_ON_10S_STOP               0x6A







_EXT_ void fjh_thread_init(void);
_EXT_ void fjh_thread_msg_send(uint32_t msgid);
_EXT_ void fjhhds_delay_set_time(uint32_t millisec);
extern uint8_t twscmdsync[34];
_EXT_ void fjhhds_delay_close_time(void);

#ifdef __INOUTBOX_PATCH__
_EXT_ void inoutbox_delay_set_time(uint32_t millisec);
_EXT_ void inoutbox_delay_close_time(void);
#endif

#ifdef __SCO_TWS_SWITCH_PATCH__
	extern bool sco_mode_flg;
#endif



#ifdef __cplusplus
}
#endif
#endif//__FMDEC_H__
