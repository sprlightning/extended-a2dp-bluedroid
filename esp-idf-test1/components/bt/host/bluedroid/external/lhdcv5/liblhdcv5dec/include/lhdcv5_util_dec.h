/**
 * @file lhdcv5_util_dec.h
 * @brief LHDC V5解码工具函数
 */
#ifndef LHDCV5_UTIL_DEC_H
#define LHDCV5_UTIL_DEC_H

#include "lhdc_v5_dec.h"
#include <stdint.h>

// CRC计算多项式
#define LHDCV5_CRC_POLY 0x1021

/**
 * @brief 计算LHDC帧的CRC校验值
 * @param data 数据指针
 * @param length 数据长度
 * @return CRC校验值
 */
uint16_t lhdcv5_crc16(const uint8_t *data, uint32_t length);

/**
 * @brief 检查LHDC帧的CRC校验
 * @param frame_data 帧数据指针
 * @param frame_len 帧数据长度
 * @return 1 校验通过，0 校验失败
 */
int lhdcv5_check_crc(const uint8_t *frame_data, uint32_t frame_len);

/**
 * @brief 转换LHDC采样率枚举值为实际数值
 * @param rate_code 采样率编码
 * @return 采样率数值，0表示无效
 */
uint32_t lhdcv5_rate_code_to_value(uint8_t rate_code);

/**
 * @brief 转换LHDC比特率枚举值为实际数值
 * @param bitrate_code 比特率编码
 * @return 比特率数值(bps)，0表示无效
 */
uint32_t lhdcv5_bitrate_code_to_value(uint8_t bitrate_code);

/**
 * @brief 打印LHDC帧调试信息
 * @param frame_info 帧信息结构体
 */
void lhdcv5_print_frame_info(const lhdcv5_frame_info_t *frame_info);

/**
 * @brief 转换PCM格式（24位转16位）
 * @param in 输入24位PCM数据
 * @param out 输出16位PCM数据
 * @param samples 样本数
 * @param channels 声道数
 */
void lhdcv5_convert_24_to_16(const uint8_t *in, uint8_t *out, uint32_t samples, uint32_t channels);

#endif // LHDCV5_UTIL_DEC_H