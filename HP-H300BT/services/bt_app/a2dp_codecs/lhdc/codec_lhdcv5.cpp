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
#ifdef A2DP_LHDCV5_ON

#include <stdio.h>
#include "cmsis_os.h"
#include "hal_uart.h"
#include "hal_timer.h"
#include "audioflinger.h"
#include "lockcqueue.h"
#include "hal_trace.h"
#include "hal_cmu.h"
#include "analog.h"
#include "bt_drv.h"
#include "app_audio.h"
#include "bt_drv_interface.h"
#include "app_bt_stream.h"
#include "nvrecord.h"
#include "nvrecord_env.h"

#include "spp_api.h"
#include "hal_chipid.h"
#include "bt_drv_reg_op.h"
#include "besbt.h"
#include "cqueue.h"
#include "btapp.h"
#include "app_bt.h"
#include "apps.h"
#include "resources.h"
#include "app_bt_media_manager.h"
#include "tgt_hardware.h"
#ifdef __TWS__
#include "app_tws.h"
#include "app_bt.h"
#endif

#include "codec_lhdcv5.h"

void lhdcv5_info_parse(uint8_t * elements, lhdcv5_info_t * info)
{
    info->vendor_id = (uint32_t) elements[0];
    info->vendor_id |= ((uint32_t) elements[1]) << 8;
    info->vendor_id |= ((uint32_t) elements[2]) << 16;
    info->vendor_id |= ((uint32_t) elements[3]) << 24;

    info->codec_id = (uint16_t) elements[4];
    info->codec_id |= ((uint16_t) elements[5]) << 8;
    uint8_t config = elements[6];

    if (info->vendor_id == A2DP_LHDCV5_VENDOR_ID && info->codec_id == A2DP_LHDCV5_CODEC_ID) {

        TRACE(2,"Vendor ID = 0x%08x, Codec ID = 0x%04x, LHDC Codec\n", info->vendor_id, info->codec_id);
        switch (A2DP_LHDCV5_SR_DATA(config)) {
            case A2DP_LHDCV5_SR_96000:
                info->sample_rater = A2D_SBC_IE_SAMP_FREQ_96;
                TRACE(1,"%s:CodecCfg sample_rate 96000\n", __func__);
                break;
            case A2DP_LHDCV5_SR_48000:
                info->sample_rater = A2D_SBC_IE_SAMP_FREQ_48;
                TRACE(1,"%s:CodecCfg sample_rate 48000\n", __func__);
                break;
            case A2DP_LHDCV5_SR_44100:
                info->sample_rater = A2D_SBC_IE_SAMP_FREQ_44;
                TRACE(1,"%s:CodecCfg sample_rate 44100\n", __func__);
                break;
        }

        config = elements[7];
        switch (A2DP_LHDCV5_FMT_DATA(config)) {
            case A2DP_LHDCV5_FMT_16:
                info->bits = 16;
                TRACE(1,"%s:CodecCfg bits per sampe = 16", __func__);
                break;
            case A2DP_LHDCV5_FMT_24:
                TRACE(1,"%s:CodecCfg bits per sampe = 24", __func__);
                info->bits = 24;
                break;
        }
        config = elements[8];
        info->version_num = 0x0f & config;
        TRACE(2,"%s:lhdc codec version num:%x\n", __func__, info->version_num);
    }
}

uint8_t a2dp_lhdcv5_get_sample_rate(uint8_t * elements)
{
    uint32_t vendor_id = (uint32_t) elements[0];
    vendor_id |= ((uint32_t) elements[1]) << 8;
    vendor_id |= ((uint32_t) elements[2]) << 16;
    vendor_id |= ((uint32_t) elements[3]) << 24;
    uint16_t codec_id = (uint16_t) elements[4];
    codec_id |= ((uint16_t) elements[5]) << 8;

    uint8_t config = elements[6];
    if (vendor_id == A2DP_LHDCV5_VENDOR_ID && codec_id == A2DP_LHDCV5_CODEC_ID) {
	switch (A2DP_LHDCV5_SR_DATA(config)) {
	case A2DP_LHDCV5_SR_96000:
	    return A2D_SBC_IE_SAMP_FREQ_96;
	case A2DP_LHDCV5_SR_48000:
	    return A2D_SBC_IE_SAMP_FREQ_48;
	case A2DP_LHDCV5_SR_44100:
	    return A2D_SBC_IE_SAMP_FREQ_44;
	}
    }
    return 0;
}

#endif
