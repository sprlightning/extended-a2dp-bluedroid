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
#include "cmsis.h"
#include "plat_types.h"
#include "tgt_hardware.h"
#include "string.h"
#include "crc32.h"
#include "bleapp_section.h"
#include "pmu.h"
#include "hal_trace.h"
#include "hal_norflash.h"
#include "norflash_api.h"
#include "heap_api.h"

extern uint32_t __bleapp_start[];
extern uint32_t __bleapp_end[];

static bleapp_section_data_t *bleapp_section_p = NULL;


bleapp_section_data_t bleappdata;


//uint8_t bleapp_section_data[7] = {0};


static void bleapp_callback(void* param)
{
    NORFLASH_API_OPERA_RESULT *opera_result;

    opera_result = (NORFLASH_API_OPERA_RESULT*)param;

    TRACE(5,"%s:type = %d, addr = 0x%x,len = 0x%x,result = %d.",
                __func__,
                opera_result->type,
                opera_result->addr,
                opera_result->len,
                opera_result->result);
}



void bleapp_section_init(void)
{
    enum NORFLASH_API_RET_T result;
    uint32_t sector_size = 0;
    uint32_t block_size = 0;
    uint32_t page_size = 0;

    hal_norflash_get_size(HAL_FLASH_ID_0,
               NULL,
               &block_size,
               &sector_size,
               &page_size);

	            sector_size = 0x100;
                    result = norflash_api_register(NORFLASH_API_MODULE_ID_BLEAPP,
                    HAL_FLASH_ID_0,
                    ((uint32_t)__bleapp_start)&0x00FFFFFF,
                    (uint32_t)__bleapp_end - (uint32_t)__bleapp_start,
                    block_size,
                    sector_size,
                    page_size,
                    bleapp_SECTOR_SIZE,
                    bleapp_callback
                    );

    TRACE(5,"block_size = 0x%x, sector_size = 0x%x,page_size = 0x%x,sndkey_SECTOR_SIZE = 0x%x.",block_size,sector_size,page_size,bleapp_SECTOR_SIZE);

	//result = NORFLASH_API_OK;
   ASSERT(result == NORFLASH_API_OK,"nv_record_init: module register failed! result = %d.",result);
}



int bleapp_section_open(void)
{
    bleapp_section_p = (bleapp_section_data_t *)__bleapp_start;
/*
     TRACE(0,"sndkey_section_p->p_sensor2.PsCtGain = %d",sndkey_section_p->p_sensor2.PsCtGain);
	TRACE(0,"sndkey_section_p->p_sensor2.PsCtDac = %d",sndkey_section_p->p_sensor2.PsCtDac);
	TRACE(0,"sndkey_section_p->p_sensor2.PsCal = %d",sndkey_section_p->p_sensor2.PsCal);
	TRACE(0,"sndkey_section_p->p_sensor2.PsCorrectionflg = %d",sndkey_section_p->p_sensor2.PsCorrectionflg);
	TRACE(0,"sndkey_section_p->p_sensor2.PsThreHigh_P = %d",sndkey_section_p->p_sensor2.PsThreHigh_P);
	TRACE(0,"sndkey_section_p->p_sensor2.PsThreHighflg = %d",sndkey_section_p->p_sensor2.PsThreHighflg);
	TRACE(0,"sndkey_section_p->p_sensor2.PsThreLow_p = %d",sndkey_section_p->p_sensor2.PsThreLow_p);
	TRACE(0,"sndkey_section_p->p_sensor2.PsThreLowflg = %d",sndkey_section_p->p_sensor2.PsThreLowflg);
*/
    return 0;
}


//int sndkey_data_set(void)


int bleapp_data_get(bleapp_section_data_t **bleapp_env2)
{
   // *sndkey_env2 = &(sndkey_section_p->p_sensor2);
   //sndkey_section_p = (sndkey_section_data_t *)__sndkey_start;
   //*sndkey_env2 = &sndkey_section_p;
   
   *bleapp_env2 = bleapp_section_p;
   return 0;
}



int bleapp_data_set(void)
{
    //uint8_t *mempool = NULL;
    uint32_t lock;
    enum NORFLASH_API_RET_T ret;

    if (bleapp_section_p){

        lock = int_lock_global();
        ret = norflash_api_erase(NORFLASH_API_MODULE_ID_BLEAPP,(uint32_t)(__bleapp_start)&0x00FFFFFF,0x100,false);
        ASSERT(ret == NORFLASH_API_OK,"factory_section_xtal_fcap_set: erase failed! ret = %d.",ret);
        ret = norflash_api_write(NORFLASH_API_MODULE_ID_BLEAPP,(uint32_t)(__bleapp_start)&0x00FFFFFF,(uint8_t *)&bleappdata,0x100,false);
		
        ASSERT(ret == NORFLASH_API_OK,"factory_section_xtal_fcap_set: write failed! ret = %d.",ret);

        int_unlock_global(lock);

        return 0;
    }else{
        return -1;
    }
}



#if 0
int sndkey_px318_data_set(uint8_t PsCtGain_t,uint8_t PsCtDac_t,uint16_t PsCal_t,bool PsCorrectionFlg_t,uint16_t PsThreHigh_t,bool PsThreHighFlg_t,uint16_t PsThreLow_t,bool PsThreLowFlg_t)
{
    uint32_t lock;
    enum NORFLASH_API_RET_T ret;

    if (sndkey_section_p){
		


		//memcpy(px318data.sndpkey,__sndkey_start, sndkey_SECTOR_SIZE);

        //memcpy(g_ct101_user_sector_buf, (void* )CT101_USER_SECTOR_ADDR, CT101_USER_SECTOR_SIZE);
        lock = int_lock_global();

        ret = norflash_api_erase(NORFLASH_API_MODULE_ID_FJH,(uint32_t)(__sndkey_start)&0x00FFFFFF,0x100,false);
        ASSERT(ret == NORFLASH_API_OK,"factory_section_xtal_fcap_set: erase failed! ret = %d.",ret);
        ret = norflash_api_write(NORFLASH_API_MODULE_ID_FJH,(uint32_t)(__sndkey_start)&0x00FFFFFF,(uint8_t *)&px318data,0x100,false);
        ASSERT(ret == NORFLASH_API_OK,"factory_section_xtal_fcap_set: write failed! ret = %d.",ret);

#if defined(ELEVOC_BLE)
        ELEVOC_SECTION_DATA_T* elevoc_param_ptr;
        elevoc_param_ptr = (ELEVOC_SECTION_DATA_T* )&g_ct101_user_sector_buf[CT101_USER_ELEVOC_PARAM_OFFSET];
        ret = norflash_api_write(NORFLASH_API_MODULE_ID_ELEVOC_BLE, (uint32_t)(__elevoc_start) & 0x00FFFFFF,
            (uint8_t* )elevoc_param_ptr, sizeof(ELEVOC_SECTION_DATA_T), false);
        ASSERT(ret == NORFLASH_API_OK, "[ELEVOC][ELEVOC_BLE]elevoc_section_data_set: elevoc_param_ptr failed! ret = %d.",ret);
#endif

#if defined(ELEVOC)
        ELEVOC_AUTH_KEY_T* elevoc_key_ptr;
        elevoc_key_ptr = (ELEVOC_AUTH_KEY_T* )&g_ct101_user_sector_buf[CT101_USER_ELEVOC_KEY_OFFSET];
        ret = norflash_api_write(NORFLASH_API_MODULE_ID_ELEVOC_AUTH, (uint32_t)(__elevoc_auth_key_start) & 0x00FFFFFF,
            (uint8_t* )elevoc_key_ptr, sizeof(ELEVOC_AUTH_KEY_T), false);
        ASSERT(ret == NORFLASH_API_OK, "[ELEVOC][ELEVOC_BLE]elevoc_section_data_set: elevoc_key_ptr failed! ret = %d.",ret);
#endif

        int_unlock_global(lock);

        return 0;
    }else{
        return -1;
    }
}





bool factory_get_px318data_flg(void)
{
    if(factory_section_p){
        TRACE(0,"get factory_get_px318data_flg");
        if (1 == nv_record_dev_rev)
        {

        }
        else
        {
        	 return factory_section_p->data.p_sensor2.PsCorrectionflg;
        }
    }else{
        TRACE(0,"get factory_get_px318data_flg_ERROR");
    }
	return true;
}



uint8_t factory_get_px318data_PsCtGain_t(void)
{
    if(factory_section_p){
        TRACE(0,"get factory_get_px318data_flg");
        if (1 == nv_record_dev_rev)
        {

        }
        else
        {
        	 return factory_section_p->data.p_sensor2.PsCtGain;
        }
    }else{
        TRACE(0,"get factory_get_px318data_flg_ERROR");
    }
	return true;
}

uint8_t factory_get_px318data_PsCtDac_t(void)
{
    if(factory_section_p){
        TRACE(0,"get factory_get_px318data_flg");
        if (1 == nv_record_dev_rev)
        {

        }
        else
        {
        	 return factory_section_p->data.p_sensor2.PsCtDac;
        }
    }else{
        TRACE(0,"get factory_get_px318data_flg_ERROR");
    }
	return true;
}

uint8_t factory_get_px318data_PsCal_t(void)
{
    if(factory_section_p){
        TRACE(0,"get factory_get_px318data_flg");
        if (1 == nv_record_dev_rev)
        {

        }
        else
        {
        	 return factory_section_p->data.p_sensor2.PsCal;
        }
    }else{
        TRACE(0,"get factory_get_px318data_flg_ERROR");
    }
	return true;
}




/*******************************************************************************************/
uint16_t factory_get_px318data_PsThreHigh_P(void)
{
    if(factory_section_p){
        TRACE(0,"get factory_get_px318data_flg");
        if (1 == nv_record_dev_rev)
        {

        }
        else
        {
        	 return factory_section_p->data.p_sensor2.PsThreHigh_P;
        }
    }else{
        TRACE(0,"get factory_get_px318data_flg_ERROR");
    }
	return true;
}
bool factory_get_px318data_PsThreHighflg(void)
{
    if(factory_section_p){
        TRACE(0,"get factory_get_px318data_flg");
        if (1 == nv_record_dev_rev)
        {

        }
        else
        {
        	 return factory_section_p->data.p_sensor2.PsThreHighflg;
        }
    }else{
        TRACE(0,"get factory_get_px318data_flg_ERROR");
    }
	return true;
}
uint16_t factory_get_px318data_PsThreLow_p(void)
{
    if(factory_section_p){
        TRACE(0,"get factory_get_px318data_flg");
        if (1 == nv_record_dev_rev)
        {

        }
        else
        {
        	 return factory_section_p->data.p_sensor2.PsThreLow_p;
        }
    }else{
        TRACE(0,"get factory_get_px318data_flg_ERROR");
    }
	return true;
}
bool factory_get_px318data_PsThreLowflg(void)
{
    if(factory_section_p){
        TRACE(0,"get factory_get_px318data_flg");
        if (1 == nv_record_dev_rev)
        {

        }
        else
        {
        	 return factory_section_p->data.p_sensor2.PsThreLowflg;
        }
    }else{
        TRACE(0,"get factory_get_px318data_flg_ERROR");
    }
	return true;
}

/*******************************************************************************************/





int factory_px318_data_set(uint8_t PsCtGain_t,uint8_t PsCtDac_t,uint16_t PsCal_t,bool PsCorrectionFlg_t,uint16_t PsThreHigh_t,bool PsThreHighFlg_t,uint16_t PsThreLow_t,bool PsThreLowFlg_t)
{
    uint8_t *mempool = NULL;
    uint32_t lock;
    enum NORFLASH_API_RET_T ret;

    if (factory_section_p){
       // TRACE(1,"factory_section_xtal_fcap_set:%d", xtal_fcap);
        syspool_init();
        syspool_get_buff((uint8_t **)&mempool, 0x1000);
        memcpy(mempool, factory_section_p, 0x1000);
        if (1 == nv_record_dev_rev)
        {
        	//((factory_section_t *)mempool)->data.xtal_fcap = xtal_fcap;
            /*
        	((factory_section_t *)mempool)->data.p_sensor2.PsCtGain = PsCtGain_t;
			((factory_section_t *)mempool)->data.p_sensor2.PsCtDac = PsCtDac_t;
			((factory_section_t *)mempool)->data.p_sensor2.PsCal = PsCal_t;
			((factory_section_t *)mempool)->data.p_sensor2.PsCorrectionflg = PsCorrectionFlg_t;
			((factory_section_t *)mempool)->data.p_sensor2.PsThreHigh_P = PsThreHigh_t;
			((factory_section_t *)mempool)->data.p_sensor2.PsThreHighflg = PsThreHighFlg_t;
			((factory_section_t *)mempool)->data.p_sensor2.PsThreLow_p = PsThreLow_t;
			((factory_section_t *)mempool)->data.p_sensor2.PsThreLowflg = PsThreLowFlg_t;
              */


		
        	((factory_section_t *)mempool)->head.crc = crc32(0,(unsigned char *)(&(((factory_section_t *)mempool)->head.reserved0)),sizeof(factory_section_t)-2-2-4);
		}
		else
		{
        	//((factory_section_t *)mempool)->data.rev2_xtal_fcap = xtal_fcap;
        	((factory_section_t *)mempool)->data.p_sensor2.PsCtGain = PsCtGain_t;
			((factory_section_t *)mempool)->data.p_sensor2.PsCtDac = PsCtDac_t;
			((factory_section_t *)mempool)->data.p_sensor2.PsCal = PsCal_t;
			((factory_section_t *)mempool)->data.p_sensor2.PsCorrectionflg = PsCorrectionFlg_t;
			((factory_section_t *)mempool)->data.p_sensor2.PsThreHigh_P = PsThreHigh_t;
			((factory_section_t *)mempool)->data.p_sensor2.PsThreHighflg = PsThreHighFlg_t;
			((factory_section_t *)mempool)->data.p_sensor2.PsThreLow_p = PsThreLow_t;
			((factory_section_t *)mempool)->data.p_sensor2.PsThreLowflg = PsThreLowFlg_t;        	
        	((factory_section_t *)mempool)->data.rev2_crc =
        		crc32(0,(unsigned char *)(&(factory_section_p->data.rev2_reserved0)),
				factory_section_p->data.rev2_data_len);
		}
        lock = int_lock_global();

        ret = norflash_api_erase(NORFLASH_API_MODULE_ID_FACTORY,(uint32_t)(__factory_start)&0x00FFFFFF,0x1000,false);
        ASSERT(ret == NORFLASH_API_OK,"factory_section_xtal_fcap_set: erase failed! ret = %d.",ret);
        ret = norflash_api_write(NORFLASH_API_MODULE_ID_FACTORY,(uint32_t)(__factory_start)&0x00FFFFFF,(uint8_t *)mempool,0x1000,false);
        ASSERT(ret == NORFLASH_API_OK,"factory_section_xtal_fcap_set: write failed! ret = %d.",ret);

        int_unlock_global(lock);

        return 0;
    }else{
        return -1;
    }
}
#endif


