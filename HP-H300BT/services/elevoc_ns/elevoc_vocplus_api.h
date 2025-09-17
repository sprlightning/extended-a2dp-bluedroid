/*
 * =============================================================================
 * Copyright (c) 2017-2021 Elevoc Technologies, Inc.
 * All rights reserved. Elevoc Proprietary and Confidential.
 * =============================================================================
 */

#ifndef ELEVOC_VOCPLUS_API_H
#define	ELEVOC_VOCPLUS_API_H

#include "elevoc_typedef.h"

#define VOCPLUS_FRAME_MS	SPEECH_PROCESS_FRAME_MS

#ifdef __cplusplus
extern "C"
{
#endif


ele_error elevoc_vocplus_initial(ele_uint32 sample_rate);
ele_error elevoc_vocplus_release(void);
ele_error elevoc_vocplus_process(short** near_in, short** far_in, short** near_out, unsigned int size);
ele_error elevoc_vocplus_co_process(void);
ele_error elevoc_vocplus_set_param(ele_uint32 t_id, void* data, ele_uint32 t_size);
ele_error elevoc_vocplus_get_param(ele_uint32 t_id, void* data, ele_uint32 t_size);

ele_error elevoc_vocplus_rx_initial(ele_uint32 sample_rate);
ele_error elevoc_vocplus_rx_process(short* pcm, unsigned int size);

#ifdef __cplusplus
}
#endif

#endif

