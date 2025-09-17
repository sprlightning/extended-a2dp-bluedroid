
#ifndef __APP_DET_H__
#define __APP_DET_H__

#ifdef __cplusplus
extern "C" {
#endif



void box_det_init(void);
void close_box_det(void);
void out_box_det(void);

void boxdet_init_delay_thread(void);
void boxdet_Send_Cmd(uint32_t idtest);

extern bool earinboxflg;


#ifdef __cplusplus
}
#endif

#endif
