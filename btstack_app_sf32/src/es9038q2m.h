#ifndef __es9038q2m_H__
#define __es9038q2m_H__
#include "stdint.h"
#include "stdio.h"

// 下面是必要的接口
void es9038q2m_init(void);
void es9038q2m_set_volume(int volume);
void es9038q2m_set_data_width(uint8_t bit_width);

#endif /* __es9038q2m_H__ */
