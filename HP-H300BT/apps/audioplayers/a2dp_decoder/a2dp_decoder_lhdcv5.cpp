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
// Standard C Included Files
#include "cmsis.h"
#include "plat_types.h"
#include <string.h>
#include "heap_api.h"
#include "hal_location.h"
#include "a2dp_decoder_internal.h"
#include "lhdcv5_util_dec.h"

typedef struct {
    uint16_t sequenceNumber;
    uint32_t timestamp;    
    uint16_t curSubSequenceNumber;
    uint16_t totalSubSequenceNumber;
    uint8_t *buffer;
    uint32_t buffer_len;
} a2dp_audio_lhdcv5_decoder_frame_t;

#define LHDCV5_MTU_LIMITER (100)
#define A2DP_LHDCV5_OUTPUT_FRAME_SAMPLES   (lhdcv5_sample_per_frame)
#define LHDCV5_READBUF_SIZE (512)
#define A2DP_LHDCV5_HDR_FRAME_NO_MASK 0xfc

static A2DP_AUDIO_CONTEXT_T *a2dp_audio_context_p = NULL;
static A2DP_AUDIO_DECODER_LASTFRAME_INFO_T a2dp_audio_lhdcv5_lastframe_info;
static A2DP_AUDIO_OUTPUT_CONFIG_T a2dp_audio_lhdcv5_output_config;
static uint16_t lhdcv5_mtu_limiter = LHDCV5_MTU_LIMITER;
static int lhdcv5_sample_per_frame = 240;
static uint8_t serial_no;
static uint32_t *lhdcv5_util_dec_mem_ptr = NULL;

static void *a2dp_audio_lhdcv5_frame_malloc(uint32_t packet_len)
{
    a2dp_audio_lhdcv5_decoder_frame_t *decoder_frame_p = NULL;
    uint8_t *buffer = NULL;

    buffer = (uint8_t *)a2dp_audio_heap_malloc(packet_len);
    decoder_frame_p = (a2dp_audio_lhdcv5_decoder_frame_t *)a2dp_audio_heap_malloc(sizeof(a2dp_audio_lhdcv5_decoder_frame_t));
    decoder_frame_p->buffer = buffer;
    decoder_frame_p->buffer_len = packet_len;
    return (void *)decoder_frame_p;
}

/**
 * A2DP packet ���
 */
int assemble_lhdcv5_packet(uint8_t *input, uint32_t input_len, uint8_t **pLout, uint32_t *pLlen)
{
    uint8_t hdr = 0, seqno = 0xff;
    int lhdc_total_frame_nb = 0;
    int ret = -1;
    uint32_t status = 0;

    hdr = (*input);
    input++;
    seqno = (*input);
    input++;
    input_len -= 2;

    //Get number of frame in packet.
    status = (hdr & A2DP_LHDCV5_HDR_FRAME_NO_MASK) >> 2;
    if (status <= 0)
    {
        TRACE(0, "No any frame in packet.");
        return 0;
    }
    lhdc_total_frame_nb = status;
    if (seqno != serial_no)
    {
        TRACE(0, "Packet lost! now(%d), expect(%d)", seqno, serial_no);
    }
    serial_no = seqno + 1;

    //sav_lhdcv5_log_bytes_len(input_len);

    if (pLlen && pLout)
    {
        *pLlen = input_len;
        *pLout = input;
    }
    ret = lhdc_total_frame_nb;

    return ret;
}

#ifdef A2DP_CP_ACCEL
struct A2DP_CP_LHDCV5_IN_FRM_INFO_T {
    uint16_t sequenceNumber;
    uint32_t timestamp;
    uint16_t curSubSequenceNumber;
    uint16_t totalSubSequenceNumber;
};

struct A2DP_CP_LHDCV5_OUT_FRM_INFO_T {
    struct A2DP_CP_LHDCV5_IN_FRM_INFO_T in_info;
    uint16_t frame_samples;
    uint16_t decoded_frames;
    uint16_t frame_idx;
    uint16_t pcm_len;
};

extern "C" uint32_t get_in_cp_frame_cnt(void);
extern "C" unsigned int set_cp_reset_flag(uint8_t evt);
extern uint32_t app_bt_stream_get_dma_buffer_samples(void);

int a2dp_cp_lhdcv5_cp_decode(void);

TEXT_LHDC_LOCV5
static int a2dp_cp_lhdcv5_after_cache_underflow(void)
{
    int ret = 0;
    return ret;
}

static int a2dp_cp_lhdcv5_mcu_decode(uint8_t *buffer, uint32_t buffer_bytes)
{
    a2dp_audio_lhdcv5_decoder_frame_t *lhdcv5_decoder_frame_p = NULL;
    list_node_t *node = NULL;
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    int ret, dec_ret;
    struct A2DP_CP_LHDCV5_IN_FRM_INFO_T in_info;
    struct A2DP_CP_LHDCV5_OUT_FRM_INFO_T *p_out_info;

    uint8_t *out;
    uint32_t out_len;
    uint32_t out_frame_len;

    uint32_t cp_buffer_frames_max = 0;
    cp_buffer_frames_max = app_bt_stream_get_dma_buffer_samples()/2;

    if(!a2dp_audio_lhdcv5_lastframe_info.frame_samples)
    {
        a2dp_audio_lhdcv5_lastframe_info.frame_samples = A2DP_LHDCV5_OUTPUT_FRAME_SAMPLES;
    }

    if (cp_buffer_frames_max %(a2dp_audio_lhdcv5_lastframe_info.frame_samples) ){
        cp_buffer_frames_max =  cp_buffer_frames_max /(a2dp_audio_lhdcv5_lastframe_info.frame_samples) +1  ;
    }else{
        cp_buffer_frames_max =  cp_buffer_frames_max /(a2dp_audio_lhdcv5_lastframe_info.frame_samples) ;
    }

    out_frame_len = sizeof(*p_out_info) + buffer_bytes;  

    ret = a2dp_cp_decoder_init(out_frame_len, cp_buffer_frames_max * 8);
    if (ret){
        TRACE(2,"%s: a2dp_cp_decoder_init() failed: ret=%d", __func__, ret);
        set_cp_reset_flag(true);
        return A2DP_DECODER_DECODE_ERROR;
    }

    while ((node = a2dp_audio_list_begin(list)) != NULL) {
        lhdcv5_decoder_frame_p = (a2dp_audio_lhdcv5_decoder_frame_t *)a2dp_audio_list_node(node);

        in_info.sequenceNumber = lhdcv5_decoder_frame_p->sequenceNumber;
        in_info.timestamp = lhdcv5_decoder_frame_p->timestamp;
        in_info.curSubSequenceNumber = lhdcv5_decoder_frame_p->curSubSequenceNumber;
        in_info.totalSubSequenceNumber = lhdcv5_decoder_frame_p->totalSubSequenceNumber;

        ret = a2dp_cp_put_in_frame(&in_info, sizeof(in_info), lhdcv5_decoder_frame_p->buffer, lhdcv5_decoder_frame_p->buffer_len);
        if (ret) {
            //TRACE(2,"%s  piff  !!!!!!ret: %d ",__func__, ret);
            break;
        }
        a2dp_audio_list_remove(list, lhdcv5_decoder_frame_p);    
    }

    ret = a2dp_cp_get_full_out_frame((void **)&out, &out_len);

    if (ret) {
        TRACE(2,"%s %d cp find cache underflow",__func__, __LINE__);
        TRACE(2,"aud_list_len:%d. cp_get_in_frame:%d", a2dp_audio_list_length(list), get_in_cp_frame_cnt());
        a2dp_cp_lhdcv5_after_cache_underflow();
        return A2DP_DECODER_CACHE_UNDERFLOW_ERROR;
    }

    if (out_len == 0) {
        memset(buffer, 0, buffer_bytes);
        a2dp_cp_consume_full_out_frame();  
        TRACE(2,"%s  olz!!!%d ",__func__, __LINE__);
        return A2DP_DECODER_NO_ERROR;      
    }
    if (out_len != out_frame_len){
        TRACE(3, "%s: Bad out len %u (should be %u)", __func__, out_len, out_frame_len);
        set_cp_reset_flag(true);
        return A2DP_DECODER_DECODE_ERROR;
    }
    p_out_info = (struct A2DP_CP_LHDCV5_OUT_FRM_INFO_T *)out;
    if (p_out_info->pcm_len) {
        a2dp_audio_lhdcv5_lastframe_info.sequenceNumber = p_out_info->in_info.sequenceNumber;
        a2dp_audio_lhdcv5_lastframe_info.timestamp = p_out_info->in_info.timestamp;
        a2dp_audio_lhdcv5_lastframe_info.curSubSequenceNumber = p_out_info->in_info.curSubSequenceNumber;
        a2dp_audio_lhdcv5_lastframe_info.totalSubSequenceNumber = p_out_info->in_info.totalSubSequenceNumber;
        a2dp_audio_lhdcv5_lastframe_info.frame_samples = p_out_info->frame_samples;
        a2dp_audio_lhdcv5_lastframe_info.decoded_frames += p_out_info->decoded_frames;
        a2dp_audio_lhdcv5_lastframe_info.undecode_frames =
        a2dp_audio_list_length(list) + a2dp_cp_get_in_frame_cnt_by_index(p_out_info->frame_idx) - 1;
        a2dp_audio_decoder_internal_lastframe_info_set(&a2dp_audio_lhdcv5_lastframe_info);        
    }

     if (p_out_info->pcm_len == buffer_bytes) {
        memcpy(buffer, p_out_info + 1, p_out_info->pcm_len);
        dec_ret = A2DP_DECODER_NO_ERROR;
    } else {
        TRACE(2,"%s  %d cp decoder error  !!!!!!", __func__, __LINE__);
        set_cp_reset_flag(true);
        return A2DP_DECODER_DECODE_ERROR;
    }   

    ret = a2dp_cp_consume_full_out_frame();
    if (ret) {

        TRACE(2, "%s: a2dp_cp_consume_full_out_frame() failed: ret=%d", __func__, ret);
        set_cp_reset_flag(true);
        return A2DP_DECODER_DECODE_ERROR;
    }
    return dec_ret;
}

#ifdef __CP_EXCEPTION_TEST__
static bool  _cp_assert = false;
 int cp_assert(void)
{
    _cp_assert = true;
    return 0;
}
#endif

TEXT_LHDC_LOCV5
int a2dp_cp_lhdcv5_cp_decode(void)
{
    int ret;
    enum CP_EMPTY_OUT_FRM_T out_frm_st;
    uint8_t *out;
    uint32_t out_len;
    uint8_t *dec_start;
    uint32_t dec_len;
    struct A2DP_CP_LHDCV5_IN_FRM_INFO_T *p_in_info;
    struct A2DP_CP_LHDCV5_OUT_FRM_INFO_T *p_out_info;
    uint8_t *in_buf;
    uint32_t in_len;

    int32_t dec_sum;
    uint32_t lhdc_out_len = 0;

#ifdef __CP_EXCEPTION_TEST__
    if (_cp_assert){
         _cp_assert = false;
         *(int*) 0 = 1;
         //ASSERT(0, "ASSERT  %s %d", __func__, __LINE__);
    }
#endif
    out_frm_st = a2dp_cp_get_emtpy_out_frame((void **)&out, &out_len);

    if (out_frm_st != CP_EMPTY_OUT_FRM_OK && out_frm_st != CP_EMPTY_OUT_FRM_WORKING) {
        return out_frm_st;
    } 

    ASSERT(out_len > sizeof(*p_out_info), "%s: Bad out_len %u (should > %u)", __func__, out_len, sizeof(*p_out_info));
    
    p_out_info = (struct A2DP_CP_LHDCV5_OUT_FRM_INFO_T *)out;
    if (out_frm_st == CP_EMPTY_OUT_FRM_OK) {
        p_out_info->pcm_len = 0;
        p_out_info->decoded_frames = 0;
    }

    ASSERT(out_len > sizeof(*p_out_info) + p_out_info->pcm_len, "%s: Bad out_len %u (should > %u + %u)", __func__, out_len, sizeof(*p_out_info), p_out_info->pcm_len);

    dec_start = (uint8_t *)(p_out_info + 1) + p_out_info->pcm_len;
    dec_len = out_len - (dec_start - (uint8_t *)out);

    dec_sum = 0;

    while (dec_sum < (int32_t)dec_len)
    {

        ret = a2dp_cp_get_in_frame((void **)&in_buf, &in_len);

        if (ret) {
            TRACE(1,"cp_get_in_frame fail, ret=%d", ret);
            return 4;
        }

        ASSERT(in_len > sizeof(*p_in_info), "%s: Bad in_len %u (should > %u)", __func__, in_len, sizeof(*p_in_info));

        p_in_info = (struct A2DP_CP_LHDCV5_IN_FRM_INFO_T *)in_buf;
        in_buf += sizeof(*p_in_info);
        in_len -= sizeof(*p_in_info);

        int32_t ret = 0;
        ret = lhdcv5_util_dec_process(lhdcv5_util_dec_mem_ptr, dec_start + dec_sum, in_buf, in_len, &lhdc_out_len);
        dec_sum += lhdc_out_len;
        ret = a2dp_cp_consume_in_frame();

        if (ret != 0) {
            TRACE(2,"%s: a2dp_cp_consume_in_frame() failed: ret=%d", __func__, ret);
        }
        ASSERT(ret == 0, "%s: a2dp_cp_consume_in_frame() failed: ret=%d", __func__, ret);
    
        memcpy(&p_out_info->in_info, p_in_info, sizeof(*p_in_info));
        p_out_info->decoded_frames++;
        p_out_info->frame_samples = A2DP_LHDCV5_OUTPUT_FRAME_SAMPLES;
        p_out_info->frame_idx = a2dp_cp_get_in_frame_index();
    }

    if ( (dec_sum % A2DP_LHDCV5_OUTPUT_FRAME_SAMPLES) ){
        TRACE(2,"error!!! dec_sum:%d  != dec_len:%d", dec_sum, dec_len);
        ASSERT(0, "%s", __func__);
    }

    p_out_info->pcm_len += dec_sum;

    if (out_len <= sizeof(*p_out_info) + p_out_info->pcm_len) {
        ret = a2dp_cp_consume_emtpy_out_frame();
        ASSERT(ret == 0, "%s: a2dp_cp_consume_emtpy_out_frame() failed: ret=%d", __func__, ret);
    }

    return 0;
}
#endif

static int a2dp_audio_lhdcv5_list_checker(void)
{
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    list_node_t *node = NULL;
    a2dp_audio_lhdcv5_decoder_frame_t *lhdcv5_decoder_frame_p = NULL;
    int cnt = 0;

    do {
        lhdcv5_decoder_frame_p = (a2dp_audio_lhdcv5_decoder_frame_t *)a2dp_audio_lhdcv5_frame_malloc(LHDCV5_READBUF_SIZE);
        if (lhdcv5_decoder_frame_p){
            a2dp_audio_list_append(list, lhdcv5_decoder_frame_p);
        }
        cnt++;
    }while(lhdcv5_decoder_frame_p && cnt < LHDCV5_MTU_LIMITER);

    do {
        node = a2dp_audio_list_begin(list);
        if (node){
            lhdcv5_decoder_frame_p = (a2dp_audio_lhdcv5_decoder_frame_t *)a2dp_audio_list_node(node);
            a2dp_audio_list_remove(list, lhdcv5_decoder_frame_p);
        }
    }while(node);

    TRACE(3,"%s cnt:%d list:%d", __func__, cnt, a2dp_audio_list_length(list));

    return 0;
}

int a2dp_audio_lhdcv5_mcu_decode_frame(uint8_t *buffer, uint32_t buffer_bytes)
{
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    list_node_t *node = NULL;
    a2dp_audio_lhdcv5_decoder_frame_t *lhdcv5_decoder_frame_p = NULL;
    
    bool cache_underflow = false;
    int output_byte = 0;
    
    uint32_t lhdc_out_len = 0;

    node = a2dp_audio_list_begin(list);    
    if (!node){
        TRACE(0,"lhdc_decode cache underflow");
        cache_underflow = true;
        goto exit;
    }else{
        lhdcv5_decoder_frame_p = (a2dp_audio_lhdcv5_decoder_frame_t *)a2dp_audio_list_node(node);
        do {
            lhdcv5_util_dec_process(lhdcv5_util_dec_mem_ptr, buffer + output_byte, lhdcv5_decoder_frame_p->buffer, \
                            lhdcv5_decoder_frame_p->buffer_len, &lhdc_out_len);
            if (lhdc_out_len > 0){
                output_byte += lhdc_out_len;
            }else{
                break;
            }
        }while(output_byte < (int)buffer_bytes && output_byte > 0);

        if (output_byte != (int)buffer_bytes){
            TRACE_IMM(3,"[warning] lhdc_decode_frame output_byte:%d lhdc_out_len:%d buffer_bytes:%d", output_byte, lhdc_out_len, buffer_bytes);
            TRACE_IMM(5,"[warning] lhdc_decode_frame frame_len:%d rtp seq:%d timestamp:%d decoder_frame:%d/%d ", 
                                                                                           lhdcv5_decoder_frame_p->buffer_len,
                                                                                           lhdcv5_decoder_frame_p->sequenceNumber,
                                                                                           lhdcv5_decoder_frame_p->timestamp,
                                                                                           lhdcv5_decoder_frame_p->curSubSequenceNumber,
                                                                                           lhdcv5_decoder_frame_p->totalSubSequenceNumber);
            output_byte = buffer_bytes;
            int32_t dump_byte = lhdcv5_decoder_frame_p->buffer_len;
            int32_t dump_offset = 0;
            while(1){
                uint32_t dump_byte_output = 0;
                dump_byte_output = dump_byte>32?32:dump_byte;              
                DUMP8("%02x ", lhdcv5_decoder_frame_p->buffer + dump_offset, dump_byte_output);
                dump_offset += dump_byte_output;
                dump_byte -= dump_byte_output;
                if (dump_byte<=0){
                    break;
                }
            }
            ASSERT(0, "%s", __func__);
        }

        a2dp_audio_lhdcv5_lastframe_info.sequenceNumber = lhdcv5_decoder_frame_p->sequenceNumber;
        a2dp_audio_lhdcv5_lastframe_info.timestamp = lhdcv5_decoder_frame_p->timestamp;
        a2dp_audio_lhdcv5_lastframe_info.curSubSequenceNumber = lhdcv5_decoder_frame_p->curSubSequenceNumber;
        a2dp_audio_lhdcv5_lastframe_info.totalSubSequenceNumber = lhdcv5_decoder_frame_p->totalSubSequenceNumber;
        a2dp_audio_lhdcv5_lastframe_info.frame_samples = A2DP_LHDCV5_OUTPUT_FRAME_SAMPLES;
        a2dp_audio_lhdcv5_lastframe_info.decoded_frames++;
        a2dp_audio_lhdcv5_lastframe_info.undecode_frames = a2dp_audio_list_length(list)-1;
        a2dp_audio_decoder_internal_lastframe_info_set(&a2dp_audio_lhdcv5_lastframe_info);        
        a2dp_audio_list_remove(list, lhdcv5_decoder_frame_p);
    }
exit:
    if(cache_underflow){
        a2dp_audio_lhdcv5_lastframe_info.undecode_frames = 0;
        a2dp_audio_decoder_internal_lastframe_info_set(&a2dp_audio_lhdcv5_lastframe_info);        
        output_byte = A2DP_DECODER_CACHE_UNDERFLOW_ERROR;
    }
    return output_byte;
}

int a2dp_audio_lhdcv5_decode_frame(uint8_t *buffer, uint32_t buffer_bytes)
{
#ifdef A2DP_CP_ACCEL
    return a2dp_cp_lhdcv5_mcu_decode(buffer, buffer_bytes);
#else
    return a2dp_audio_lhdcv5_mcu_decode_frame(buffer, buffer_bytes);
#endif
}

int a2dp_audio_lhdcv5_preparse_packet(btif_media_header_t * header, uint8_t *buffer, uint32_t buffer_bytes)
{
    a2dp_audio_lhdcv5_lastframe_info.sequenceNumber = header->sequenceNumber;
    a2dp_audio_lhdcv5_lastframe_info.timestamp = header->timestamp;
    a2dp_audio_lhdcv5_lastframe_info.curSubSequenceNumber = 0;
    a2dp_audio_lhdcv5_lastframe_info.totalSubSequenceNumber = 0;
    a2dp_audio_lhdcv5_lastframe_info.frame_samples = A2DP_LHDCV5_OUTPUT_FRAME_SAMPLES;
    a2dp_audio_lhdcv5_lastframe_info.list_samples = A2DP_LHDCV5_OUTPUT_FRAME_SAMPLES;
    a2dp_audio_lhdcv5_lastframe_info.decoded_frames = 0;
    a2dp_audio_lhdcv5_lastframe_info.undecode_frames = 0;
    a2dp_audio_decoder_internal_lastframe_info_set(&a2dp_audio_lhdcv5_lastframe_info);

    TRACE(3,"%s seq:%d timestamp:%08x", __func__, header->sequenceNumber, header->timestamp);

    return A2DP_DECODER_NO_ERROR;
}

void a2dp_audio_lhdcv5_free(void *packet)
{
    a2dp_audio_lhdcv5_decoder_frame_t *decoder_frame_p = (a2dp_audio_lhdcv5_decoder_frame_t *)packet; 
    a2dp_audio_heap_free(decoder_frame_p->buffer);
    a2dp_audio_heap_free(decoder_frame_p);
}

int a2dp_audio_lhdcv5_store_packet(btif_media_header_t * header, uint8_t *buffer, uint32_t buffer_bytes)
{
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    int nRet = A2DP_DECODER_NO_ERROR;
    uint32_t frame_num  = 0;
    uint32_t frame_cnt  = 0;
    uint32_t lSize      = 0;
    uint8_t *lPTR       = NULL;
    uint32_t ptr_offset = 0;
    int lhdcv5_ret = 0;
    lhdcv5_frame_Info_t lhdcv5_frame_Info;

    frame_num = assemble_lhdcv5_packet(buffer, buffer_bytes, &lPTR, &lSize);
    if(((a2dp_audio_list_length(list)+frame_num) < lhdcv5_mtu_limiter) && (frame_num > 0))
    {
        for(ptr_offset=0;frame_cnt<frame_num;ptr_offset+=lhdcv5_frame_Info.frame_len, frame_cnt++)
        {
            lhdcv5_ret = lhdcv5_util_dec_fetch_frame_info( lhdcv5_util_dec_mem_ptr, lPTR + ptr_offset, lSize - ptr_offset, &lhdcv5_frame_Info);
            if (!lhdcv5_frame_Info.frame_len || lhdcv5_ret != 0)
            {
                TRACE(0, "[LHDCV5][INPUT] ERROR LHDC FRAME0 ret:%d !!!",lhdcv5_ret);
                DUMP8("%02x ", lPTR + ptr_offset, 12);
                break;
            }
            if (lhdcv5_frame_Info.frame_len + ptr_offset > lSize)
            {
                TRACE(0, "[LHDCV5][INPUT] ERROR LHDC FRAME1 !!!");
                return 0;
            }
            a2dp_audio_lhdcv5_decoder_frame_t *decoder_frame_p = (a2dp_audio_lhdcv5_decoder_frame_t *)a2dp_audio_lhdcv5_frame_malloc(lhdcv5_frame_Info.frame_len);
            if (!decoder_frame_p){
                nRet = A2DP_DECODER_MEMORY_ERROR;
                break;
            }
            decoder_frame_p->sequenceNumber = header->sequenceNumber;
            decoder_frame_p->timestamp = header->timestamp;
            decoder_frame_p->curSubSequenceNumber = frame_cnt;
            decoder_frame_p->totalSubSequenceNumber = frame_num;
            memcpy(decoder_frame_p->buffer, lPTR + ptr_offset, lhdcv5_frame_Info.frame_len);
            decoder_frame_p->buffer_len = lhdcv5_frame_Info.frame_len;
            a2dp_audio_list_append(list, decoder_frame_p);
#if 0
            TRACE(0, "lhdc_store_packet save seq:%d timestamp:%d len:%d lSize:%d frame_len:%d Split:%d/%d",
                                                                                                            header->sequenceNumber,
                                                                                                            header->timestamp,
                                                                                                            buffer_bytes,
                                                                                                            lSize,
                                                                                                            lhdcv5_frame_Info.frame_len,
                                                                                                            lhdcv5_frame_Info.isSplit,
                                                                                                            lhdcv5_frame_Info.isLeft);
#endif
        }
    }else{
        TRACE(0, "[LHDCV5][INPUT] OVERFLOW list:%d,want_insert=%d,limit=%d", a2dp_audio_list_length(list), frame_num, lhdcv5_mtu_limiter);
        nRet = A2DP_DECODER_MTU_LIMTER_ERROR;
    }
    return nRet;
}

int a2dp_audio_lhdcv5_discards_packet(uint32_t packets)
{
#ifdef A2DP_CP_ACCEL
    a2dp_cp_reset_frame();
#endif

    int nRet = a2dp_audio_context_p->audio_decoder.audio_decoder_synchronize_dest_packet_mut(a2dp_audio_context_p->dest_packet_mut);
    return nRet;
}

static int a2dp_audio_lhdcv5_headframe_info_get(A2DP_AUDIO_HEADFRAME_INFO_T* headframe_info)
{
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    list_node_t *node = NULL;
    a2dp_audio_lhdcv5_decoder_frame_t *decoder_frame_p = NULL;

    if (a2dp_audio_list_length(list)){
        node = a2dp_audio_list_begin(list);                
        decoder_frame_p = (a2dp_audio_lhdcv5_decoder_frame_t *)a2dp_audio_list_node(node);
        headframe_info->sequenceNumber = decoder_frame_p->sequenceNumber;
        headframe_info->timestamp = decoder_frame_p->timestamp;
        headframe_info->curSubSequenceNumber = 0;
        headframe_info->totalSubSequenceNumber = 0;
    }else{
        memset(headframe_info, 0, sizeof(A2DP_AUDIO_HEADFRAME_INFO_T));
    }

    return A2DP_DECODER_NO_ERROR;
}

int a2dp_audio_lhdcv5_info_get(void *info)
{
    return A2DP_DECODER_NO_ERROR;
}
int a2dp_audio_lhdcv5_channel_select(A2DP_AUDIO_CHANNEL_SELECT_E chnl_sel){
    lhdcv5_channel_t  channel_type;
    switch(chnl_sel){
        case A2DP_AUDIO_CHANNEL_SELECT_STEREO:
        case A2DP_AUDIO_CHANNEL_SELECT_LRMERGE:
            channel_type = LHDCV5_OUTPUT_STEREO;
            #ifdef A2DP_TRACE_CP_ACCEL
            TRACE_A2DP_DECODER_I("channel select stereo.");
            #endif
            break;
        case A2DP_AUDIO_CHANNEL_SELECT_LCHNL:
            channel_type = LHDCV5_OUTPUT_LEFT_CAHNNEL;
            #ifdef A2DP_TRACE_CP_ACCEL
            TRACE_A2DP_DECODER_I("channel select L channel.");
            #endif
            break;
        case A2DP_AUDIO_CHANNEL_SELECT_RCHNL:
            channel_type = LHDCV5_OUTPUT_RIGHT_CAHNNEL;
            #ifdef A2DP_TRACE_CP_ACCEL
            TRACE_A2DP_DECODER_I("channel select R channel.");
            #endif
            break;
        default:
            #ifdef A2DP_TRACE_CP_ACCEL
            TRACE_A2DP_DECODER_I("Unsupported channel config(%d).", chnl_sel);
            #endif
            return A2DP_DECODER_NOT_SUPPORT;
    }
    lhdcv5_util_dec_channel_selsect(lhdcv5_util_dec_mem_ptr, channel_type);
    return A2DP_DECODER_NO_ERROR;
}
extern uint8_t __lhdc_license_start[];
int a2dp_audio_lhdcv5_init(A2DP_AUDIO_OUTPUT_CONFIG_T *config, void *context)
{   
    uint32_t size = 0;
    uint8_t* pLHDC_lic = (uint8_t*)__lhdc_license_start;
    bool lhdcv5_lic_ret = false;
    //uint8_t savi_uuid[16] = {0};
    pLHDC_lic = pLHDC_lic + 0x98;
    TRACE(5,"%s %s ch:%d freq:%d bits:%d", __func__, lhdcv5_util_dec_get_version(), 
                                                          config->num_channels,
                                                          config->sample_rate, 
                                                          config->bits_depth);

    a2dp_audio_context_p = (A2DP_AUDIO_CONTEXT_T *)context;

    lhdcv5_ver_t version = VERSION_5;
    lhdcv5_util_dec_get_mem_req(version, config->sample_rate, 0, 0, &size);
    lhdcv5_util_dec_mem_ptr = (uint32_t *)a2dp_audio_heap_malloc(size);
    ASSERT(lhdcv5_util_dec_mem_ptr, "lhdcv5_util_dec_mem_ptr = NULL");
    lhdcv5_util_init_decoder(lhdcv5_util_dec_mem_ptr, config->bits_depth, config->sample_rate, 0, 50, 0, version);
    lhdcv5_sample_per_frame = lhdcv5_util_dec_get_sample_size(lhdcv5_util_dec_mem_ptr);
    lhdcv5_lic_ret = lhdcv5_util_set_license(lhdcv5_util_dec_mem_ptr, pLHDC_lic, NULL);
    if(lhdcv5_lic_ret == true)
    {
        TRACE_IMM(1,"%s LHDCV5 license check success.",__func__);
    }
    else
    {
        TRACE_IMM(1,"%s LHDCV5 license check fail.",__func__);
    }
    lhdcv5_util_set_license_check_period (lhdcv5_util_dec_mem_ptr, 5);
    memset(&a2dp_audio_lhdcv5_lastframe_info, 0, sizeof(A2DP_AUDIO_DECODER_LASTFRAME_INFO_T));    
    memcpy(&a2dp_audio_lhdcv5_output_config, config, sizeof(A2DP_AUDIO_OUTPUT_CONFIG_T));
    a2dp_audio_lhdcv5_lastframe_info.stream_info = a2dp_audio_lhdcv5_output_config;
    a2dp_audio_lhdcv5_lastframe_info.frame_samples = A2DP_LHDCV5_OUTPUT_FRAME_SAMPLES;
    a2dp_audio_lhdcv5_lastframe_info.list_samples = A2DP_LHDCV5_OUTPUT_FRAME_SAMPLES;
    a2dp_audio_decoder_internal_lastframe_info_set(&a2dp_audio_lhdcv5_lastframe_info);

#ifdef A2DP_CP_ACCEL
    int ret;
    ret = a2dp_cp_init(a2dp_cp_lhdcv5_cp_decode, CP_PROC_DELAY_2_FRAMES);
    ASSERT(ret == 0, "%s: a2dp_cp_init() failed: ret=%d", __func__, ret);
#endif
    a2dp_audio_lhdcv5_list_checker();
    a2dp_audio_lhdcv5_channel_select(a2dp_audio_context_p->chnl_sel);
    return A2DP_DECODER_NO_ERROR;
}

int a2dp_audio_lhdcv5_deinit(void)
{
#ifdef A2DP_CP_ACCEL
    a2dp_cp_deinit();
#endif
    lhdcv5_util_dec_destroy(lhdcv5_util_dec_mem_ptr);
    a2dp_audio_heap_free(lhdcv5_util_dec_mem_ptr);
    lhdcv5_util_dec_mem_ptr = NULL;
    serial_no = 0xff;
    return A2DP_DECODER_NO_ERROR;
}

int  a2dp_audio_lhdcv5_synchronize_packet(A2DP_AUDIO_SYNCFRAME_INFO_T *sync_info, uint32_t mask)
{
    int nRet = A2DP_DECODER_SYNC_ERROR;
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    list_node_t *node = NULL;
    int list_len;
    a2dp_audio_lhdcv5_decoder_frame_t *lhdcv5_decoder_frame;

#ifdef A2DP_CP_ACCEL
    a2dp_cp_reset_frame();
#endif

    list_len = a2dp_audio_list_length(list);

    for (uint16_t i=0; i<list_len; i++){
        node = a2dp_audio_list_begin(list);
        lhdcv5_decoder_frame = (a2dp_audio_lhdcv5_decoder_frame_t *)a2dp_audio_list_node(node);
        if  (A2DP_AUDIO_SYNCFRAME_CHK(lhdcv5_decoder_frame->sequenceNumber         == sync_info->sequenceNumber,        A2DP_AUDIO_SYNCFRAME_MASK_SEQ,        mask)&&
             A2DP_AUDIO_SYNCFRAME_CHK(lhdcv5_decoder_frame->curSubSequenceNumber   == sync_info->curSubSequenceNumber,  A2DP_AUDIO_SYNCFRAME_MASK_CURRSUBSEQ, mask)&&
             A2DP_AUDIO_SYNCFRAME_CHK(lhdcv5_decoder_frame->totalSubSequenceNumber == sync_info->totalSubSequenceNumber,A2DP_AUDIO_SYNCFRAME_MASK_TOTALSUBSEQ,mask)){
            nRet = A2DP_DECODER_NO_ERROR;
            break;
        }
        a2dp_audio_list_remove(list, lhdcv5_decoder_frame);
    }

    node = a2dp_audio_list_begin(list);
    if (node){
        lhdcv5_decoder_frame = (a2dp_audio_lhdcv5_decoder_frame_t *)a2dp_audio_list_node(node);
        TRACE(6,"%s nRet:%d SEQ:%d timestamp:%d %d/%d", __func__, nRet, lhdcv5_decoder_frame->sequenceNumber, lhdcv5_decoder_frame->timestamp,
                                                      lhdcv5_decoder_frame->curSubSequenceNumber, lhdcv5_decoder_frame->totalSubSequenceNumber);
    }else{
        TRACE(2,"%s nRet:%d", __func__, nRet);
    }

    return nRet;
}


int a2dp_audio_lhdcv5_synchronize_dest_packet_mut(uint16_t packet_mut)
{
    list_node_t *node = NULL;
    uint32_t list_len = 0;
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    a2dp_audio_lhdcv5_decoder_frame_t *lhdcv5_decoder_frame_p = NULL;

    list_len = a2dp_audio_list_length(list);
    if (list_len > packet_mut){
        do{        
            node = a2dp_audio_list_begin(list);            
            lhdcv5_decoder_frame_p = (a2dp_audio_lhdcv5_decoder_frame_t *)a2dp_audio_list_node(node);
            a2dp_audio_list_remove(list, lhdcv5_decoder_frame_p);
        }while(a2dp_audio_list_length(list) > packet_mut);
    }

    TRACE(2,"%s list:%d", __func__, a2dp_audio_list_length(list));
    return A2DP_DECODER_NO_ERROR;
}

int a2dp_audio_lhdcv5_convert_list_to_samples(uint32_t *samples)
{
    uint32_t list_len = 0;
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;

    list_len = a2dp_audio_list_length(list);
    *samples = A2DP_LHDCV5_OUTPUT_FRAME_SAMPLES*list_len;

    TRACE(3,"%s list:%d samples:%d", __func__, list_len, *samples);

    return A2DP_DECODER_NO_ERROR;
}

int a2dp_audio_lhdcv5_discards_samples(uint32_t samples)
{
    int nRet = A2DP_DECODER_SYNC_ERROR;
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    a2dp_audio_lhdcv5_decoder_frame_t *lhdcv5_decoder_frame = NULL;
    list_node_t *node = NULL;
    int need_remove_list = 0;
    uint32_t list_samples = 0;
    ASSERT(!(samples%A2DP_LHDCV5_OUTPUT_FRAME_SAMPLES), "%s samples err:%d", __func__, samples);

    a2dp_audio_lhdcv5_convert_list_to_samples(&list_samples);
    if (list_samples >= samples){
        need_remove_list = samples/A2DP_LHDCV5_OUTPUT_FRAME_SAMPLES;
        for (int i=0; i<need_remove_list; i++){
            node = a2dp_audio_list_begin(list);
            lhdcv5_decoder_frame = (a2dp_audio_lhdcv5_decoder_frame_t *)a2dp_audio_list_node(node);
            a2dp_audio_list_remove(list, lhdcv5_decoder_frame);
        }
        nRet = A2DP_DECODER_NO_ERROR;
    }

    return nRet;
}

A2DP_AUDIO_DECODER_T a2dp_audio_lhdcv5_decoder_config = {
                                                        {96000, 2, 24},
                                                        1,
                                                        a2dp_audio_lhdcv5_init,
                                                        a2dp_audio_lhdcv5_deinit,
                                                        a2dp_audio_lhdcv5_decode_frame,
                                                        a2dp_audio_lhdcv5_preparse_packet,
                                                        a2dp_audio_lhdcv5_store_packet,
                                                        a2dp_audio_lhdcv5_discards_packet,
                                                        a2dp_audio_lhdcv5_synchronize_packet,
                                                        a2dp_audio_lhdcv5_synchronize_dest_packet_mut,
                                                        a2dp_audio_lhdcv5_convert_list_to_samples,
                                                        a2dp_audio_lhdcv5_discards_samples,
                                                        a2dp_audio_lhdcv5_headframe_info_get,
                                                        a2dp_audio_lhdcv5_info_get,
                                                        a2dp_audio_lhdcv5_free,
                                                     } ;
