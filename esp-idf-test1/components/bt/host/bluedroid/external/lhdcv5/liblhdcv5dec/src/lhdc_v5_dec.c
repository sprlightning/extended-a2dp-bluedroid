/**
 * @file lhdc_v5_dec.c
 * @brief LHDC V5解码器核心实现
 */
#include "lhdc_v5_dec.h"
#include <string.h>
#include <stdio.h>

// 内部函数声明
static int check_config_validity(const lhdcv5_dec_config_t *config);
static int parse_frame_header(const uint8_t *frame_data, uint32_t frame_len, lhdcv5_frame_info_t *info);

int lhdcv5_dec_init(lhdcv5_decoder_t *decoder, const lhdcv5_dec_config_t *config) {
    if (!decoder || !config) {
        return LHDCV5_DEC_ERR_PARAM;
    }
    
    // 检查配置有效性
    if (check_config_validity(config) != 0) {
        return LHDCV5_DEC_ERR_PARAM;
    }
    
    // 初始化解码器结构体
    memset(decoder, 0, sizeof(lhdcv5_decoder_t));
    memcpy(&decoder->config, config, sizeof(lhdcv5_dec_config_t));
    
    // 初始化工作区
    if (lhdcv5_dec_workspace_init(&decoder->workspace) != 0) {
        return LHDCV5_DEC_ERR_INIT;
    }
    
    decoder->is_running = 1;
    decoder->total_frames_decoded = 0;
    decoder->total_samples_decoded = 0;
    
    return LHDCV5_DEC_SUCCESS;
}

void lhdcv5_dec_deinit(lhdcv5_decoder_t *decoder) {
    if (decoder) {
        lhdcv5_dec_workspace_deinit(&decoder->workspace);
        decoder->is_running = 0;
        memset(decoder, 0, sizeof(lhdcv5_decoder_t));
    }
}

int lhdcv5_dec_get_frame_info(lhdcv5_decoder_t *decoder, const uint8_t *frame_data, 
                             uint32_t frame_len, lhdcv5_frame_info_t *info) {
    if (!decoder || !frame_data || !info || frame_len == 0) {
        return LHDCV5_DEC_ERR_PARAM;
    }
    
    if (!decoder->is_running) {
        return LHDCV5_DEC_ERR_INIT;
    }
    
    // 解析帧头信息
    return parse_frame_header(frame_data, frame_len, info);
}

int lhdcv5_dec_decode_frame(lhdcv5_decoder_t *decoder, const uint8_t *frame_data, 
                           uint32_t frame_len, uint8_t *pcm_out, 
                           uint32_t pcm_out_len, uint32_t *bytes_written) {
    if (!decoder || !frame_data || !pcm_out || !bytes_written || frame_len == 0 || pcm_out_len == 0) {
        return LHDCV5_DEC_ERR_PARAM;
    }
    
    if (!decoder->is_running) {
        return LHDCV5_DEC_ERR_INIT;
    }
    
    lhdcv5_frame_info_t frame_info;
    int ret = parse_frame_header(frame_data, frame_len, &frame_info);
    if (ret != LHDCV5_DEC_SUCCESS) {
        return ret;
    }
    
    // 计算所需的PCM输出缓冲区大小
    uint32_t required_size = frame_info.sample_count * decoder->config.channels * (decoder->config.bit_depth / 8);
    if (pcm_out_len < required_size) {
        return LHDCV5_DEC_ERR_MEM;
    }
    
    // TODO: 实际解码实现
    // 这里只是模拟解码过程，实际项目中需要替换为真实的解码算法
    memset(pcm_out, 0, required_size);
    
    // 更新解码器统计信息
    decoder->total_frames_decoded++;
    decoder->total_samples_decoded += frame_info.sample_count;
    
    *bytes_written = required_size;
    return LHDCV5_DEC_SUCCESS;
}

int lhdcv5_dec_get_status(lhdcv5_decoder_t *decoder, char *buffer, uint32_t buffer_len) {
    if (!decoder || !buffer || buffer_len == 0) {
        return 0;
    }
    
    return snprintf(buffer, buffer_len, 
                   "LHDC V%d.%d Decoder Status:\n"
                   "Running: %s\n"
                   "Config: %d Hz, %d bit, %d channels\n"
                   "Frames decoded: %u\n"
                   "Samples decoded: %u\n"
                   "Workspace status: %s",
                   LHDCV5_VERSION_MAJOR, LHDCV5_VERSION_MINOR,
                   decoder->is_running ? "Yes" : "No",
                   decoder->config.sample_rate, decoder->config.bit_depth, decoder->config.channels,
                   decoder->total_frames_decoded,
                   decoder->total_samples_decoded,
                   lhdcv5_dec_workspace_get_status(&decoder->workspace));
}

// 内部函数实现
static int check_config_validity(const lhdcv5_dec_config_t *config) {
    // 检查采样率
    if (config->sample_rate != LHDCV5_SAMPLERATE_44100 &&
        config->sample_rate != LHDCV5_SAMPLERATE_48000 &&
        config->sample_rate != LHDCV5_SAMPLERATE_96000) {
        return -1;
    }
    
    // 检查比特深度
    if (config->bit_depth != LHDCV5_BITDEPTH_16 &&
        config->bit_depth != LHDCV5_BITDEPTH_24) {
        return -1;
    }
    
    // 检查声道数
    if (config->channels != LHDCV5_CHANNEL_MONO &&
        config->channels != LHDCV5_CHANNEL_STEREO) {
        return -1;
    }
    
    // 检查比特率
    if (config->max_bitrate < 300000 || config->max_bitrate > 900000) {
        return -1;
    }
    
    return 0;
}

static int parse_frame_header(const uint8_t *frame_data, uint32_t frame_len, lhdcv5_frame_info_t *info) {
    if (!frame_data || !info || frame_len < 8) { // 最小帧头大小
        return LHDCV5_DEC_ERR_FRAME;
    }
    
    // 解析帧头信息 (实际项目中需要根据LHDCV5规范解析)
    info->frame_size = frame_len;
    info->bitrate = (frame_data[2] << 16) | (frame_data[3] << 8) | frame_data[4];
    info->sample_count = 1024; // 假设每帧固定样本数，实际需要根据规范计算
    info->frame_type = (frame_data[5] >> 4) & 0x0F;
    info->crc_valid = (frame_data[7] & 0x01) ? 1 : 0;
    
    return LHDCV5_DEC_SUCCESS;
}