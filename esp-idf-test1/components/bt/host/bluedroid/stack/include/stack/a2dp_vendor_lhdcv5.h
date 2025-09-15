/**
 * @file a2dp_vendor_lhdcv5.h
 * @brief A2DP LHDCV5编码器头文件
 */
#ifndef A2DP_VENDOR_LHDCV5_H
#define A2DP_VENDOR_LHDCV5_H

#include "lhdcv5BT_dec.h"
#include "stack/bt_types.h"
#include "a2d_api.h"
#include "a2dp_codec_api.h"
#include "a2dp_vendor_lhdcv5_constants.h"
#include "a2dp_shim.h"
#include "avdt_api.h"
#include "bt_av.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief LHDCV5编码能力信息结构体（LHDCV5 Codec Information Element structure）
 */
typedef struct {
    uint16_t vendor_id;        // 厂商ID
    uint16_t codec_id;         // 编码ID
    uint8_t sample_rate;       // 支持的采样率掩码
    uint8_t channel_mode;      // 支持的声道模式掩码
    uint8_t bit_depth;         // 支持的比特深度掩码
    uint8_t bitrate;           // 支持的比特率掩码
    uint8_t quality_mode;
    uint8_t extra_info[4];
} tA2DP_LHDCV5_CIE;

/**
 * @brief LHDCV5解码器回调结构体
 */
typedef struct {
    bool initialized;          // 初始化标志
    uint32_t sample_rate;      // 采样率
    uint32_t bits_depth;       // 比特深度
    uint8_t channel_mode;      // 声道模式
} tA2DP_LHDCV5_DECODER_CB;

/**
 * @brief 获取LHDCV5源编码能力
 * @return 编码能力结构体指针
 */
const tA2DP_LHDCV5_CIE* a2dp_get_lhdcv5_source_caps(void);

/**
 * @brief 获取LHDCV5 sink编码能力
 * @return 编码能力结构体指针
 */
const tA2DP_LHDCV5_CIE* a2dp_get_lhdcv5_sink_caps(void);

/**
 * @brief 检查LHDCV5编码能力是否匹配
 * @param local 本地编码能力
 * @param remote 远程编码能力
 * @return true 匹配，false 不匹配
 */
bool a2dp_check_lhdcv5_capability(const tA2DP_LHDCV5_CIE* local, const tA2DP_LHDCV5_CIE* remote);

/**
 * @brief 初始化LHDCV5解码器
 * @param p_cb 解码器回调结构体
 * @param caps 编码能力
 * @return true 成功，false 失败
 */
bool a2dp_lhdcv5_decoder_init(tA2DP_LHDCV5_DECODER_CB* p_cb, const tA2DP_LHDCV5_CIE* caps);

/**
 * @brief 解码LHDCV5数据包
 * @param p_cb 解码器回调结构体
 * @param p_data 输入数据
 * @param len 输入数据长度
 * @param p_out 输出缓冲区
 * @param out_len 输出缓冲区长度
 * @param p_written 实际写入的字节数
 * @return 处理的输入字节数，负数表示错误
 */
int a2dp_lhdcv5_decoder_decode(tA2DP_LHDCV5_DECODER_CB* p_cb, const uint8_t* p_data, uint32_t len, 
                              uint8_t* p_out, uint32_t out_len, uint32_t* p_written);

/**
 * @brief 关闭LHDCV5解码器
 * @param p_cb 解码器回调结构体
 */
void a2dp_lhdcv5_decoder_cleanup(tA2DP_LHDCV5_DECODER_CB* p_cb);

// 适配a2dp_vendor.c
tA2D_STATUS A2DP_ParseInfoLhdcv5(tA2DP_LHDCV5_CIE* p_ie, const uint8_t* p_codec_info, bool is_capability);
bool A2DP_IsVendorPeerSinkCodecValidLhdcv5(const uint8_t* p_codec_info);
tA2D_STATUS A2DP_IsVendorPeerSourceCodecValidLhdcv5(const uint8_t* p_codec_info);
btav_a2dp_codec_index_t A2DP_VendorSinkCodecIndexLhdcv5(const uint8_t* p_codec_info);
btav_a2dp_codec_index_t A2DP_VendorSourceCodecIndexLhdcv5(const uint8_t* p_codec_info);
bool A2DP_VendorInitCodecConfigLhdcv5(btav_a2dp_codec_index_t codec_index, UINT8* p_result);
bool A2DP_VendorBuildCodecConfigLhdcv5(UINT8* p_src_cap, UINT8* p_result);
const char* A2DP_VendorCodecNameLhdcv5(const uint8_t* p_codec_info);
bool A2DP_VendorCodecTypeEqualsLhdcv5(const uint8_t* p_codec_info_a, const uint8_t* p_codec_info_b);
const tA2DP_DECODER_INTERFACE* A2DP_GetVendorDecoderInterfaceLhdcv5(const uint8_t* p_codec_info);

#if defined(__cplusplus)
}
#endif

#endif // A2DP_VENDOR_LHDCV5_H