/*
 * lhdcv5_util_dec.c
 *
 * 根据lhdcv5_util_dec.h模拟LHDCV5解码器util函数的实现，只是具备一个函数框架；
 * 仅用于通过编译，避免找不到定义而报错，不实际解码LHDC音频；
 * 如获得真正的lhdcv5_util_dec.c来代替此文件，LHDCV5便有可能可以工作；
 * 目前正在逐步分析，试图写出本文件中对应函数的具体内容
 */

#include "lhdcv5_util_dec.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_log.h>

// Internal state
static int decoder_initialized = 0;
static uint32_t g_bitPerSample = 16;
static uint32_t g_sampleRate = 44100;
static uint32_t g_scaleTo16Bits = 1;
static uint32_t g_is_lossless_enable = 0;
static lhdc_ver_t g_version = VERSION_5;
static print_log_fp g_log_cb = NULL;
static lhdc_channel_t g_channel_type = LHDC_OUTPUT_STEREO;
static const char *TAG = "LHDCV5_UTIL_DEC";

lhdc_ver_t lhdc_ver = VERSION_5; // 定义为LHDCV5版本

// ===== 模拟的函数定义，无任何解码功能 =====

int32_t lhdcv5_util_init_decoder(uint32_t *ptr, uint32_t bitPerSample, uint32_t sampleRate, uint32_t scaleTo16Bits, uint32_t is_lossless_enable, lhdc_ver_t version) {
    decoder_initialized = 1;
    g_bitPerSample = bitPerSample;
    g_sampleRate = sampleRate;
    g_scaleTo16Bits = scaleTo16Bits;
    g_is_lossless_enable = is_lossless_enable;
    g_version = version;
    if (g_log_cb) ESP_LOGI(TAG, "LHDCV5: Decoder initialized.");
    return LHDCV5_UTIL_DEC_SUCCESS;
}

int32_t lhdcv5_util_dec_process(uint8_t * pOutBuf, uint8_t * pInput, uint32_t InLen, uint32_t *OutLen) {
    if (!decoder_initialized) return LHDCV5_UTIL_DEC_ERROR_NO_INIT;
    // Just copy input to output as mock
    if (!pInput || !pOutBuf || InLen == 0 || !OutLen) return LHDCV5_UTIL_DEC_ERROR_PARAM;
    memcpy(pOutBuf, pInput, InLen);
    *OutLen = InLen; // 1:1 模拟
    if (g_log_cb) ESP_LOGI(TAG, "LHDCV5: Decoded one frame (mock).");
    return LHDCV5_UTIL_DEC_SUCCESS;
}

/**
 * @brief 获取解码器版本（功能应该没问题）
 * 
 * @return char* 版本字符串
 */
char *lhdcv5_util_dec_get_version() {
    if (lhdc_ver == VERSION_5) return (char *)"LHDCV5"; 
    else return (char *)"LHDC";
}

int32_t lhdcv5_util_dec_destroy() {
    if (!decoder_initialized) return LHDCV5_UTIL_DEC_ERROR_NO_INIT;
    decoder_initialized = 0;
    if (g_log_cb) ESP_LOGI(TAG, "LHDCV5: Decoder destroyed.");
    return LHDCV5_UTIL_DEC_SUCCESS;
}

void lhdcv5_util_dec_register_log_cb(print_log_fp cb) {
    g_log_cb = cb;
    if (g_log_cb) ESP_LOGI(TAG, "LHDCV5: Log callback registered.");
}

/**
 * @brief 获取一帧音频的样本数（功能未实现）
 * 
 * @param frame_samples 指向样本数的指针
 * @return int32_t 错误码
 */
int32_t lhdcv5_util_dec_get_sample_size(uint32_t *frame_samples) {
    if (!decoder_initialized) return LHDCV5_UTIL_DEC_ERROR_NO_INIT;
    if (!frame_samples) return LHDCV5_UTIL_DEC_ERROR_PARAM;
    *frame_samples = 256; // 这里直接赋值，随便填的值，只是为了通过编译，也有可能是依据某些内容解析判断后再赋值
    return LHDCV5_UTIL_DEC_SUCCESS;
}

int32_t lhdcv5_util_dec_fetch_frame_info(uint8_t *frameData, uint32_t frameDataLen, lhdc_frame_Info_t *frameInfo) {
    if (!decoder_initialized) return LHDCV5_UTIL_DEC_ERROR_NO_INIT;
    if (!frameData || !frameInfo || frameDataLen == 0) return LHDCV5_UTIL_DEC_ERROR_PARAM;
    frameInfo->frame_len = frameDataLen;
    frameInfo->isSplit = (frameDataLen > 128) ? 1 : 0; // 模拟的逻辑
    frameInfo->isLeft = 0;
    return LHDCV5_UTIL_DEC_SUCCESS;
}

/**
 * @brief 选择解码器通道（功能未实现）
 * 
 * @param channel_type 通道类型
 * @return int32_t 错误码
 */
int32_t lhdcv5_util_dec_channel_selsect(lhdc_channel_t channel_type) {
    if (!decoder_initialized) return LHDCV5_UTIL_DEC_ERROR_NO_INIT;
    g_channel_type = channel_type;
    if (channel_type == LHDC_OUTPUT_STEREO) {
        ESP_LOGI(TAG, "LHDCV5: Channel selected (stereo).");
        // 处理选择立体声时的情况，具体内容待实现
    } else if (channel_type == LHDC_OUTPUT_LEFT_CAHNNEL) {
        ESP_LOGI(TAG, "LHDCV5: Channel selected (left channel).");
        // 处理选择左声道时的情况，具体内容待实现
    } else if (channel_type == LHDC_OUTPUT_RIGHT_CAHNNEL) {
        ESP_LOGI(TAG, "LHDCV5: Channel selected (right channel).");
        // 处理选择右声道时的情况，具体内容待实现
    }
    return LHDCV5_UTIL_DEC_SUCCESS;
}

int32_t lhdcv5_util_dec_get_mem_req(lhdc_ver_t version, uint32_t *mem_req_bytes) {
    if (!mem_req_bytes) return LHDCV5_UTIL_DEC_ERROR_PARAM;
    switch (version) {
        case VERSION_5:
            *mem_req_bytes = 4096; // 模拟的value
            break;
        default:
            *mem_req_bytes = 0;
            return LHDCV5_UTIL_DEC_ERROR_PARAM;
    }
    return LHDCV5_UTIL_DEC_SUCCESS;
}
