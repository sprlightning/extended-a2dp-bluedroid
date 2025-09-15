/**
 * @file lhdc_v5_dec.h
 * @brief LHDC V5解码器核心接口定义
 */
#ifndef LHDC_V5_DEC_H
#define LHDC_V5_DEC_H

#include "lhdc_v5_dec_workspace.h"
#include <stdint.h>

// LHDC V5版本定义
#define LHDCV5_VERSION_MAJOR 5
#define LHDCV5_VERSION_MINOR 0

// 采样率定义
#define LHDCV5_SAMPLERATE_44100 44100
#define LHDCV5_SAMPLERATE_48000 48000
#define LHDCV5_SAMPLERATE_96000 96000

// 比特深度定义
#define LHDCV5_BITDEPTH_16 16
#define LHDCV5_BITDEPTH_24 24

// 声道模式
#define LHDCV5_CHANNEL_MONO 1
#define LHDCV5_CHANNEL_STEREO 2

// 解码错误码
#define LHDCV5_DEC_SUCCESS 0
#define LHDCV5_DEC_ERR_PARAM 1
#define LHDCV5_DEC_ERR_INIT 2
#define LHDCV5_DEC_ERR_FRAME 3
#define LHDCV5_DEC_ERR_MEM 4
#define LHDCV5_DEC_ERR_BITRATE 5

/**
 * @brief LHDC V5解码器配置结构体
 */
typedef struct {
    uint32_t sample_rate;      // 采样率
    uint32_t bit_depth;        // 比特深度
    uint32_t channels;         // 声道数
    uint32_t max_bitrate;      // 最大比特率
    uint8_t use_hw_accel;      // 是否使用硬件加速
} lhdcv5_dec_config_t;

/**
 * @brief LHDC V5帧信息结构体
 */
typedef struct {
    uint32_t frame_size;       // 帧大小(字节)
    uint32_t sample_count;     // 样本数
    uint32_t bitrate;          // 当前比特率
    uint8_t frame_type;        // 帧类型
    uint8_t crc_valid;         // CRC校验是否有效
} lhdcv5_frame_info_t;

/**
 * @brief LHDC V5解码器上下文结构体
 */
typedef struct {
    lhdcv5_dec_config_t config;            // 解码器配置
    lhdcv5_dec_workspace_t workspace;      // 解码器工作区
    uint8_t is_running;                    // 解码器运行状态
    uint32_t total_frames_decoded;         // 已解码总帧数
    uint32_t total_samples_decoded;        // 已解码总样本数
} lhdcv5_decoder_t;

/**
 * @brief 初始化LHDC V5解码器
 * @param decoder 解码器实例指针
 * @param config 解码器配置
 * @return 0 成功，其他 错误码
 */
int lhdcv5_dec_init(lhdcv5_decoder_t *decoder, const lhdcv5_dec_config_t *config);

/**
 * @brief 解初始化LHDC V5解码器
 * @param decoder 解码器实例指针
 */
void lhdcv5_dec_deinit(lhdcv5_decoder_t *decoder);

/**
 * @brief 获取LHDC V5帧信息
 * @param decoder 解码器实例指针
 * @param frame_data 帧数据指针
 * @param frame_len 帧数据长度
 * @param info 输出的帧信息
 * @return 0 成功，其他 错误码
 */
int lhdcv5_dec_get_frame_info(lhdcv5_decoder_t *decoder, const uint8_t *frame_data, 
                             uint32_t frame_len, lhdcv5_frame_info_t *info);

/**
 * @brief 解码LHDC V5帧数据
 * @param decoder 解码器实例指针
 * @param frame_data 帧数据指针
 * @param frame_len 帧数据长度
 * @param pcm_out 输出PCM缓冲区
 * @param pcm_out_len 输出PCM缓冲区长度(字节)
 * @param bytes_written 实际写入的字节数
 * @return 0 成功，其他 错误码
 */
int lhdcv5_dec_decode_frame(lhdcv5_decoder_t *decoder, const uint8_t *frame_data, 
                           uint32_t frame_len, uint8_t *pcm_out, 
                           uint32_t pcm_out_len, uint32_t *bytes_written);

/**
 * @brief 获取解码器状态信息
 * @param decoder 解码器实例指针
 * @param buffer 输出缓冲区
 * @param buffer_len 缓冲区长度
 * @return 写入的字节数
 */
int lhdcv5_dec_get_status(lhdcv5_decoder_t *decoder, char *buffer, uint32_t buffer_len);

#endif // LHDC_V5_DEC_H