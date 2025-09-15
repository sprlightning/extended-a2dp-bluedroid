/**
 * @file lhdc_v5_dec_workspace.h
 * @brief LHDC V5解码器工作区定义，用于内存管理和状态维护
 */
#ifndef LHDC_V5_DEC_WORKSPACE_H
#define LHDC_V5_DEC_WORKSPACE_H

#include <stdint.h>
#include <stdlib.h>

// LHDC V5解码器工作区大小定义
#define LHDCV5_DEC_WORKSPACE_SIZE     0x8000  // 32KB工作区
#define LHDCV5_FRAME_INFO_SIZE        64      // 帧信息结构体大小
#define LHDCV5_TEMP_BUFFER_SIZE       2048    // 临时缓冲区大小

/**
 * @brief LHDC V5解码器工作区结构体
 * 用于存储解码器运行时所需的各种缓冲区和状态信息
 */
typedef struct {
    uint8_t main_workspace[LHDCV5_DEC_WORKSPACE_SIZE];  // 主工作区
    uint8_t frame_info[LHDCV5_FRAME_INFO_SIZE];         // 帧信息缓冲区
    uint8_t temp_buffer[LHDCV5_TEMP_BUFFER_SIZE];       // 临时缓冲区
    uint32_t workspace_used;                            // 已使用的工作区大小
    uint32_t last_error;                                // 最后一次错误码
    uint8_t is_initialized;                             // 初始化标志
} lhdcv5_dec_workspace_t;

/**
 * @brief 初始化解码器工作区
 * @param workspace 工作区指针
 * @return 0 成功，其他 错误码
 */
int lhdcv5_dec_workspace_init(lhdcv5_dec_workspace_t *workspace);

/**
 * @brief 释放解码器工作区资源
 * @param workspace 工作区指针
 */
void lhdcv5_dec_workspace_deinit(lhdcv5_dec_workspace_t *workspace);

/**
 * @brief 检查工作区是否有足够空间
 * @param workspace 工作区指针
 * @param size 需要的空间大小
 * @return 1 有足够空间，0 空间不足
 */
int lhdcv5_dec_workspace_check_space(lhdcv5_dec_workspace_t *workspace, uint32_t size);

/**
 * @brief 获取工作区状态信息
 * @param workspace 工作区指针
 * @return 工作区状态描述字符串
 */
const char* lhdcv5_dec_workspace_get_status(lhdcv5_dec_workspace_t *workspace);

#endif // LHDC_V5_DEC_WORKSPACE_H