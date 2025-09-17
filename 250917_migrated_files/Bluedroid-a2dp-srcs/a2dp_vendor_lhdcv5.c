#include <string.h>
#include "stack/a2dp_vendor_lhdcv5.h"
#include "stack/a2dp_vendor_lhdcv5_decoder.h"
#include "osi/osi.h"
#include "common/bt_target.h"
#include "common/bt_trace.h"

#if (defined(LHDCV5_DEC_INCLUDED) && LHDCV5_DEC_INCLUDED == TRUE)

// 拆分LHDC Vendor ID为4字节（大端序，符合蓝牙协议规范）
#define A2DP_LHDC_VENDOR_ID_BYTE0 ((A2DP_LHDC_VENDOR_ID >> 24) & 0xFF)
#define A2DP_LHDC_VENDOR_ID_BYTE1 ((A2DP_LHDC_VENDOR_ID >> 16) & 0xFF)
#define A2DP_LHDC_VENDOR_ID_BYTE2 ((A2DP_LHDC_VENDOR_ID >> 8) & 0xFF)
#define A2DP_LHDC_VENDOR_ID_BYTE3 (A2DP_LHDC_VENDOR_ID & 0xFF)

const tA2DP_LHDCV5_CIE a2dp_lhdcv5_source_caps = {
    {A2DP_LHDC_VENDOR_ID_BYTE1, A2DP_LHDC_VENDOR_ID_BYTE2, A2DP_LHDC_VENDOR_ID_BYTE3}, 
    A2DP_LHDCV5_CODEC_ID,
    A2DP_LHDCV5_SAMPLING_FREQ_44100 | A2DP_LHDCV5_SAMPLING_FREQ_48000 | 
    A2DP_LHDCV5_SAMPLING_FREQ_96000 |A2DP_LHDCV5_SAMPLING_FREQ_192000,
    A2DP_LHDCV5_CHANNEL_MODE_MONO | A2DP_LHDCV5_CHANNEL_MODE_STEREO,
    A2DP_LHDCV5_BIT_FMT_16 | A2DP_LHDCV5_BIT_FMT_24 | A2DP_LHDCV5_BIT_FMT_32,
    A2DP_LHDC_QUALITY_ABR | A2DP_LHDC_QUALITY_HIGH1 | A2DP_LHDC_QUALITY_HIGH1 | 
    A2DP_LHDC_QUALITY_MID | A2DP_LHDC_QUALITY_LOW | A2DP_LHDC_QUALITY_LOW4 | 
    A2DP_LHDC_QUALITY_LOW3 | A2DP_LHDC_QUALITY_LOW2 | A2DP_LHDC_QUALITY_LOW1 | 
    A2DP_LHDC_QUALITY_LOW0,
    0x00
};

const tA2DP_LHDCV5_CIE a2dp_lhdcv5_sink_caps = {
    {A2DP_LHDC_VENDOR_ID_BYTE1, A2DP_LHDC_VENDOR_ID_BYTE2, A2DP_LHDC_VENDOR_ID_BYTE3}, 
    A2DP_LHDCV5_CODEC_ID,
    A2DP_LHDCV5_SAMPLING_FREQ_44100 | A2DP_LHDCV5_SAMPLING_FREQ_48000 | 
    A2DP_LHDCV5_SAMPLING_FREQ_96000 | A2DP_LHDCV5_SAMPLING_FREQ_192000,
    A2DP_LHDCV5_CHANNEL_MODE_MONO | A2DP_LHDCV5_CHANNEL_MODE_STEREO,
    A2DP_LHDCV5_BIT_FMT_16 | A2DP_LHDCV5_BIT_FMT_24 | A2DP_LHDCV5_BIT_FMT_32,
    A2DP_LHDC_QUALITY_ABR | A2DP_LHDC_QUALITY_HIGH1 | A2DP_LHDC_QUALITY_HIGH1 | 
    A2DP_LHDC_QUALITY_MID | A2DP_LHDC_QUALITY_LOW | A2DP_LHDC_QUALITY_LOW4 | 
    A2DP_LHDC_QUALITY_LOW3 | A2DP_LHDC_QUALITY_LOW2 | A2DP_LHDC_QUALITY_LOW1 | 
    A2DP_LHDC_QUALITY_LOW0,
    0x00
};

tA2D_STATUS A2DP_ParseInfoLhdcv5(tA2DP_LHDCV5_CIE* p_ie, const uint8_t* p_codec_info, bool is_capability) {
    if (p_ie == NULL || p_codec_info == NULL) {
        return A2D_INVALID_PARAMS;
    }

    memset(p_ie, 0, sizeof(tA2DP_LHDCV5_CIE));
    
    // 解析4字节Vendor ID（P0-P3）
    p_ie->vendor_id[0] = p_codec_info[0];
    p_ie->vendor_id[1] = p_codec_info[1];
    p_ie->vendor_id[2] = p_codec_info[2];
    p_ie->vendor_id[3] = p_codec_info[3];
    
    // 解析2字节Codec ID（P4-P5，组合为16位）
    p_ie->codec_id = (p_codec_info[4] << 8) | p_codec_info[5];
    
    if (is_capability) {
        // 能力字段解析
        p_ie->sample_rate = p_codec_info[6];  // 采样率（P6）
        p_ie->channel_mode = p_codec_info[8]; // 通道模式（根据LHDCV5格式定义）
        p_ie->bits_per_sample = p_codec_info[7]; // 位深（P7[2:0]）
        p_ie->quality_mode = (p_codec_info[7] & 0xF0) >> 4; // 质量模式（P7高4位）
    } else {
        // 配置字段解析（带掩码过滤）
        p_ie->sample_rate = p_codec_info[6] & A2DP_LHDCV5_SAMPLING_FREQ_MASK;
        p_ie->channel_mode = p_codec_info[8] & A2DP_LHDCV5_CHANNEL_MODE_MASK;
        p_ie->bits_per_sample = p_codec_info[7] & A2DP_LHDCV5_BIT_FMT_MASK;
        p_ie->quality_mode = (p_codec_info[7] & A2DP_LHDC_QUALITY_MASK) >> 4;
    }

    return A2D_SUCCESS;
}

bool A2DP_IsVendorPeerSinkCodecValidLhdcv5(const uint8_t* p_codec_info) {
    if (p_codec_info == NULL) {
        return false;
    }

    tA2DP_LHDCV5_CIE ie;
    if (A2DP_ParseInfoLhdcv5(&ie, p_codec_info, false) != A2D_SUCCESS) {
        return false;
    }

     // 验证Vendor ID（4字节）和Codec ID（2字节）
    if (ie.vendor_id[0] != A2DP_LHDC_VENDOR_ID_BYTE0 ||
        ie.vendor_id[1] != A2DP_LHDC_VENDOR_ID_BYTE1 ||
        ie.vendor_id[2] != A2DP_LHDC_VENDOR_ID_BYTE2 ||
        ie.vendor_id[3] != A2DP_LHDC_VENDOR_ID_BYTE3 ||
        ie.codec_id != A2DP_LHDCV5_CODEC_ID) {
        return false;
    }

    // 验证能力匹配
    if ((ie.sample_rate & a2dp_lhdcv5_source_caps.sample_rate) == 0 ||
        (ie.channel_mode & a2dp_lhdcv5_source_caps.channel_mode) == 0 ||
        (ie.bits_per_sample & a2dp_lhdcv5_source_caps.bits_per_sample) == 0 ||
        (ie.quality_mode & a2dp_lhdcv5_source_caps.quality_mode) == 0) {
        return false;
    }

    return true;
}

tA2D_STATUS A2DP_IsVendorPeerSourceCodecValidLhdcv5(const uint8_t* p_codec_info) {
    if (p_codec_info == NULL) {
        return A2D_INVALID_PARAMS;
    }

    tA2DP_LHDCV5_CIE ie;
    if (A2DP_ParseInfoLhdcv5(&ie, p_codec_info, false) != A2D_SUCCESS) {
        return A2D_INVALID_PARAMS;
    }

    // 验证Vendor ID（4字节）和Codec ID（2字节）
    if (ie.vendor_id[0] != A2DP_LHDC_VENDOR_ID_BYTE0 ||
        ie.vendor_id[1] != A2DP_LHDC_VENDOR_ID_BYTE1 ||
        ie.vendor_id[2] != A2DP_LHDC_VENDOR_ID_BYTE2 ||
        ie.vendor_id[3] != A2DP_LHDC_VENDOR_ID_BYTE3 ||
        ie.codec_id != A2DP_LHDCV5_CODEC_ID) {
        return A2D_INVALID_PARAMS;
    }

    // 验证能力匹配
    if ((ie.sample_rate & a2dp_lhdcv5_sink_caps.sample_rate) == 0 ||
        (ie.channel_mode & a2dp_lhdcv5_sink_caps.channel_mode) == 0 ||
        (ie.bits_per_sample & a2dp_lhdcv5_sink_caps.bits_per_sample) == 0 ||
        (ie.quality_mode & a2dp_lhdcv5_sink_caps.quality_mode) == 0) {
        return A2D_INVALID_PARAMS;
    }

    return A2D_SUCCESS;
}

btav_a2dp_codec_index_t A2DP_VendorSinkCodecIndexLhdcv5(const uint8_t* p_codec_info) {
    return BTAV_A2DP_CODEC_INDEX_SINK_LHDCV5;
}

btav_a2dp_codec_index_t A2DP_VendorSourceCodecIndexLhdcv5(const uint8_t* p_codec_info) {
    return BTAV_A2DP_CODEC_INDEX_SOURCE_LHDCV5;
}

bool A2DP_VendorInitCodecConfigLhdcv5(btav_a2dp_codec_index_t codec_index, UINT8* p_result) {
    if (p_result == NULL) {
        return false;
    }

    const tA2DP_LHDCV5_CIE* p_caps = (codec_index == BTAV_A2DP_CODEC_INDEX_SOURCE_LHDCV5) ? 
                                    &a2dp_lhdcv5_source_caps : &a2dp_lhdcv5_sink_caps;

    // 填充Vendor ID（P0-P3）
    p_result[0] = p_caps->vendor_id[0];
    p_result[1] = p_caps->vendor_id[1];
    p_result[2] = p_caps->vendor_id[2];
    p_result[3] = p_caps->vendor_id[3];
    // 填充Codec ID（P4-P5）
    p_result[4] = (p_caps->codec_id >> 8) & 0xFF;  // 高8位
    p_result[5] = p_caps->codec_id & 0xFF;         // 低8位
    // 填充采样率（P6）
    p_result[6] = p_caps->sample_rate;
    // 填充位深和质量模式（P7）
    p_result[7] = (p_caps->bits_per_sample) | ((p_caps->quality_mode << 4) & 0xF0);
    // 填充通道模式（P8）
    p_result[8] = p_caps->channel_mode;

    return true;
}

bool A2DP_VendorBuildCodecConfigLhdcv5(UINT8* p_src_cap, UINT8* p_result) {
    if (p_src_cap == NULL || p_result == NULL) {
        return false;
    }

    tA2DP_LHDCV5_CIE src_ie, sink_ie;
    if (A2DP_ParseInfoLhdcv5(&src_ie, p_src_cap, true) != A2D_SUCCESS) {
        return false;
    }

    memcpy(&sink_ie, &a2dp_lhdcv5_sink_caps, sizeof(tA2DP_LHDCV5_CIE));

    // 填充Vendor ID和Codec ID
    p_result[0] = sink_ie.vendor_id[0];
    p_result[1] = sink_ie.vendor_id[1];
    p_result[2] = sink_ie.vendor_id[2];
    p_result[3] = sink_ie.vendor_id[3];
    p_result[4] = (sink_ie.codec_id >> 8) & 0xFF;
    p_result[5] = sink_ie.codec_id & 0xFF;
    
    // 计算协商后的能力
    p_result[6] = src_ie.sample_rate & sink_ie.sample_rate;  // 采样率（P6）
    p_result[7] = (src_ie.bits_per_sample & sink_ie.bits_per_sample) | 
                 (((src_ie.quality_mode & sink_ie.quality_mode) << 4) & 0xF0);  // 位深和质量模式（P7）
    p_result[8] = src_ie.channel_mode & sink_ie.channel_mode;  // 通道模式（P8）
    
    return true;
}

const char* A2DP_VendorCodecNameLhdcv5(const uint8_t* p_codec_info) {
    return "LHDC V5";
}

bool A2DP_VendorCodecTypeEqualsLhdcv5(const uint8_t* p_codec_info_a, const uint8_t* p_codec_info_b) {
    if (p_codec_info_a == NULL || p_codec_info_b == NULL) {
        return false;
    }

    return (memcmp(p_codec_info_a, p_codec_info_b, A2DP_LHDCV5_CODEC_LEN) == 0);
}

const tA2DP_DECODER_INTERFACE* A2DP_GetVendorDecoderInterfaceLhdcv5(const uint8_t* p_codec_info) {
    return A2DP_LHDCV5_DecoderInterface();
}

#endif /* LHDCV5_DEC_INCLUDED */