#ifndef BI_QUAD_FILTER_H
#define BI_QUAD_FILTER_H
typedef enum
{
    LOW_PASS,
    HIGH_PASS,
    BAND_PASS,
    NOTCH,
    ALL_PASS,
    PEAKING,
    LOW_SHELF,
    HIGH_SHELF
} EQ_Type;
void BiQuad_IIR_Init(EQ_Type type, double dbGain, double freq, double srate,
                     double bandwidthOrQOrS, bool isBandwidthOrS,
                     int *output);
#endif