/**
 * @file lhdcv5BT_dec.c
 * @brief 适用于蓝牙A2DP的LHDC V5解码器实现
 */
#include "lhdcv5BT_dec.h"
#include <string.h>
#include <stdio.h>

// 静态解码器实例和状态
static lhdcv5_decoder_t s_decoder;
static uint8_t s_is_initialized = 0;
static uint8_t s_serial_no = 0xff;

// 内部函数声明
static int assemble_lhdcv5_packet(uint8_t *input, uint32_t input_len, uint8_t **pLout, 
                                 uint32_t *pLlen, int upd_seq_no);

int lhdcv5BT_dec_init_decoder(tLHDCV5_DEC_CONFIG *config) {
    if (!config) {
        return LHDCBT_DEC_FUNC_FAIL;
    }
    
    // 检查参数有效性
    if ((config->bits_depth != LHDCV5_BITDEPTH_16) && 
        (config->bits_depth != LHDCV5_BITDEPTH_24)) {
        return LHDCBT_DEC_FUNC_FAIL;
    }
    
    if ((config->sample_rate != LHDCV5_SAMPLERATE_44100) && 
        (config->sample_rate != LHDCV5_SAMPLERATE_48000) &&
        (config->sample_rate != LHDCV5_SAMPLERATE_96000)) {
        return LHDCBT_DEC_FUNC_FAIL;
    }
    
    if (config->version != VERSION_5) {
        return LHDCBT_DEC_FUNC_FAIL;
    }
    
    // 初始化解码器配置
    lhdcv5_dec_config_t dec_config = {
        .sample_rate = config->sample_rate,
        .bit_depth = config->bits_depth,
        .channels = LHDCV5_CHANNEL_STEREO,
        .max_bitrate = 900000,  // 最大支持900kbps
        .use_hw_accel = 0       // 默认不使用硬件加速
    };
    
    // 初始化解码器
    if (lhdcv5_dec_init(&s_decoder, &dec_config) != LHDCV5_DEC_SUCCESS) {
        return LHDCBT_DEC_FUNC_FAIL;
    }
    
    s_serial_no = 0xff;
    s_is_initialized = 1;
    
    return LHDCBT_DEC_FUNC_SUCCEED;
}

int lhdcv5BT_dec_check_frame_data_enough(const uint8_t *frameData, uint32_t frameBytes, uint32_t *packetBytes) {
    if (!s_is_initialized || !frameData || !packetBytes) {
        return LHDCBT_DEC_FUNC_FAIL;
    }
    
    uint8_t *in_buf = NULL;
    uint32_t in_len = 0;
    uint32_t frame_num = 0;
    lhdcv5_frame_info_t frame_info;
    uint32_t ptr_offset = 0;
    
    *packetBytes = 0;
    
    // 组装LHDC数据包
    frame_num = assemble_lhdcv5_packet((uint8_t *)frameData, frameBytes, &in_buf, &in_len, LHDCBT_DEC_NOT_UPD_SEQ_NO);
    if (frame_num == 0) {
        return LHDCBT_DEC_FUNC_SUCCEED;
    }
    
    ptr_offset = 0;
    while ((frame_num > 0) && (ptr_offset < in_len)) {
        // 获取帧信息
        if (lhdcv5_dec_get_frame_info(&s_decoder, in_buf + ptr_offset, in_len - ptr_offset, &frame_info) != LHDCV5_DEC_SUCCESS) {
            return LHDCBT_DEC_FUNC_FAIL;
        }
        
        // 检查帧数据是否完整
        if ((ptr_offset + frame_info.frame_size) > in_len) {
            return LHDCBT_DEC_FUNC_INPUT_NOT_ENOUGH;
        }
        
        ptr_offset += frame_info.frame_size;
        frame_num--;
    }
    
    *packetBytes = ptr_offset;
    return LHDCBT_DEC_FUNC_SUCCEED;
}

int lhdcv5BT_dec_decode(const uint8_t *frameData, uint32_t frameBytes, uint8_t* pcmData, 
                       uint32_t* pcmBytes, uint32_t bits_depth) {
    if (!s_is_initialized || !frameData || !pcmData || !pcmBytes) {
        return LHDCBT_DEC_FUNC_FAIL;
    }
    
    uint8_t *in_buf = NULL;
    uint32_t in_len = 0;
    uint32_t frame_num = 0;
    lhdcv5_frame_info_t frame_info;
    uint32_t ptr_offset = 0;
    uint32_t total_bytes_written = 0;
    uint32_t pcm_space = *pcmBytes;
    
    *pcmBytes = 0;
    
    // 组装LHDC数据包
    frame_num = assemble_lhdcv5_packet((uint8_t *)frameData, frameBytes, &in_buf, &in_len, LHDCBT_DEC_UPD_SEQ_NO);
    if (frame_num == 0) {
        return LHDCBT_DEC_FUNC_SUCCEED;
    }
    
    ptr_offset = 0;
    while ((frame_num > 0) && (ptr_offset < in_len)) {
        uint32_t bytes_written;
        
        // 获取帧信息
        if (lhdcv5_dec_get_frame_info(&s_decoder, in_buf + ptr_offset, in_len - ptr_offset, &frame_info) != LHDCV5_DEC_SUCCESS) {
            return LHDCBT_DEC_FUNC_FAIL;
        }
        
        // 检查输入数据是否足够
        if ((ptr_offset + frame_info.frame_size) > in_len) {
            return LHDCBT_DEC_FUNC_INPUT_NOT_ENOUGH;
        }
        
        // 计算所需输出缓冲区大小
        uint32_t required_size = frame_info.sample_count * 2 * (bits_depth / 8); // 2声道
        if (total_bytes_written + required_size > pcm_space) {
            return LHDCBT_DEC_FUNC_OUTPUT_NOT_ENOUGH;
        }
        
        // 解码当前帧
        if (lhdcv5_dec_decode_frame(&s_decoder, in_buf + ptr_offset, frame_info.frame_size,
                                   pcmData + total_bytes_written, required_size, &bytes_written) != LHDCV5_DEC_SUCCESS) {
            return LHDCBT_DEC_FUNC_FAIL;
        }
        
        // 如果需要，转换为16位PCM
        if (s_decoder.config.bit_depth == 24 && bits_depth == 16) {
            lhdcv5_convert_24_to_16(pcmData + total_bytes_written, 
                                   pcmData + total_bytes_written, 
                                   frame_info.sample_count, 2);
            bytes_written = frame_info.sample_count * 2 * 2; // 16位，2声道
        }
        
        ptr_offset += frame_info.frame_size;
        total_bytes_written += bytes_written;
        frame_num--;
    }
    
    *pcmBytes = total_bytes_written;
    return LHDCBT_DEC_FUNC_SUCCEED;
}

int lhdcv5BT_dec_deinit_decoder(void) {
    if (s_is_initialized) {
        lhdcv5_dec_deinit(&s_decoder);
        s_is_initialized = 0;
    }
    return LHDCBT_DEC_FUNC_SUCCEED;
}

// 内部函数：组装LHDC数据包
static int assemble_lhdcv5_packet(uint8_t *input, uint32_t input_len, uint8_t **pLout, 
                                 uint32_t *pLlen, int upd_seq_no) {
    if (!input || !pLout || !pLlen || input_len == 0) {
        return 0;
    }
    
    // 简单实现：假设输入已经是正确的LHDC数据包格式
    // 实际应用中需要根据蓝牙A2DP协议和LHDC规范处理数据包
    *pLout = input;
    *pLlen = input_len;
    
    // 更新序列号
    if (upd_seq_no) {
        s_serial_no++;
        if (s_serial_no == 0xff) {
            s_serial_no = 0;
        }
    }
    
    // 简单返回1表示包含1帧，实际应根据数据包内容解析
    return 1;
}