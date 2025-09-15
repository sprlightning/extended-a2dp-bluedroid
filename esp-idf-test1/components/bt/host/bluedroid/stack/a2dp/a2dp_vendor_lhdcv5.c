/**
 * @file a2dp_vendor_lhdcv5.c
 * @brief A2DP LHDCV5编码器实现
 */
#include "stack/avdt_api.h"
#include <string.h>
#include "stack/a2d_api.h"
#include "stack/a2dp_vendor.h"
#include "stack/a2dp_vendor_lhdcv5.h"
#include "stack/a2dp_vendor_lhdcv5_decoder.h"

#if (defined(LHDCV5_DEC_INCLUDED) && LHDCV5_DEC_INCLUDED == TRUE)

// LHDCV5编解码器能力信息
static const tA2DP_LHDCV5_CIE a2dp_lhdcv5_source_caps = {
    .vendor_id = A2DP_LHDCV5_VENDOR_ID,
    .codec_id = A2DP_LHDCV5_CODEC_ID,
    .sample_rate = A2DP_LHDCV5_SAMPLERATE_44100 | A2DP_LHDCV5_SAMPLERATE_48000 | A2DP_LHDCV5_SAMPLERATE_96000,
    .channel_mode = A2DP_LHDCV5_CHANNEL_MODE_STEREO,
    .bit_depth = A2DP_LHDCV5_BIT_DEPTH_16 | A2DP_LHDCV5_BIT_DEPTH_24,
    .bitrate = A2DP_LHDCV5_BITRATE_300 | A2DP_LHDCV5_BITRATE_400 | 
               A2DP_LHDCV5_BITRATE_500 | A2DP_LHDCV5_BITRATE_600 |
               A2DP_LHDCV5_BITRATE_700 | A2DP_LHDCV5_BITRATE_800 |
               A2DP_LHDCV5_BITRATE_900
};

static const tA2DP_LHDCV5_CIE a2dp_lhdcv5_sink_caps = {
    .vendor_id = A2DP_LHDCV5_VENDOR_ID,
    .codec_id = A2DP_LHDCV5_CODEC_ID,
    .sample_rate = A2DP_LHDCV5_SAMPLERATE_44100 | A2DP_LHDCV5_SAMPLERATE_48000 | A2DP_LHDCV5_SAMPLERATE_96000,
    .channel_mode = A2DP_LHDCV5_CHANNEL_MODE_STEREO,
    .bit_depth = A2DP_LHDCV5_BIT_DEPTH_16 | A2DP_LHDCV5_BIT_DEPTH_24,
    .bitrate = A2DP_LHDCV5_BITRATE_300 | A2DP_LHDCV5_BITRATE_400 | 
               A2DP_LHDCV5_BITRATE_500 | A2DP_LHDCV5_BITRATE_600 |
               A2DP_LHDCV5_BITRATE_700 | A2DP_LHDCV5_BITRATE_800 |
               A2DP_LHDCV5_BITRATE_900
};

// 获取LHDCV5源编码能力
const tA2DP_LHDCV5_CIE* a2dp_get_lhdcv5_source_caps(void) {
    return &a2dp_lhdcv5_source_caps;
}

// 获取LHDCV5 sink编码能力
const tA2DP_LHDCV5_CIE* a2dp_get_lhdcv5_sink_caps(void) {
    return &a2dp_lhdcv5_sink_caps;
}

// 检查LHDCV5编码能力是否匹配
bool a2dp_check_lhdcv5_capability(const tA2DP_LHDCV5_CIE* local, const tA2DP_LHDCV5_CIE* remote) {
    if (!local || !remote) return false;
    
    // 检查厂商ID和编码ID
    if (local->vendor_id != remote->vendor_id || local->codec_id != remote->codec_id) {
        return false;
    }
    
    // 检查采样率是否有交集
    if ((local->sample_rate & remote->sample_rate) == 0) {
        return false;
    }
    
    // 检查声道模式是否有交集
    if ((local->channel_mode & remote->channel_mode) == 0) {
        return false;
    }
    
    // 检查比特深度是否有交集
    if ((local->bit_depth & remote->bit_depth) == 0) {
        return false;
    }
    
    // 检查比特率是否有交集
    if ((local->bitrate & remote->bitrate) == 0) {
        return false;
    }
    
    return true;
}

// 初始化LHDCV5解码器
bool a2dp_lhdcv5_decoder_init(tA2DP_LHDCV5_DECODER_CB* p_cb, const tA2DP_LHDCV5_CIE* caps) {
    if (!p_cb || !caps) return false;
    
    memset(p_cb, 0, sizeof(tA2DP_LHDCV5_DECODER_CB));
    
    // 配置解码器
    tLHDCV5_DEC_CONFIG config;
    config.version = VERSION_5;
    
    // 选择最高支持的采样率
    if (caps->sample_rate & A2DP_LHDCV5_SAMPLERATE_96000) {
        config.sample_rate = 96000;
    } else if (caps->sample_rate & A2DP_LHDCV5_SAMPLERATE_48000) {
        config.sample_rate = 48000;
    } else {
        config.sample_rate = 44100;
    }
    
    // 选择最高支持的比特深度
    if (caps->bit_depth & A2DP_LHDCV5_BIT_DEPTH_24) {
        config.bits_depth = 24;
    } else {
        config.bits_depth = 16;
    }
    
    // 初始化LHDCV5解码器
    if (lhdcv5BT_dec_init_decoder(&config) != LHDCBT_DEC_FUNC_SUCCEED) {
        return false;
    }
    
    p_cb->initialized = true;
    p_cb->sample_rate = config.sample_rate;
    p_cb->bits_depth = config.bits_depth;
    p_cb->channel_mode = A2DP_LHDCV5_CHANNEL_MODE_STEREO;
    
    return true;
}

// 解码LHDCV5数据包
int a2dp_lhdcv5_decoder_decode(tA2DP_LHDCV5_DECODER_CB* p_cb, const uint8_t* p_data, uint32_t len, 
                              uint8_t* p_out, uint32_t out_len, uint32_t* p_written) {
    if (!p_cb || !p_cb->initialized || !p_data || !p_out || !p_written || len == 0 || out_len == 0) {
        return -1;
    }
    
    // 检查输入数据是否足够
    uint32_t packet_bytes;
    int ret = lhdcv5BT_dec_check_frame_data_enough(p_data, len, &packet_bytes);
    if (ret == LHDCBT_DEC_FUNC_INPUT_NOT_ENOUGH) {
        return 0; // 数据不足，需要更多数据
    } else if (ret != LHDCBT_DEC_FUNC_SUCCEED) {
        return -1; // 错误
    }
    
    // 解码数据
    *p_written = out_len;
    ret = lhdcv5BT_dec_decode(p_data, packet_bytes, p_out, p_written, p_cb->bits_depth);
    
    if (ret == LHDCBT_DEC_FUNC_SUCCEED) {
        return packet_bytes; // 返回已处理的字节数
    } else if (ret == LHDCBT_DEC_FUNC_OUTPUT_NOT_ENOUGH) {
        return -2; // 输出缓冲区不足
    } else {
        return -1; // 解码错误
    }
}

// 关闭LHDCV5解码器
void a2dp_lhdcv5_decoder_cleanup(tA2DP_LHDCV5_DECODER_CB* p_cb) {
    if (p_cb && p_cb->initialized) {
        lhdcv5BT_dec_deinit_decoder();
        p_cb->initialized = false;
    }
}

// 下面是适配a2dp_vendor.c的函数
static const tA2DP_DECODER_INTERFACE lhdcv5_decoder_interface = {
    A2DP_InitLhdcv5Decoder,          // decoder_init：需接收 decoded_data_callback_t 参数
    A2DP_CleanupLhdcv5Decoder,       // decoder_cleanup
    NULL,                            // decoder_reset（暂未实现）
    NULL,                            // decode_packet_header（暂未实现）
    A2DP_DecodeLhdcv5Packet,         // decode_packet：参数需匹配 (BT_HDR*, unsigned char*, size_t)
    NULL,                            // decoder_start（暂未实现）
    NULL,                            // decoder_suspend（暂未实现）
    NULL                             // decoder_configure（暂未实现）
};

tA2D_STATUS A2DP_ParseInfoLhdcv5(tA2DP_LHDCV5_CIE* p_ie, const uint8_t* p_codec_info, bool is_capability) {
    if (p_ie == NULL || p_codec_info == NULL) {
        return A2D_INVALID_PARAMS;
    }

    /* Parse codec info - adjust offsets according to actual LHDCV5 spec */
    p_ie->sample_rate = p_codec_info[7] & 0x0F;
    p_ie->channel_mode = (p_codec_info[7] >> 4) & 0x0F;
    p_ie->bit_depth = p_codec_info[8] & 0x0F;
    p_ie->quality_mode = (p_codec_info[8] >> 4) & 0x0F;

    return A2D_SUCCESS;
}

bool A2DP_IsVendorPeerSinkCodecValidLhdcv5(const uint8_t* p_codec_info) {
    if (p_codec_info == NULL) {
        return false;
    }

    /* Basic validation of LHDCV5 codec info */
    uint8_t sample_rate = p_codec_info[7] & 0x0F;
    if (sample_rate > 0x07) {  /* Assume valid rates are 0-7 */
        return false;
    }

    return true;
}

tA2D_STATUS A2DP_IsVendorPeerSourceCodecValidLhdcv5(const uint8_t* p_codec_info) {
    if (!A2DP_IsVendorPeerSinkCodecValidLhdcv5(p_codec_info)) {
        // return A2D_INVALID_CODEC;
        return A2D_BAD_CODEC_TYPE;
    }
    return A2D_SUCCESS;
}

btav_a2dp_codec_index_t A2DP_VendorSinkCodecIndexLhdcv5(const uint8_t* p_codec_info) {
    UNUSED(p_codec_info);
    return BTAV_A2DP_CODEC_INDEX_SINK_LHDCV5;
}

btav_a2dp_codec_index_t A2DP_VendorSourceCodecIndexLhdcv5(const uint8_t* p_codec_info) {
    UNUSED(p_codec_info);
    return BTAV_A2DP_CODEC_INDEX_SOURCE_LHDCV5;
}

bool A2DP_VendorInitCodecConfigLhdcv5(btav_a2dp_codec_index_t codec_index, UINT8* p_result) {
    if (p_result == NULL) {
        return false;
    }

    if (codec_index != BTAV_A2DP_CODEC_INDEX_SOURCE_LHDCV5 && 
        codec_index != BTAV_A2DP_CODEC_INDEX_SINK_LHDCV5) {
        return false;
    }

    /* Initialize default LHDCV5 codec config */
    memset(p_result, 0, AVDT_CODEC_SIZE);
    p_result[0] = A2D_MEDIA_CT_NON_A2DP;  /* Non-A2DP codec type */
    p_result[1] = 0x00;                   /* Sample rate and channel */
    p_result[2] = 0x00;                   /* Bitpool range */

    /* Vendor ID (32-bit) - Savitech */
    p_result[3] = (A2DP_LHDCV5_VENDOR_ID >> 24) & 0xFF;
    p_result[4] = (A2DP_LHDCV5_VENDOR_ID >> 16) & 0xFF;
    p_result[5] = (A2DP_LHDCV5_VENDOR_ID >> 8) & 0xFF;
    p_result[6] = A2DP_LHDCV5_VENDOR_ID & 0xFF;

    /* Codec ID (16-bit) */
    p_result[7] = (A2DP_LHDCV5_CODEC_ID >> 8) & 0xFF;
    p_result[8] = A2DP_LHDCV5_CODEC_ID & 0xFF;

    return true;
}

bool A2DP_VendorBuildCodecConfigLhdcv5(UINT8* p_src_cap, UINT8* p_result) {
    if (p_src_cap == NULL || p_result == NULL) {
        return false;
    }

    /* Copy and adjust codec config from source capabilities */
    memcpy(p_result, p_src_cap, AVDT_CODEC_SIZE);
    return true;
}

const char* A2DP_VendorCodecNameLhdcv5(const uint8_t* p_codec_info) {
    UNUSED(p_codec_info);
    return "LHDC V5";
}

bool A2DP_VendorCodecTypeEqualsLhdcv5(const uint8_t* p_codec_info_a, const uint8_t* p_codec_info_b) {
    if (p_codec_info_a == NULL || p_codec_info_b == NULL) {
        return false;
    }

    /* Compare key codec parameters */
    return (p_codec_info_a[7] == p_codec_info_b[7] &&  /* Sample rate + channel mode */
            p_codec_info_a[8] == p_codec_info_b[8]);   /* Bit depth + quality mode */
}

const tA2DP_DECODER_INTERFACE* A2DP_GetVendorDecoderInterfaceLhdcv5(const uint8_t* p_codec_info) {
    if (p_codec_info == NULL) {
        return NULL;
    }

    /* Validate codec info before returning decoder interface */
    if (A2DP_VendorCodecGetVendorId(p_codec_info) != A2DP_LHDCV5_VENDOR_ID ||
        A2DP_VendorCodecGetCodecId(p_codec_info) != A2DP_LHDCV5_CODEC_ID) {
        return NULL;
    }

    return &lhdcv5_decoder_interface;
}

#else  /* LHDCV5_DEC_INCLUDED */

tA2D_STATUS A2DP_ParseInfoLhdcv5(tA2DP_LHDCV5_CIE* p_ie, const uint8_t* p_codec_info, bool is_capability) {
    UNUSED(p_ie);
    UNUSED(p_codec_info);
    UNUSED(is_capability);
    return A2D_FAIL;
}

bool A2DP_IsVendorPeerSinkCodecValidLhdcv5(const uint8_t* p_codec_info) {
    UNUSED(p_codec_info);
    return false;
}

tA2D_STATUS A2DP_IsVendorPeerSourceCodecValidLhdcv5(const uint8_t* p_codec_info) {
    UNUSED(p_codec_info);
    return A2D_FAIL;
}

btav_a2dp_codec_index_t A2DP_VendorSinkCodecIndexLhdcv5(const uint8_t* p_codec_info) {
    UNUSED(p_codec_info);
    return BTAV_A2DP_CODEC_INDEX_MAX;
}

btav_a2dp_codec_index_t A2DP_VendorSourceCodecIndexLhdcv5(const uint8_t* p_codec_info) {
    UNUSED(p_codec_info);
    return BTAV_A2DP_CODEC_INDEX_MAX;
}

bool A2DP_VendorInitCodecConfigLhdcv5(btav_a2dp_codec_index_t codec_index, UINT8* p_result) {
    UNUSED(codec_index);
    UNUSED(p_result);
    return false;
}

bool A2DP_VendorBuildCodecConfigLhdcv5(UINT8* p_src_cap, UINT8* p_result) {
    UNUSED(p_src_cap);
    UNUSED(p_result);
    return false;
}

const char* A2DP_VendorCodecNameLhdcv5(const uint8_t* p_codec_info) {
    UNUSED(p_codec_info);
    return "LHDC V5 (Not Supported)";
}

bool A2DP_VendorCodecTypeEqualsLhdcv5(const uint8_t* p_codec_info_a, const uint8_t* p_codec_info_b) {
    UNUSED(p_codec_info_a);
    UNUSED(p_codec_info_b);
    return false;
}

const tA2DP_DECODER_INTERFACE* A2DP_GetVendorDecoderInterfaceLhdcv5(const uint8_t* p_codec_info) {
    UNUSED(p_codec_info);
    return NULL;
}

#endif // LHDCV5_INCLUDED == TRUE