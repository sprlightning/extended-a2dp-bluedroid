#ifndef A2DP_VENDOR_LHDCV5_H
#define A2DP_VENDOR_LHDCV5_H

#include "stack/a2dp_vendor.h"
#include "stack/a2dp_vendor_lhdc_constants.h"
#include "stack/a2dp_vendor_lhdcv5_constants.h"

#if (defined(LHDCV5_DEC_INCLUDED) && LHDCV5_DEC_INCLUDED == TRUE)

typedef struct {
    uint8_t vendor_id[3];
    uint8_t codec_id;
    uint8_t sample_rate;
    uint8_t channel_mode;
    uint8_t bits_per_sample;
    uint8_t quality_mode;
    uint8_t reserved;
} tA2DP_LHDCV5_CIE;

tA2D_STATUS A2DP_ParseInfoLhdcv5(tA2DP_LHDCV5_CIE* p_ie, const uint8_t* p_codec_info, bool is_capability);
bool A2DP_IsVendorPeerSinkCodecValidLhdcv5(const uint8_t* p_codec_info);
tA2D_STATUS A2DP_IsVendorPeerSourceCodecValidLhdcv5(const uint8_t* p_codec_info);
btav_a2dp_codec_index_t A2DP_VendorSinkCodecIndexLhdcv5(const uint8_t* p_codec_info);
btav_a2dp_codec_index_t A2DP_VendorSourceCodecIndexLhdcv5(const uint8_t* p_codec_info);
bool A2DP_VendorInitCodecConfigLhdcv5(btav_a2dp_codec_index_t codec_index, UINT8* p_result);
bool A2DP_VendorBuildCodecConfigLhdcv5(UINT8* p_src_cap, UINT8* p_result);
const char* A2DP_VendorCodecNameLhdcv5(const uint8_t* p_codec_info);
bool A2DP_VendorCodecTypeEqualsLhdcv5(const uint8_t* p_codec_info_a, const uint8_t* p_codec_info_b);
const tA2DP_DECODER_INTERFACE* A2DP_GetVendorDecoderInterfaceLhdcv5(const uint8_t* p_codec_info);

extern const tA2DP_LHDCV5_CIE a2dp_lhdcv5_source_caps;
extern const tA2DP_LHDCV5_CIE a2dp_lhdcv5_sink_caps;

#endif /* LHDCV5_DEC_INCLUDED */

#endif /* A2DP_VENDOR_LHDCV5_H */