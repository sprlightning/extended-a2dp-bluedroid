#include <string.h>
#include <stdint.h>
#include "stack/a2dp_vendor_lhdcv5_decoder.h"
#include "stack/a2dp_vendor_lhdcv5.h"
#include "osi/osi.h"
#include "common/bt_target.h"
#include "common/bt_trace.h"
#include "esp_log.h"

#define LOG_TAG "A2DP_LHDCV5"

#if (defined(LHDCV5_DEC_INCLUDED) && LHDCV5_DEC_INCLUDED == TRUE)

static tA2DP_LHDCV5_DECODER lhdcv5_decoder;
static const tA2DP_DECODER_INTERFACE lhdcv5_decoder_interface;

static bool a2dp_lhdcv5_decoder_init(decoded_data_callback_t decode_callback) {
    ESP_LOGI(LOG_TAG, "Initializing LHDC V5 decoder");
    memset(&lhdcv5_decoder, 0, sizeof(tA2DP_LHDCV5_DECODER));

    lhdcv5_decoder.decode_callback = decode_callback;
    lhdcv5_decoder.initialized = true;
    
    ESP_LOGI(LOG_TAG, "LHDC V5 decoder initialized");
    return true;
}

static void a2dp_lhdcv5_decoder_cleanup(void) {
    ESP_LOGI(LOG_TAG, "Cleaning up LHDC V5 decoder");
    
    if (lhdcv5_decoder.initialized) {
        if (lhdcv5_decoder.handle != NULL) {
            lhdcv5BT_dec_deinit_decoder(lhdcv5_decoder.handle);
            lhdcv5_decoder.handle = NULL;
        }
        lhdcv5_decoder.initialized = false;
        lhdcv5_decoder.decode_callback = NULL;
    }
}

static void a2dp_lhdcv5_decoder_start(void) {
    ESP_LOGI(LOG_TAG, "Starting LHDC V5 decoder");
}

static void a2dp_lhdcv5_decoder_suspend(void) {
    ESP_LOGI(LOG_TAG, "Suspending LHDC V5 decoder");
}

static ssize_t a2dp_lhdcv5_decoder_decode_header(BT_HDR* p_buf) {
    if (!lhdcv5_decoder.initialized || p_buf == NULL) {
        return -1;
    }

    // 跳过A2DP头部
    size_t header_len = sizeof(struct media_packet_header) + A2DP_LHDC_MPL_HDR_LEN;
    p_buf->offset += header_len;
    p_buf->len -= header_len;
    return 0;
}

static bool a2dp_lhdcv5_decoder_decode_packet(BT_HDR* p_buf, unsigned char* buf, size_t buf_len) {
    if (!lhdcv5_decoder.initialized || p_buf == NULL || buf == NULL) {
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
        lhdcv5_decoder.bits_per_sample
    );

    if (ret != LHDCV5BT_DEC_API_SUCCEED) {
        ESP_LOGE(LOG_TAG, "Decode failed: %d", ret);
        return false;
    }
    
    // 调用回调函数传递解码后的数据
    if (lhdcv5_decoder.decode_callback) {
        lhdcv5_decoder.decode_callback(buf, decoded_bytes);
    }
    
    return true;
}

static void a2dp_lhdcv5_decoder_configure(const uint8_t* p_codec_info) {
    if (!lhdcv5_decoder.initialized || p_codec_info == NULL) {
        return;
    }

    tA2DP_LHDCV5_CIE ie;
    if (A2DP_ParseInfoLhdcv5(&ie, p_codec_info, false) != A2D_SUCCESS) {
        ESP_LOGE(LOG_TAG, "Failed to parse codec info");
        return;
    }

    // 映射LHDC采样率
    switch (ie.sample_rate & A2DP_LHDCV5_SAMPLING_FREQ_MASK) {
        case A2DP_LHDCV5_SAMPLING_FREQ_44100:
            lhdcv5_decoder.sample_rate = 44100;
            break;
        case A2DP_LHDCV5_SAMPLING_FREQ_48000:
            lhdcv5_decoder.sample_rate = 48000;
            break;
        case A2DP_LHDCV5_SAMPLING_FREQ_96000:
            lhdcv5_decoder.sample_rate = 96000;
            break;
        case A2DP_LHDCV5_SAMPLING_FREQ_192000:
            lhdcv5_decoder.sample_rate = 192000;
            break;
        default:
            ESP_LOGE(LOG_TAG, "Unsupported sample rate: 0x%02x", ie.sample_rate);
            return;
    }

    // 映射通道模式
    if (ie.channel_mode & A2DP_LHDCV5_CHANNEL_MODE_STEREO) {
        lhdcv5_decoder.channel_count = 2;
    } else if (ie.channel_mode & A2DP_LHDCV5_CHANNEL_MODE_MONO) {
        lhdcv5_decoder.channel_count = 1;
    } else {
        ESP_LOGE(LOG_TAG, "Unsupported channel mode: 0x%02x", ie.channel_mode);
        return;
    }

    // 映射位深
    switch (ie.bits_per_sample & A2DP_LHDCV5_BIT_FMT_MASK) {
        case A2DP_LHDCV5_BIT_FMT_32:
            lhdcv5_decoder.bits_per_sample = 32;
            break;
        case A2DP_LHDCV5_BIT_FMT_24:
            lhdcv5_decoder.bits_per_sample = 24;
            break;
        case A2DP_LHDCV5_BIT_FMT_16:
            lhdcv5_decoder.bits_per_sample = 16;
            break;
        default:
            ESP_LOGE(LOG_TAG, "Unsupported bits per sample: 0x%02x", ie.bits_per_sample);
            return;
    }

    // 如果已存在解码器句柄，先释放
    if (lhdcv5_decoder.handle != NULL) {
        lhdcv5BT_dec_deinit_decoder(lhdcv5_decoder.handle);
        lhdcv5_decoder.handle = NULL;
    }

    // 初始化解码器配置
    tLHDCV5_DEC_CONFIG config = {0};
    config.version = VERSION_5;
    config.sample_rate = lhdcv5_decoder.sample_rate;
    config.bits_depth = lhdcv5_decoder.bits_per_sample;
    config.lossless_enable = (ie.quality_mode & A2DP_LHDCV5_FEATURE_LLESS48K) ? 1 : 0;

    // 初始化解码器
    int32_t ret = lhdcv5BT_dec_init_decoder(&lhdcv5_decoder.handle, &config);
    if (ret != LHDCV5BT_DEC_API_SUCCEED || lhdcv5_decoder.handle == NULL) {
        ESP_LOGE(LOG_TAG, "Failed to initialize LHDC V5 decoder: %d", ret);
        return;
    }

    ESP_LOGI(LOG_TAG, "LHDC V5 decoder configured - Sample rate: %d, Channels: %d, Bits: %d",
             lhdcv5_decoder.sample_rate, lhdcv5_decoder.channel_count, 
             lhdcv5_decoder.bits_per_sample);
}

static const tA2DP_DECODER_INTERFACE lhdcv5_decoder_interface = {
    a2dp_lhdcv5_decoder_init,
    a2dp_lhdcv5_decoder_cleanup,
    a2dp_lhdcv5_decoder_start,
    a2dp_lhdcv5_decoder_suspend,
    a2dp_lhdcv5_decoder_configure,
    a2dp_lhdcv5_decoder_decode_header,
    a2dp_lhdcv5_decoder_decode_packet
};

const tA2DP_DECODER_INTERFACE* A2DP_LHDCV5_DecoderInterface(void) {
    return &lhdcv5_decoder_interface;
}

#endif /* LHDCV5_DEC_INCLUDED */