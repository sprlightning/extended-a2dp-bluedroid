 #ifndef __SP_AUDIO_H__
#define __SP_AUDIO_H__

#include "hal_aud.h"


#ifdef __cplusplus
extern "C" {
#endif

#define EQ_MODE_DEFAULT 0x00  //默认模式
#define EQ_MODE_ROCK  	0x01  //动感模式
#define EQ_MODE_CLASSIC 0x02  //轻松模式
#define EQ_MODE_WARM    0x03  //温暖模式
#define EQ_MODE_CUSTOM 0xff //自定义模式


void sp_audio_init(enum AUD_SAMPRATE_T sample_rate, enum AUD_BITS_T sample_bits);

void sp_audio_func(uint8_t *buf, uint32_t len);


int sp_check_license(void);

void set_app_eq_mode(int mode,char *gain);

#ifdef __cplusplus
}
#endif


#endif
