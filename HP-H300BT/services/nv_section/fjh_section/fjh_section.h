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
#ifndef __FJH_SECTIONS_H__
#define __FJH_SECTIONS_H__


#define ALIGN4 __attribute__((aligned(4)))
#define nvrec_mini_version   	1
#define nvrec_dev_magic      	0xba80
#define nvrec_current_version 	2
#define twslistkey_SECTOR_SIZE     0x100



typedef struct{
	uint8_t tws_ibrt_bt_addr_nv[6];
	uint8_t twspairenableflg;
	uint8_t nvrole;
}twslistkey_section_data_t;


#ifdef __cplusplus
extern "C" {
#endif

extern uint8_t twslistkey_section_data[8];
void twslistkey_section_init(void);
int twslistkey_section_open(void);
int twslistkey_data_set();
int twslistkey_data_get(twslistkey_section_data_t **twslistkey_env2);
//int sndkey_px318_data_set(uint8_t PsCtGain_t,uint8_t PsCtDac_t,uint16_t PsCal_t,bool PsCorrectionFlg_t,uint16_t PsThreHigh_t,bool PsThreHighFlg_t,uint16_t PsThreLow_t,bool PsThreLowFlg_t);



#ifdef __cplusplus
}
#endif
#endif


