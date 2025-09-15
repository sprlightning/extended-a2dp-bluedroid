/**
 * @file lhdcv5BT_dec.h
 * @brief 适用于蓝牙A2DP的LHDC V5解码器接口
 */
#ifndef LHDCV5BT_DEC_H
#define LHDCV5BT_DEC_H

#include "lhdc_v5_dec.h"
#include "lhdcv5_util_dec.h"
#include <stdint.h>

// 函数返回值定义
#define LHDCBT_DEC_FUNC_SUCCEED        0
#define LHDCBT_DEC_FUNC_FAIL           -1
#define LHDCBT_DEC_FUNC_INPUT_NOT_ENOUGH -2
#define LHDCBT_DEC_FUNC_OUTPUT_NOT_ENOUGH -3

// 版本定义
#define VERSION_5                      5

// 序列更新标志
#define LHDCBT_DEC_NOT_UPD_SEQ_NO      0
#define LHDCBT_DEC_UPD_SEQ_NO          1

/**
 * @brief LHDC V5蓝牙解码器配置结构体
 */
typedef struct {
    uint32_t bits_depth;       // 比特深度
    uint32_t sample_rate;      // 采样率
    uint32_t version;          // 版本号
} tLHDCV5_DEC_CONFIG;

/**
 * @brief 初始化LHDC V5蓝牙解码器
 * @param config 解码器配置
 * @return 0 成功，其他 错误码
 */
int lhdcv5BT_dec_init_decoder(tLHDCV5_DEC_CONFIG *config);

/**
 * @brief 检查缓冲区中的帧数据是否完整
 * @param frameData 帧数据指针
 * @param frameBytes 帧数据长度
 * @param packetBytes 输出的完整数据包长度
 * @return 0 成功，其他 错误码
 */
int lhdcv5BT_dec_check_frame_data_enough(const uint8_t *frameData, uint32_t frameBytes, uint32_t *packetBytes);

/**
 * @brief 解码LHDC V5蓝牙数据包
 * @param frameData 输入帧数据
 * @param frameBytes 输入帧数据长度
 * @param pcmData 输出PCM数据缓冲区
 * @param pcmBytes 输入：缓冲区大小；输出：实际写入大小
 * @param bits_depth 输出PCM的比特深度
 * @return 0 成功，其他 错误码
 */
int lhdcv5BT_dec_decode(const uint8_t *frameData, uint32_t frameBytes, uint8_t* pcmData, 
                       uint32_t* pcmBytes, uint32_t bits_depth);

/**
 * @brief 解初始化LHDC V5蓝牙解码器
 * @return 0 成功
 */
int lhdcv5BT_dec_deinit_decoder(void);

#endif // LHDCV5BT_DEC_H