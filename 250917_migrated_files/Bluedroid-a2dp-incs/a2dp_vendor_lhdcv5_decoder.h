#ifndef A2DP_VENDOR_LHDCV5_DECODER_H
#define A2DP_VENDOR_LHDCV5_DECODER_H

#include "stack/a2dp_vendor.h"
#include "a2dp_codec_api.h"
#include "stack/bt_types.h"
#include "stack/a2d_api.h"
#include "stack/a2dp_decoder.h"
#include "stack/a2dp_shim.h"
#include "lhdcv5BT_dec.h"

// 解决UNUSED_ATTR重定义问题
#ifdef UNUSED_ATTR
#undef UNUSED_ATTR
#endif
#define UNUSED_ATTR __attribute__((unused))

#if (defined(LHDCV5_DEC_INCLUDED) && LHDCV5_DEC_INCLUDED == TRUE)

typedef struct {
    bool initialized;
    HANDLE_LHDCV5_BT handle;
    uint32_t sample_rate;
    uint8_t channel_count;
    uint8_t bits_per_sample;
    decoded_data_callback_t decode_callback;
} tA2DP_LHDCV5_DECODER;

static bool a2dp_lhdcv5_decoder_init(decoded_data_callback_t decode_callback);
static void a2dp_lhdcv5_decoder_cleanup(void);
static void a2dp_lhdcv5_decoder_start(void);
static void a2dp_lhdcv5_decoder_suspend(void);
static void a2dp_lhdcv5_decoder_configure(const uint8_t* p_codec_info);
static ssize_t a2dp_lhdcv5_decoder_decode_header(BT_HDR* p_buf);
static bool a2dp_lhdcv5_decoder_decode_packet(BT_HDR* p_buf, unsigned char* buf, size_t buf_len);

const tA2DP_DECODER_INTERFACE* A2DP_LHDCV5_DecoderInterface(void);

#endif /* LHDCV5_DEC_INCLUDED */

#endif /* A2DP_VENDOR_LHDCV5_DECODER_H */