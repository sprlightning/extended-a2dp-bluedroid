/**
 * @file lhdcv5_util_dec.c
 * @brief LHDC V5解码工具函数实现
 */
#include "lhdcv5_util_dec.h"
#include <stdio.h>
#include <string.h>

uint16_t lhdcv5_crc16(const uint8_t *data, uint32_t length) {
    uint16_t crc = 0xFFFF;
    uint8_t i;
    
    if (!data || length == 0) {
        return crc;
    }
    
    while (length--) {
        crc ^= (*data++) << 8;
        
        for (i = 0; i < 8; i++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ LHDCV5_CRC_POLY;
            } else {
                crc <<= 1;
            }
        }
    }
    
    return crc;
}

int lhdcv5_check_crc(const uint8_t *frame_data, uint32_t frame_len) {
    if (!frame_data || frame_len < 2) {
        return 0;
    }
    
    // 最后两个字节是CRC值
    uint16_t received_crc = (frame_data[frame_len - 2] << 8) | frame_data[frame_len - 1];
    uint16_t calculated_crc = lhdcv5_crc16(frame_data, frame_len - 2);
    
    return (received_crc == calculated_crc) ? 1 : 0;
}

uint32_t lhdcv5_rate_code_to_value(uint8_t rate_code) {
    switch (rate_code) {
        case 0x00: return LHDCV5_SAMPLERATE_44100;
        case 0x01: return LHDCV5_SAMPLERATE_48000;
        case 0x02: return LHDCV5_SAMPLERATE_96000;
        default: return 0; // 无效采样率
    }
}

uint32_t lhdcv5_bitrate_code_to_value(uint8_t bitrate_code) {
    // 根据LHDCV5规范，比特率编码对应的实际值
    switch (bitrate_code) {
        case 0x00: return 300000;   // 300kbps
        case 0x01: return 400000;   // 400kbps
        case 0x02: return 500000;   // 500kbps
        case 0x03: return 600000;   // 600kbps
        case 0x04: return 700000;   // 700kbps
        case 0x05: return 800000;   // 800kbps
        case 0x06: return 900000;   // 900kbps
        default: return 0;          // 无效比特率
    }
}

void lhdcv5_print_frame_info(const lhdcv5_frame_info_t *frame_info) {
    if (!frame_info) return;
    
    printf("LHDCV5 Frame Info:\n");
    printf("  Frame size: %u bytes\n", frame_info->frame_size);
    printf("  Sample count: %u\n", frame_info->sample_count);
    printf("  Bitrate: %u bps\n", frame_info->bitrate);
    printf("  Frame type: 0x%02X\n", frame_info->frame_type);
    printf("  CRC valid: %s\n", frame_info->crc_valid ? "Yes" : "No");
}

void lhdcv5_convert_24_to_16(const uint8_t *in, uint8_t *out, uint32_t samples, uint32_t channels) {
    if (!in || !out || samples == 0 || channels == 0) {
        return;
    }
    
    uint32_t total_samples = samples * channels;
    const int32_t *in_24 = (const int32_t *)in;
    int16_t *out_16 = (int16_t *)out;
    
    for (uint32_t i = 0; i < total_samples; i++) {
        // 24位转16位，右移8位并饱和处理
        int32_t sample = (in_24[i] >> 8) & 0xFFFFFF;
        if (sample & 0x800000) {
            sample |= 0xFF000000; // 符号扩展
        }
        
        // 饱和到16位范围
        if (sample > 32767) {
            out_16[i] = 32767;
        } else if (sample < -32768) {
            out_16[i] = -32768;
        } else {
            out_16[i] = (int16_t)sample;
        }
    }
}