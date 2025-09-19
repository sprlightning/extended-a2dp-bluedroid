/**
 * SPDX-FileCopyrightText: 2025 The Android Open Source Project
 * 
 * SPDX-License-Identifier: Apache-2.0
 * 
 * a2dp_vendor_lhdcv5_decoder.c
 * 
 * 本文件依赖外部库liblhdcv5dec中的lhdcv5BT_dec.h
 * 库liblhdcv5dec中的lhdcv5_util_dec.c功能未实现，只有框架，不具备解码能力
 * 需要实现lhdcv5_util_dec.c中所有内容，或者使用动态库lhdcv5_util_dec.so、lhdcv5BT_dec.so
 */

// #include <string.h>
// #include <stdint.h>
#include "common/bt_trace.h"
#include "stack/a2dp_vendor_lhdcv5.h"
#include "stack/a2dp_vendor_lhdc_constants.h"
#include "stack/a2dp_vendor_lhdcv5_constants.h"
#include "stack/a2dp_vendor_lhdcv5_decoder.h"

// #include "osi/osi.h"
// #include "common/bt_target.h"

#if (defined(LHDCV5_DEC_INCLUDED) && LHDCV5_DEC_INCLUDED == TRUE)

/* Encode Quality Mode Index. (EQMID)
 *  The configuration of encoding in LHDCV5 will be coordinated by "Encode Quality Mode Index"
 *  parameter. Configurable values are shown below.
 *   - LHDCV5BT_EQMID_HQ : Encode setting for High Quality.
 *   - LHDCV5BT_EQMID_SQ : Encode setting for Standard Quality.
 *   - LHDCV5BT_EQMID_MQ : Encode setting for Mobile use Quality.
 */
enum {
    LHDCV5BT_EQMID_HQ = 0,
    LHDCV5BT_EQMID_SQ,
    LHDCV5BT_EQMID_MQ,
    LHDCV5BT_EQMID_NUM,     /* terminater */
};

#define MAX_CHANNELS (2)
typedef struct {
  int8_t sampleFormat;
  uint32_t sample_rate;
  uint32_t nframes_per_buffer;
} pcm_info;

typedef struct {
  uint32_t vendorId;
  uint16_t codecId;
  uint8_t config[3];
} __attribute__((__packed__))  tA2DP_LHDC_CIE;

typedef struct {
  uint32_t vendorId;
  uint16_t codecId;
  uint8_t config[5];
} __attribute__((__packed__))  tA2DP_LHDCv5_CIE;

typedef struct {
    bool initialized;
    HANDLE_LHDCV5_BT handle;
    uint32_t sample_rate;
    uint8_t channel_count;
    uint8_t bits_per_sample;
    decoded_data_callback_t decode_callback;
} tA2DP_LHDCV5_DECODER_CB;

static tA2DP_LHDCV5_DECODER_CB a2dp_lhdcv5_decoder_cb;
static const tA2DP_DECODER_INTERFACE lhdcv5_decoder_interface;

bool a2dp_lhdcv5_decoder_init(decoded_data_callback_t decode_callback) {
    LOG_INFO("Initializing LHDC V5 decoder");
    memset(&a2dp_lhdcv5_decoder_cb, 0, sizeof(tA2DP_LHDCV5_DECODER_CB));

    a2dp_lhdcv5_decoder_cb.decode_callback = decode_callback;
    a2dp_lhdcv5_decoder_cb.initialized = true;
    
    LOG_INFO("LHDC V5 decoder initialized");
    return true;
}

void a2dp_lhdcv5_decoder_cleanup() {
    LOG_INFO("Cleaning up LHDC V5 decoder");
    
    if (a2dp_lhdcv5_decoder_cb.initialized) {
        if (a2dp_lhdcv5_decoder_cb.handle != NULL) {
            lhdcv5BT_dec_deinit_decoder(a2dp_lhdcv5_decoder_cb.handle);
            a2dp_lhdcv5_decoder_cb.handle = NULL;
        }
        a2dp_lhdcv5_decoder_cb.initialized = false;
        a2dp_lhdcv5_decoder_cb.decode_callback = NULL;
    }
}

ssize_t a2dp_lhdcv5_decoder_decode_packet_header(BT_HDR* p_buf) {
    // 跳过A2DP头部
    size_t header_len = sizeof(struct media_packet_header) + 
                        A2DP_LHDC_MPL_HDR_LEN;
    p_buf->offset += header_len;
    p_buf->len -= header_len;
    return 0;
}

bool a2dp_lhdcv5_decoder_decode_packet(BT_HDR* p_buf, unsigned char* buf, size_t buf_len) {
    if (!a2dp_lhdcv5_decoder_cb.initialized || p_buf == NULL || buf == NULL) {
        return false;
    }

    // 获取payload数据
    uint8_t* payload = (uint8_t*)(p_buf + 1) + p_buf->offset;
    uint16_t payload_len = p_buf->len;
    
    uint32_t decoded_bytes = buf_len;
    int32_t ret = lhdcv5BT_dec_decode(
        payload,
        payload_len,
        buf,
        &decoded_bytes,
        a2dp_lhdcv5_decoder_cb.bits_per_sample
    );

    if (ret != LHDCV5BT_DEC_API_SUCCEED) {
        LOG_ERROR("%s: decode error. result = %d", __func__, ret);
        return false;
    }
    
    // 调用回调函数传递解码后的数据
    if (a2dp_lhdcv5_decoder_cb.decode_callback) {
        a2dp_lhdcv5_decoder_cb.decode_callback(buf, decoded_bytes);
    }
    
    return true;
}

void a2dp_lhdcv5_decoder_start() {
    LOG_INFO("Starting LHDC V5 decoder");
}

void a2dp_lhdcv5_decoder_suspend() {
    LOG_INFO("Suspending LHDC V5 decoder");
}

void a2dp_lhdcv5_decoder_configure(const uint8_t* p_codec_info) {
    if (!a2dp_lhdcv5_decoder_cb.initialized || p_codec_info == NULL) {
        return;
    }

    tA2DP_LHDCV5_CIE cie;
    tA2D_STATUS status;
    status = A2DP_ParseInfoLhdcv5(&cie, (uint8_t*)p_codec_info, false);
    if (status != A2D_SUCCESS) {
        LOG_ERROR("%s: failed parsing codec info. %d", __func__, status);
        return;
    }

    // 映射LHDC采样率
    switch (cie.sampleRate & A2DP_LHDCV5_SAMPLING_FREQ_MASK) {
        case A2DP_LHDCV5_SAMPLING_FREQ_44100:
            a2dp_lhdcv5_decoder_cb.sample_rate = 44100;
            break;
        case A2DP_LHDCV5_SAMPLING_FREQ_48000:
            a2dp_lhdcv5_decoder_cb.sample_rate = 48000;
            break;
        case A2DP_LHDCV5_SAMPLING_FREQ_96000:
            a2dp_lhdcv5_decoder_cb.sample_rate = 96000;
            break;
        case A2DP_LHDCV5_SAMPLING_FREQ_192000:
            a2dp_lhdcv5_decoder_cb.sample_rate = 192000;
            break;
        default:
            LOG_ERROR("Unsupported sample rate: 0x%02x", cie.sampleRate);
            return;
    }
    LOG_INFO("%s: LDAC Sampling frequency = %lu", __func__, a2dp_lhdcv5_decoder_cb.sample_rate);

    // 映射通道模式
    if (cie.channelMode & A2DP_LHDCV5_CHANNEL_MODE_STEREO) {
        a2dp_lhdcv5_decoder_cb.channel_count = 2;
        LOG_INFO("%s: LDAC Channel mode: Stereo", __func__);
    } else if (cie.channelMode & A2DP_LHDCV5_CHANNEL_MODE_MONO) {
        a2dp_lhdcv5_decoder_cb.channel_count = 1;
        LOG_INFO("%s: LDAC Channel mode: Mono", __func__);
    } else {
        LOG_ERROR("Unsupported channel mode: 0x%02x", cie.channelMode);
        return;
    }

    // 映射位深
    switch (cie.bits_per_sample & A2DP_LHDCV5_BIT_FMT_MASK) {
        case A2DP_LHDCV5_BIT_FMT_32:
            a2dp_lhdcv5_decoder_cb.bits_per_sample = 32;
            break;
        case A2DP_LHDCV5_BIT_FMT_24:
            a2dp_lhdcv5_decoder_cb.bits_per_sample = 24;
            break;
        case A2DP_LHDCV5_BIT_FMT_16:
            a2dp_lhdcv5_decoder_cb.bits_per_sample = 16;
            break;
        default:
            LOG_ERROR("Unsupported bits per sample: 0x%02x", cie.bits_per_sample);
            return;
    }
    LOG_INFO("%s: LDAC Bit depth = %d", __func__, a2dp_lhdcv5_decoder_cb.bits_per_sample);

    // 如果已存在解码器句柄，先释放
    if (a2dp_lhdcv5_decoder_cb.handle != NULL) {
        lhdcv5BT_dec_deinit_decoder(a2dp_lhdcv5_decoder_cb.handle);
        a2dp_lhdcv5_decoder_cb.handle = NULL;
    }

    // 初始化解码器配置
    tLHDCV5_DEC_CONFIG config = {0};
    config.version = VERSION_5;
    config.sample_rate = a2dp_lhdcv5_decoder_cb.sample_rate;
    config.bits_depth = a2dp_lhdcv5_decoder_cb.bits_per_sample;
    config.lossless_enable = (cie.qualityMode & A2DP_LHDCV5_FEATURE_LLESS48K) ? 1 : 0;

    // 初始化解码器
    int32_t ret = lhdcv5BT_dec_init_decoder(&a2dp_lhdcv5_decoder_cb.handle, &config);
    if (ret != LHDCV5BT_DEC_API_SUCCEED || a2dp_lhdcv5_decoder_cb.handle == NULL) {
        APPL_TRACE_ERROR("Failed to initialize LHDC V5 decoder: %d", ret);
        return;
    }

    LOG_INFO("LHDC V5 decoder configured - Sample rate: %d, Channels: %d, Bits: %d",
             a2dp_lhdcv5_decoder_cb.sample_rate, a2dp_lhdcv5_decoder_cb.channel_count, 
             a2dp_lhdcv5_decoder_cb.bits_per_sample);
}

// LHDCV5 decoder interface，已转移到a2dp_vendor_lhdcv5.c
// static const tA2DP_DECODER_INTERFACE lhdcv5_decoder_interface = {
//     a2dp_lhdcv5_decoder_init,
//     a2dp_lhdcv5_decoder_cleanup,
//     NULL,
//     a2dp_lhdcv5_decoder_decode_pocket_header,
//     a2dp_lhdcv5_decoder_decode_packet,
//     a2dp_lhdcv5_decoder_start,
//     a2dp_lhdcv5_decoder_suspend,
//     a2dp_lhdcv5_decoder_configure,
// };

// const tA2DP_DECODER_INTERFACE* A2DP_LHDCV5_DecoderInterface(void) {
//     return &lhdcv5_decoder_interface;
// }

#endif /* LHDCV5_DEC_INCLUDED */