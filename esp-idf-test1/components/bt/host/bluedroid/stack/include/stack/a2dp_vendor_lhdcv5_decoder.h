/**
 * @file a2dp_vendor_lhdcv5_decoder.h
 * @brief A2DP LHDCV5解码器头文件
 */
#ifndef A2DP_VENDOR_LHDCV5_DECODER_H
#define A2DP_VENDOR_LHDCV5_DECODER_H

#include "a2dp_vendor_lhdcv5.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief 初始化LHDCV5解码器
 * @return true 成功，false 失败
 */
bool A2DP_InitLhdcv5Decoder(void);

/**
 * @brief 解码A2DP LHDCV5数据包
 * @param p_buf 输入缓冲区
 * @param p_out_buf 输出缓冲区
 * @param out_len 输出缓冲区长度
 * @param p_written 实际写入的字节数
 * @return 处理的输入字节数
 */
uint16_t A2DP_DecodeLhdcv5Packet(BT_HDR* p_buf, uint8_t* p_out_buf, uint16_t out_len, uint16_t* p_written);

/**
 * @brief 清理LHDCV5解码器资源
 */
void A2DP_CleanupLhdcv5Decoder(void);

/**
 * @brief 获取LHDCV5解码器采样率
 * @return 采样率
 */
uint32_t A2DP_GetLhdcv5SampleRate(void);

/**
 * @brief 获取LHDCV5解码器声道数
 * @return 声道数
 */
uint8_t A2DP_GetLhdcv5ChannelCount(void);

/**
 * @brief 获取LHDCV5解码器比特深度
 * @return 比特深度
 */
uint8_t A2DP_GetLhdcv5BitDepth(void);

#if defined(__cplusplus)
}
#endif

#endif // A2DP_VENDOR_LHDCV5_DECODER_H