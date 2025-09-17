/*
 * lhdcv5_util_dec.c
 * 
 * 适用于ESP-IDF的LHDC V5 util解码器实现；
 * 内容包括：帧解析、PCM输出和API接口；
 * 由于无法获得LHDC技术细节，目前核心解码功能是占位符，正常情况下，须链接Savitech LHDC V5官方库；
 * 如获得动态库(lhdcv5_util_dec.so)或静态库(lhdcv5_util_dec.a)，那就不需要本文件了，直接在CMakeLists.txt中链接即可；
 */

#include "lhdcv5_util_dec.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_log.h>

// ------------------------ Internal State ------------------------
static int decoder_initialized = 0;
static uint32_t g_bitPerSample = 16;
static uint32_t g_sampleRate = 44100;
static uint32_t g_scaleTo16Bits = 1;
static uint32_t g_is_lossless_enable = 0;
static lhdc_ver_t g_version = VERSION_5;
static print_log_fp g_log_cb = NULL;
static lhdc_channel_t g_channel_type = LHDC_OUTPUT_STEREO;

static uint8_t *g_mem_ptr = NULL;
static uint32_t g_mem_size = 0;

static const char *TAG = "LHDCV5_UTIL_DEC";

lhdc_ver_t lhdc_ver = VERSION_5;

// ------------------------ Utility Functions ------------------------

static void log_info(const char *msg)
{
    if (g_log_cb) {
        g_log_cb((char *)msg);
    } else {
        ESP_LOGI(TAG, "%s", msg);
    }
}

// ------------------------ API Implementation ------------------------

int32_t lhdcv5_util_init_decoder(uint32_t *ptr, uint32_t bitPerSample, uint32_t sampleRate, uint32_t scaleTo16Bits, uint32_t is_lossless_enable, lhdc_ver_t version)
{
    if (decoder_initialized) {
        lhdcv5_util_dec_destroy();
    }

    if (!ptr) {
        log_info("LHDCV5: Decoder init error (null memory ptr)");
        return LHDCV5_UTIL_DEC_ERROR_PARAM;
    }

    decoder_initialized = 1;
    g_bitPerSample = bitPerSample;
    g_sampleRate = sampleRate;
    g_scaleTo16Bits = scaleTo16Bits;
    g_is_lossless_enable = is_lossless_enable;
    g_version = version;
    g_mem_ptr = (uint8_t *)ptr;

    log_info("LHDCV5: Decoder initialized.");
    return LHDCV5_UTIL_DEC_SUCCESS;
}

int32_t lhdcv5_util_dec_process(uint8_t *pOutBuf, uint8_t *pInput, uint32_t InLen, uint32_t *OutLen)
{
    if (!decoder_initialized)
        return LHDCV5_UTIL_DEC_ERROR_NO_INIT;
    if (!pInput || !pOutBuf || InLen == 0 || !OutLen)
        return LHDCV5_UTIL_DEC_ERROR_PARAM;

    // ---- 1. Parse header (if any, in LHDCV5 one frame) ----
    // For migration, assume no complex header in raw frame, just pure LHDC data.
    // Normally, should parse LHDC frame header, check CRC, get channels/sample info.

    // ---- 2. LHDC V5 Decoding (Placeholder) ----
    // NOTE: You must call LHDC V5 official decoder here!!
    // For demonstration, simulate PCM conversion:
    // - Assume input is compressed LHDC frame of stereo audio.
    // - Output PCM (little-endian), 16/24/32 bits per sample.
    // - One frame: 256 samples per channel (LHDC V5 typical).
    uint32_t sample_bytes = (g_bitPerSample + 7)/8;
    uint32_t frame_samples = 256;
    uint32_t out_frame_bytes = frame_samples * sample_bytes * 2; // stereo

    // 伪解码流程：直接填零或复制输入（仅为技术集成参考）
    if (InLen < out_frame_bytes) {
        // 数据不足，填充为静音
        memset(pOutBuf, 0, out_frame_bytes);
        *OutLen = out_frame_bytes;
        log_info("LHDCV5: Frame too short, output silence.");
        return LHDCV5_UTIL_DEC_SUCCESS;
    }
    // 实际应调用官方库，将LHDC帧解码为PCM输出
    // 例如：lhdcV5_decoder_process(pInput, InLen, pOutBuf, OutLen, ...);
    // 这里模拟输出
    memcpy(pOutBuf, pInput, out_frame_bytes);
    *OutLen = out_frame_bytes;

    log_info("LHDCV5: Decoded one frame.");
    return LHDCV5_UTIL_DEC_SUCCESS;
}

char *lhdcv5_util_dec_get_version()
{
    if (lhdc_ver == VERSION_5)
        return (char *)"LHDCV5";
    else
        return (char *)"LHDC";
}

int32_t lhdcv5_util_dec_destroy()
{
    if (!decoder_initialized)
        return LHDCV5_UTIL_DEC_ERROR_NO_INIT;
    decoder_initialized = 0;
    g_mem_ptr = NULL;
    g_mem_size = 0;
    log_info("LHDCV5: Decoder destroyed.");
    return LHDCV5_UTIL_DEC_SUCCESS;
}

void lhdcv5_util_dec_register_log_cb(print_log_fp cb)
{
    g_log_cb = cb;
    log_info("LHDCV5: Log callback registered.");
}

int32_t lhdcv5_util_dec_get_sample_size(uint32_t *frame_samples)
{
    if (!decoder_initialized)
        return LHDCV5_UTIL_DEC_ERROR_NO_INIT;
    if (!frame_samples)
        return LHDCV5_UTIL_DEC_ERROR_PARAM;

    // LHDC V5: typically 256 samples per frame per channel
    *frame_samples = 256;
    return LHDCV5_UTIL_DEC_SUCCESS;
}

int32_t lhdcv5_util_dec_fetch_frame_info(uint8_t *frameData, uint32_t frameDataLen, lhdc_frame_Info_t *frameInfo)
{
    if (!decoder_initialized)
        return LHDCV5_UTIL_DEC_ERROR_NO_INIT;
    if (!frameData || !frameInfo || frameDataLen == 0)
        return LHDCV5_UTIL_DEC_ERROR_PARAM;
    // 典型LHDC V5帧长度 = 256 samples * 2 channel * (bits/8)
    uint32_t sample_bytes = (g_bitPerSample + 7)/8;
    frameInfo->frame_len = 256 * sample_bytes * 2;
    frameInfo->isSplit = (frameDataLen > frameInfo->frame_len) ? 1 : 0;
    frameInfo->isLeft = 0;
    return LHDCV5_UTIL_DEC_SUCCESS;
}

int32_t lhdcv5_util_dec_channel_selsect(lhdc_channel_t channel_type)
{
    if (!decoder_initialized)
        return LHDCV5_UTIL_DEC_ERROR_NO_INIT;
    g_channel_type = channel_type;
    switch(channel_type){
        case LHDC_OUTPUT_STEREO:
            log_info("LHDCV5: Channel selected (stereo).");
            break;
        case LHDC_OUTPUT_LEFT_CAHNNEL:
            log_info("LHDCV5: Channel selected (left).");
            break;
        case LHDC_OUTPUT_RIGHT_CAHNNEL:
            log_info("LHDCV5: Channel selected (right).");
            break;
        default:
            log_info("LHDCV5: Channel select unknown.");
            break;
    }
    return LHDCV5_UTIL_DEC_SUCCESS;
}

int32_t lhdcv5_util_dec_get_mem_req(lhdc_ver_t version, uint32_t *mem_req_bytes)
{
    if (!mem_req_bytes)
        return LHDCV5_UTIL_DEC_ERROR_PARAM;
    switch (version) {
        case VERSION_5:
            *mem_req_bytes = 4096; // 典型V5解码器所需内存
            break;
        default:
            *mem_req_bytes = 0;
            return LHDCV5_UTIL_DEC_ERROR_PARAM;
    }
    return LHDCV5_UTIL_DEC_SUCCESS;
}
