/**
 * @file a2dp_vendor_lhdcv5_decoder.c
 * @brief A2DP LHDCV5解码器实现
 */
#include "stack/a2dp_vendor_lhdcv5_decoder.h"
#include "stack/a2dp_vendor_lhdcv5_constants.h"
#include "stack/a2dp_vendor.h"
#include "stack/avdt_api.h"
#include <string.h>

#if (defined(LHDCV5_DEC_INCLUDED) && LHDCV5_DEC_INCLUDED == TRUE)

// LHDCV5解码器回调实例
static tA2DP_LHDCV5_DECODER_CB lhdcv5_decoder_cb;

// 初始化LHDCV5解码器
bool A2DP_InitLhdcv5Decoder(void) {
    // 获取LHDCV5编码能力
    const tA2DP_LHDCV5_CIE* caps = a2dp_get_lhdcv5_sink_caps();
    if (!caps) return false;
    
    // 初始化解码器
    return a2dp_lhdcv5_decoder_init(&lhdcv5_decoder_cb, caps);
}

// 解码A2DP LHDCV5数据包
uint16_t A2DP_DecodeLhdcv5Packet(BT_HDR* p_buf, uint8_t* p_out_buf, uint16_t out_len, uint16_t* p_written) {
    if (!p_buf || !p_out_buf || !p_written || !lhdcv5_decoder_cb.initialized) {
        return 0;
    }
    
    // 指向A2DP payload数据
    uint8_t* p_data = (uint8_t*)(p_buf + 1) + p_buf->offset;
    uint16_t data_len = p_buf->len;
    
    // 解码数据
    int ret = a2dp_lhdcv5_decoder_decode(&lhdcv5_decoder_cb, p_data, data_len, 
                                        p_out_buf, out_len, (uint32_t*)p_written);
    
    if (ret > 0) {
        return (uint16_t)ret; // 返回已处理的字节数
    }
    
    return 0;
}

// 清理LHDCV5解码器资源
void A2DP_CleanupLhdcv5Decoder(void) {
    a2dp_lhdcv5_decoder_cleanup(&lhdcv5_decoder_cb);
}

// 获取LHDCV5解码器采样率
uint32_t A2DP_GetLhdcv5SampleRate(void) {
    if (lhdcv5_decoder_cb.initialized) {
        return lhdcv5_decoder_cb.sample_rate;
    }
    return 0;
}

// 获取LHDCV5解码器声道数
uint8_t A2DP_GetLhdcv5ChannelCount(void) {
    if (lhdcv5_decoder_cb.initialized) {
        return (lhdcv5_decoder_cb.channel_mode == A2DP_LHDCV5_CHANNEL_MODE_STEREO) ? 2 : 1;
    }
    return 0;
}

// 获取LHDCV5解码器比特深度
uint8_t A2DP_GetLhdcv5BitDepth(void) {
    if (lhdcv5_decoder_cb.initialized) {
        return (uint8_t)lhdcv5_decoder_cb.bits_depth;
    }
    return 0;
}

#endif // LHDCV5_INCLUDED == TRUE