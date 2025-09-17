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
#ifndef __FJH_APPS_H__
#define __FJH_APPS_H__

//#include "app_status_ind.h"


#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"


#define A2DP_PLAYER_PLAYBACK_DELAY_AAC_MTU_MUSIC 5
#define A2DP_PLAYER_PLAYBACK_DELAY_SBC_MTU_MUSCI 70

#define A2DP_PLAYER_PLAYBACK_DELAY_AAC_MTU_GAME 1
#define A2DP_PLAYER_PLAYBACK_DELAY_SBC_MTU_GAME 5


#define FJH_OSEVENT_UNNOW               0x00
#define FJH_OSEVENT_CLEARPAIRING_RESET  0x01
#define FJH_OSEVENT_TEST_RESET          0x02


typedef struct{
  bool poweron5sflg;
  bool featchoutflg;
  uint8_t link_disconnet;
  uint8_t twslink_disconnet;
  bool moblieconnectedflg;
  bool linklossflg;
  bool leftrighttone_flg;
  bool slaver_reconectflg;
  bool errorpatch01flg;
  bool twsslaverpatch02flg;
  bool moblieconnectedpatch03flg;
  bool charingpoweroffflg;
  bool twsslaverbug01flg;
  bool recnecterrorenterpairflg;
  bool pairingflg;
  bool fjhpatch003flg;
  bool fjhpatch003okflg;
  #ifdef __FJH_PATCH_004__
  bool fjhpatch004flg;
  #endif
  uint8_t bugtime;
  bool bugflg;
  bool poweron_leftright_flg;
  bool earinpatchflg;
  bool boxinoutpatchflg;
  bool btconnectdtoboxcmdflg;
  bool tws_switch_status_flg;
  uint8_t onepowerofftwsswitcherrortime;
  bool disconnet03errorflg;
  bool anckeyenflg;
  #ifdef __POWERONAUTOOPENANC__
  bool poweronautoancflg;
  bool earinchargetophoneflg;
  bool earineventflg;
  #endif
  bool twsdisconnctedenterpairing;
  bool otherearstatus;
  bool waredownflg;
  #ifdef __INOUTBOX_PATCH__	
  bool peerinboxstatus;
  #endif
  bool bugflgtest;
}fjh_apps_data;


#ifdef __LICKEY__
bool lickey_auth(uint8_t* key, int Key_len);
#endif

void app_tws_pairing_fjh2(void);


void app_enter_ota_box(void);
int app_nvrecord_rebuild_fjh(void);
void app_poweron_pairing_fjh(void);
void app_tws_pairing_fjh(void);
#ifdef __ANC_SWITCH_DELAY_FJH__
int32_t app_anc_loop_switch_fjh(void);
#endif
void app_enter_ota_fjh(void);
void app_mobile_connected_fjh(void);
bool app_box_poweron_fjh(void);

void app_box_close_fjh(void);
void app_bt_disconnect_fjh(void);
void app_bt_scan_fjh(void);
#ifdef __FJH_FATCH_OUT_ERROR__
void fatch_delay_set_time(uint32_t millisec);
void fatch_delay_close_time(void);
#endif
void app_tws_disconnect_fjh(void);
void app_tws_connect_fjh(void);
void app_datainit_fjh(void);
void disconnecttone_delay_close(void);
#ifdef __FJH_SLAVER_BUG__
void slaverbug_delay_close_time(void);
void slaverbug_delay_set_time(uint32_t millisec);
#endif
void app_bt_connect_fjh(void);
void app_enter_bttest_fjh(void);
void app_enter_dut_fjh(void);
void app_tws_batterylowpoweroff_fjh(void);
void app_cancel_tws_serach(void);

#ifdef __GAMMODE__
void gamemodeset(void);
void musicmodeset(void);
void gamemusicmode_switch(void);
#endif

void app_poweroff_box(void);
void app_enter_twspairing_fjh(void);
void BT_PAIRING(void);
void app_box_poweroffcharing_fjh(void);
void close_box_test(void);
bool tws_test_boxoff_delay(void);
void bug_delay_close_time(void);

#ifdef __ANC_SWITCH_DELAY_FJH__
void app_anc_tone_init_timer(void);
#endif
void app_poweroff_box(void);
void spp_cmd_poweroff(void);
void set_disconnet03errorflg(void);
void app_box_in_fjh(void);
bool app_box_out_fjh(void);
void set_anckeytongdelay(void);

void app_clear_btpailist_fjh(bool appdata);


void set_battery_lev_to_peer(uint8_t lev);

void sync_key_cmd(void);
void sync_btname_cmd(uint8_t* ptrData, uint32_t dataLength);
void sync_eq_cmd(uint8_t* ptrData, uint32_t dataLength);
int app_nvrecord_rebuild_app(bool appdata);
void sync_ring_cmd(uint8_t onoff);



void sync_pre_cmd(void);
void sync_nex_cmd(void);
void sync_paly_cmd(void);
void sync_stop_cmd(void);



#ifdef __INOUTBOX_STATUS_APP_PATCH__
void send_battery_status_to_app(void);
#endif	
#ifdef __RELEASE_A2DP_VOLUME__
void release_a2dp_volume(void);
#endif
void sync_sco_volume_cmd(void);

void sco_volume_sync(void);



#ifdef __TWSPAIRLISTNV__
extern bool factory_tws_pairing_flg;
#endif



#ifdef __CLOSEBOX_TWS_SWITCH_PATCH__
extern bool twsswitchok_enterclosebox;
#endif







//void fjhapp_thread_enable(void);
//extern osThreadId fjhapp_thread_tid;
extern fjh_apps_data apps_data;


#ifdef __cplusplus
}
#endif
#endif//__FMDEC_H__
