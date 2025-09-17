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
#ifndef __BLEAPP_SECTIONS_H__
#define __BLEAPP_SECTIONS_H__


#define ALIGN4 __attribute__((aligned(4)))
#define nvrec_mini_version   	1
#define nvrec_dev_magic      	0xba80
#define nvrec_current_version 	2
#define bleapp_SECTOR_SIZE     0x100



typedef struct{
	uint8_t leftclicldata;
	uint8_t rightclicldata;
	uint8_t leftdoubleclicldata;
	uint8_t rightdoubleclicldata;
	uint8_t lefttrubleclicldata;
	uint8_t righttrubleclicldata;
	uint8_t leftfourclicldata;
	uint8_t rightfourclicldata;	
    uint8_t leftlongdata;
    uint8_t rightlongdata;	
	uint8_t setnameflg;
	uint8_t  rev2_bt_name[32];
    uint8_t eqmode;
    uint8_t eq_gain[7];	
	uint8_t flashint;
	uint8_t bt_name_len;
}bleapp_section_data_t;


#ifdef __cplusplus
extern "C" {
#endif

//extern uint8_t bleapp_section_data[7];
extern bleapp_section_data_t bleappdata;
void bleapp_section_init(void);
int bleapp_section_open(void);
int bleapp_data_set();
int bleapp_data_get(bleapp_section_data_t **bleapp_env2);
//int sndkey_px318_data_set(uint8_t PsCtGain_t,uint8_t PsCtDac_t,uint16_t PsCal_t,bool PsCorrectionFlg_t,uint16_t PsThreHigh_t,bool PsThreHighFlg_t,uint16_t PsThreLow_t,bool PsThreLowFlg_t);



#ifdef __cplusplus
}
#endif
#endif


