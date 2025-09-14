#ifndef PCM_CTRL_H
#define PCM_CTRL_H
#include <stdint.h>
#include <stdbool.h>

#define MAX_PCM_SAMPLERATE       48000
#define MAX_PCM_DATA_WIDTH       16
#define MAX_PCM_CHANNEL          2
#define MAX_BUFFER_LEN_MS        200
#define MAX_PCM_BUFFER_SIZE      ((MAX_PCM_SAMPLERATE * MAX_PCM_CHANNEL * (MAX_PCM_DATA_WIDTH >> 3)) * MAX_BUFFER_LEN_MS) / 1000
//48000 Hz 16bit 1/5s or 200ms
#define TEMP_PCM_BUFFER_MAX_SIZE 4096
void pcm_open(uint32_t sample_rate, uint32_t data_width, uint32_t sound_channel_num, uint32_t base_buffer_size, uint32_t buffer_num);
void set_start_loc(int32_t index);
void check_buffer_edge(uint32_t size);
void *get_pcm_tail();
#endif