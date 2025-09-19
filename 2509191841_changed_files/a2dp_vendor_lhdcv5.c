/**
 * SPDX-FileCopyrightText: 2025 The Android Open Source Project
 *
 * SPDX-License-Identifier: Apache-2.0
 * 
 * a2dp_vendor_lhdcv5.c
 * 
 * a2dp_vendor.c <-> a2dp_vendor_lhdcv5.c <-> a2dp_vendor_lhdcv5_decoder.c <-> lhdcv5BT_dec.h
 */

#include <string.h>
#include "a2d_int.h"
#include "common/bt_defs.h"
#include "common/bt_target.h"
#include "stack/a2d_api.h"
#include "stack/a2d_sbc.h"
#include "stack/a2dp_vendor_lhdcv5.h"
#include "stack/a2dp_vendor_lhdcv5_decoder.h"
#include "bt_av.h"

#if (defined(LHDCV5_DEC_INCLUDED) && LHDCV5_DEC_INCLUDED == TRUE)

// source capabilities
static const tA2DP_LHDCV5_CIE a2dp_lhdcv5_source_caps = {
    A2DP_LHDC_VENDOR_ID,  // vendorId
    A2DP_LHDCV5_CODEC_ID, // codecId
    // Sampling Frequency
    (A2DP_LHDCV5_SAMPLING_FREQ_44100 | A2DP_LHDCV5_SAMPLING_FREQ_48000 | A2DP_LHDCV5_SAMPLING_FREQ_96000 | A2DP_LHDCV5_SAMPLING_FREQ_192000),
    // Bits Per Sample
    (A2DP_LHDCV5_BIT_FMT_16 | A2DP_LHDCV5_BIT_FMT_24),
    // Channel Mode
    A2DP_LHDCV5_CHANNEL_MODE_STEREO,
    // Codec SubVersion Number
    A2DP_LHDCV5_VER_1,
    // Encoded Frame Length
    A2DP_LHDCV5_FRAME_LEN_5MS,
    // Max Target Bit Rate Type
    A2DP_LHDCV5_MAX_BIT_RATE_1000K,
    // Min Target Bit Rate Type
    A2DP_LHDCV5_MIN_BIT_RATE_64K,
    // FeatureSupported: AR
    false,
    // FeatureSupported: JAS
    false,
    // FeatureSupported: META
    false,
    // FeatureSupported: Low Latency
    true,
    // FeatureSupported: Lossless (standard 48 KHz)
    false,
    // Lossless extended configurable: 24 bit-per-sample
    false,
    // Lossless extended configurable: 96 KHz
    false,
#ifdef LHDC_LOSSLESS_RAW_SUPPORT
    // FeatureSupported: Lossless Raw mode (standard 48 KHz)
    false,
#endif
};

// default source capabilities for best select
static const tA2DP_LHDCV5_CIE a2dp_lhdcv5_source_default_caps = {
    A2DP_LHDC_VENDOR_ID,  // vendorId
    A2DP_LHDCV5_CODEC_ID, // codecId
    // Sampling Frequency
    A2DP_LHDCV5_SAMPLING_FREQ_48000,
    // Bits Per Sample
    A2DP_LHDCV5_BIT_FMT_24,
    // Channel Mode
    A2DP_LHDCV5_CHANNEL_MODE_STEREO,
    // Codec Version Number
    A2DP_LHDCV5_VER_1,
    // Encoded Frame Length
    A2DP_LHDCV5_FRAME_LEN_5MS,
    // Max Target Bit Rate Type
    A2DP_LHDCV5_MAX_BIT_RATE_1000K,
    // Min Target Bit Rate Type
    A2DP_LHDCV5_MIN_BIT_RATE_64K,
    // FeatureSupported: AR
    false,
    // FeatureSupported: JAS
    false,
    // FeatureSupported: META
    false,
    // FeatureSupported: Low Latency
    true,
    // FeatureSupported: Lossless (standard 48 KHz)
    false,
    // Lossless extended configurable: 24 bit-per-sample
    false,
    // Lossless extended configurable: 96 KHz
    false,
#ifdef LHDC_LOSSLESS_RAW_SUPPORT
    // FeatureSupported: Lossless Raw mode (standard 48 KHz)
    false,
#endif
};

// sink capabilities
static const tA2DP_LHDCV5_CIE a2dp_lhdcv5_sink_caps = {
    A2DP_LHDC_VENDOR_ID,  // vendorId
    A2DP_LHDCV5_CODEC_ID, // codecId
    // Sampling Frequency
    (A2DP_LHDCV5_SAMPLING_FREQ_44100 | A2DP_LHDCV5_SAMPLING_FREQ_48000 | A2DP_LHDCV5_SAMPLING_FREQ_96000 | A2DP_LHDCV5_SAMPLING_FREQ_192000),
    // Bits Per Sample
    (A2DP_LHDCV5_BIT_FMT_16 | A2DP_LHDCV5_BIT_FMT_24 | A2DP_LHDCV5_BIT_FMT_32),
    // Channel Mode
    A2DP_LHDCV5_CHANNEL_MODE_STEREO,
    // Codec Version Number
    A2DP_LHDCV5_VER_1,
    // Encoded Frame Length
    A2DP_LHDCV5_FRAME_LEN_5MS,
    // Max Target Bit Rate Type
    A2DP_LHDCV5_MAX_BIT_RATE_1000K,
    // Min Target Bit Rate Type
    A2DP_LHDCV5_MIN_BIT_RATE_64K,
    // FeatureSupported: AR
    false,
    // FeatureSupported: JAS
    false,
    // FeatureSupported: META
    false,
    // FeatureSupported: Low Latency
    true,
    // FeatureSupported: Lossless (standard 48 KHz)
    false,
    // Lossless extended configurable: 24 bit-per-sample
    false,
    // Lossless extended configurable: 96 KHz
    false,
#ifdef LHDC_LOSSLESS_RAW_SUPPORT
    // FeatureSupported: Lossless Raw mode (standard 48 KHz)
    false,
#endif
};

// default sink capabilities
UNUSED_ATTR static const tA2DP_LHDCV5_CIE a2dp_lhdcv5_sink_default_caps = {
    A2DP_LHDC_VENDOR_ID,  // vendorId
    A2DP_LHDCV5_CODEC_ID, // codecId
    // Sampling Frequency
    A2DP_LHDCV5_SAMPLING_FREQ_48000,
    // Bits Per Sample
    A2DP_LHDCV5_BIT_FMT_24,
    // Channel Mode
    A2DP_LHDCV5_CHANNEL_MODE_STEREO,
    // Codec Version Number
    A2DP_LHDCV5_VER_1,
    // Encoded Frame Length
    A2DP_LHDCV5_FRAME_LEN_5MS,
    // Max Target Bit Rate Type
    A2DP_LHDCV5_MAX_BIT_RATE_1000K,
    // Min Target Bit Rate Type
    A2DP_LHDCV5_MIN_BIT_RATE_64K,
    // FeatureSupported: AR
    false,
    // FeatureSupported: JAS
    false,
    // FeatureSupported: META
    false,
    // FeatureSupported: Low Latency
    true,
    // FeatureSupported: Lossless (standard 48 KHz)
    false,
    // Lossless extended configurable: 24 bit-per-sample
    false,
    // Lossless extended configurable: 96 KHz
    false,
#ifdef LHDC_LOSSLESS_RAW_SUPPORT
    // FeatureSupported: Lossless Raw mode (standard 48 KHz)
    false,
#endif
};

/* Default LHDCV5 codec configuration */
static const tA2DP_LHDCV5_CIE a2dp_lhdcv5_default_config = {
    A2DP_LHDC_VENDOR_ID,                  // vendorId
    A2DP_LHDCV5_CODEC_ID,                 // codecId
    A2DP_LHDCV5_SAMPLING_FREQ_48000,      // sampleRate
    A2DP_LHDCV5_CHANNEL_MODE_STEREO,      // channelMode
    BTAV_A2DP_CODEC_BITS_PER_SAMPLE_24    // bits_per_sample
};

/**
 * @brief A2DP LHDCV5 decoder interface.
 * 
 * 由a2dp_vendor_lhdcv5_decoder.c实现
 * 这里的函数顺序及参数必须严格遵循a2dp_codec_api.h中tA2DP_DECODER_INTERFACE的定义
 */
static const tA2DP_DECODER_INTERFACE a2dp_decoder_interface_lhdcv5 = {
    a2dp_lhdcv5_decoder_init, // decoder_init
    a2dp_lhdcv5_decoder_cleanup, // decoder_cleanup
    NULL,  // decoder_reset(非必须)
    a2dp_lhdcv5_decoder_decode_packet_header, // decoder_decode_packet_header
    a2dp_lhdcv5_decoder_decode_packet, // decoder_decode_packet
    a2dp_lhdcv5_decoder_start,  // decoder_start(非必须)
    a2dp_lhdcv5_decoder_suspend,  // decoder_suspend(非必须)
    a2dp_lhdcv5_decoder_configure, // decoder_configure
};

//
// Utilities for LHDC configuration on A2DP specifics - START
//
typedef struct {
  btav_a2dp_codec_index_t *_codec_config_;
  btav_a2dp_codec_index_t *_codec_capability_;
  btav_a2dp_codec_index_t *_codec_local_capability_;
  btav_a2dp_codec_index_t *_codec_selectable_capability_;
  btav_a2dp_codec_index_t *_codec_user_config_;
  btav_a2dp_codec_index_t *_codec_audio_config_;
}tA2DP_CODEC_CONFIGS_PACK;

typedef struct {
  uint8_t   featureCode;  /* code of LHDC features */
  uint8_t   inSpecBank;   /* target specific to store the feature flag */
  uint8_t   bitPos;       /* the bit index(0~63) of the specific(int64_t) that bit store */
  int64_t   value;        /* real value of the bit position written to the target specific */
}tA2DP_LHDC_FEATURE_POS;

// default settings of LHDC features configuration on specifics
// info of feature: JAS
static const tA2DP_LHDC_FEATURE_POS a2dp_lhdcv5_source_spec_JAS = {
    LHDCV5_FEATURE_CODE_JAS,
    LHDCV5_FEATURE_ON_A2DP_SPECIFIC_3,
    LHDCV5_FEATURE_JAS_SPEC_BIT_POS,
    (0x1ULL << LHDCV5_FEATURE_JAS_SPEC_BIT_POS),
};

// info of feature: AR
static const tA2DP_LHDC_FEATURE_POS a2dp_lhdcv5_source_spec_AR = {
    LHDCV5_FEATURE_CODE_AR,
    LHDCV5_FEATURE_ON_A2DP_SPECIFIC_3,
    LHDCV5_FEATURE_AR_SPEC_BIT_POS,
    (0x1ULL << LHDCV5_FEATURE_AR_SPEC_BIT_POS),
};

// info of feature: META
static const tA2DP_LHDC_FEATURE_POS a2dp_lhdcv5_source_spec_META = {
    LHDCV5_FEATURE_CODE_META,
    LHDCV5_FEATURE_ON_A2DP_SPECIFIC_3,
    LHDCV5_FEATURE_META_SPEC_BIT_POS,
    (0x1ULL << LHDCV5_FEATURE_META_SPEC_BIT_POS),
};

// info of feature: Low Latency
static const tA2DP_LHDC_FEATURE_POS a2dp_lhdcv5_source_spec_LL = {
    LHDCV5_FEATURE_CODE_LL,
    LHDCV5_FEATURE_ON_A2DP_SPECIFIC_2,
    LHDCV5_FEATURE_LL_SPEC_BIT_POS,
    (0x1ULL << LHDCV5_FEATURE_LL_SPEC_BIT_POS),
};

// info of feature: LossLess
static const tA2DP_LHDC_FEATURE_POS a2dp_lhdcv5_source_spec_LLESS = {
    LHDCV5_FEATURE_CODE_LLESS,
    LHDCV5_FEATURE_ON_A2DP_SPECIFIC_3,
    LHDCV5_FEATURE_LLESS_SPEC_BIT_POS,
    (0x1ULL << LHDCV5_FEATURE_LLESS_SPEC_BIT_POS),
};

#ifdef LHDC_LOSSLESS_RAW_SUPPORT
// info of feature: LossLess Raw
static const tA2DP_LHDC_FEATURE_POS a2dp_lhdcv5_source_spec_LLESS_RAW = {
    LHDCV5_FEATURE_CODE_LLESS_RAW,
    LHDCV5_FEATURE_ON_A2DP_SPECIFIC_3,
    LHDCV5_FEATURE_LLESS_RAW_SPEC_BIT_POS,
    (0x1ULL << LHDCV5_FEATURE_LLESS_RAW_SPEC_BIT_POS),
};
#endif


UNUSED_ATTR static const tA2DP_LHDC_FEATURE_POS a2dp_lhdcv5_source_spec_all[] = {
    a2dp_lhdcv5_source_spec_JAS,
    a2dp_lhdcv5_source_spec_AR,
    a2dp_lhdcv5_source_spec_META,
    a2dp_lhdcv5_source_spec_LL,
    a2dp_lhdcv5_source_spec_LLESS,
#ifdef LHDC_LOSSLESS_RAW_SUPPORT
    a2dp_lhdcv5_source_spec_LLESS_RAW,
#endif
};

// check if target version is supported right now
static bool is_codec_version_supported(uint8_t version, bool is_source) {
  const tA2DP_LHDCV5_CIE* p_a2dp_lhdcv5_caps =
      (is_source) ? &a2dp_lhdcv5_source_caps : &a2dp_lhdcv5_sink_caps;

  if ((version & p_a2dp_lhdcv5_caps->version) != A2DP_LHDCV5_VER_NS) {
    return true;
    LOG_INFO("LHDCV5: version supported! peer:{%s} local:{%s}", 
         version, p_a2dp_lhdcv5_caps->version);
  }

  LOG_ERROR( "LHDCV5: version unsupported! peer:{%s} local:{%s}",
       version, p_a2dp_lhdcv5_caps->version);
  return false;
}

// Builds the LHDC Media Codec Capabilities byte sequence beginning from the
// LOSC octet. |media_type| is the media type |AVDT_MEDIA_TYPE_*|.
// |p_ie| is a pointer to the LHDC Codec Information Element information.
// The result is stored in |p_result|. Returns A2DP_SUCCESS on success,
// otherwise the corresponding A2DP error status code.
tA2D_STATUS A2DP_BuildInfoLhdcv5(uint8_t media_type,
                                       const tA2DP_LHDCV5_CIE* p_ie,
                                       uint8_t* p_result) {

  const uint8_t* tmpInfo = p_result;
  uint8_t para = 0;

  if (p_ie == NULL || p_result == NULL) {
    LOG_ERROR( "LHDCV5: nullptr input");
    return A2DP_INVALID_PARAMS;
  }

  *p_result++ = A2DP_LHDCV5_CODEC_LEN;  // H0
  *p_result++ = (media_type << 4);      // H1
  *p_result++ = A2DP_MEDIA_CT_NON_A2DP; // H2

  // Vendor ID(P0-P3) and Codec ID(P4-P5)
  *p_result++ = (uint8_t)(p_ie->vendorId & 0x000000FF);
  *p_result++ = (uint8_t)((p_ie->vendorId & 0x0000FF00) >> 8);
  *p_result++ = (uint8_t)((p_ie->vendorId & 0x00FF0000) >> 16);
  *p_result++ = (uint8_t)((p_ie->vendorId & 0xFF000000) >> 24);
  *p_result++ = (uint8_t)(p_ie->codecId & 0x00FF);
  *p_result++ = (uint8_t)((p_ie->codecId & 0xFF00) >> 8);

  para = 0;
  // P6[5:0] Sampling Frequency
  if ((p_ie->sampleRate & A2DP_LHDCV5_SAMPLING_FREQ_MASK) != A2DP_LHDCV5_SAMPLING_FREQ_NS) {
    para |= (p_ie->sampleRate & A2DP_LHDCV5_SAMPLING_FREQ_MASK);
  } else {
    LOG_ERROR( "LHDCV5: invalid sample rate (0x{%02x})",  p_ie->sampleRate);
    return A2DP_INVALID_PARAMS;
  }
  // update P6
  *p_result++ = para; para = 0;

  // P7[2:0] Bit Depth
  if ((p_ie->bits_per_sample & A2DP_LHDCV5_BIT_FMT_MASK) != A2DP_LHDCV5_BIT_FMT_NS) {
    para |= (p_ie->bits_per_sample & A2DP_LHDCV5_BIT_FMT_MASK);
  } else {
    LOG_ERROR( "LHDCV5: invalid bits per sample (0x{%02x})",  p_ie->bits_per_sample);
    return A2DP_INVALID_PARAMS;
  }
  // P7[5:4] Max Target Bit Rate
  para |= (p_ie->maxTargetBitrate & A2DP_LHDCV5_MAX_BIT_RATE_MASK);
  // P7[7:6] Min Target Bit Rate
  para |= (p_ie->minTargetBitrate & A2DP_LHDCV5_MIN_BIT_RATE_MASK);
  // update P7
  *p_result++ = para; para = 0;

  // P8[3:0] Codec SubVersion
  if ((p_ie->version & A2DP_LHDCV5_VERSION_MASK) != A2DP_LHDCV5_VER_NS) {
    para = para | (p_ie->version & A2DP_LHDCV5_VERSION_MASK);
  } else {
    LOG_ERROR( "LHDCV5: invalid codec subversion (0x{%02x})",  p_ie->version);
    return A2DP_INVALID_PARAMS;
  }
  // P8[5:4] Frame Length Type
  if ((p_ie->frameLenType & A2DP_LHDCV5_FRAME_LEN_MASK) != A2DP_LHDCV5_FRAME_LEN_NS) {
    para = para | (p_ie->frameLenType & A2DP_LHDCV5_FRAME_LEN_MASK);
  } else {
    LOG_ERROR( "LHDCV5: invalid frame length type (0x{%02x})",  p_ie->frameLenType);
    return A2DP_INVALID_PARAMS;
  }
  // update P8
  *p_result++ = para; para = 0;

  // P9[0] HasAR
  // P9[1] HasJAS
  // P9[2] HasMeta
  // P9[4] HasLossless96K
  // P9[5] HasLossless24Bit
  // P9[6] HasLL
  // P9[7] HasLossless48K
  if (p_ie->hasFeatureAR) {
    para |= A2DP_LHDCV5_FEATURE_AR;
  }
  if (p_ie->hasFeatureJAS) {
    para |= A2DP_LHDCV5_FEATURE_JAS;
  }
  if (p_ie->hasFeatureMETA) {
    para |= A2DP_LHDCV5_FEATURE_META;
  }
  if (p_ie->hasFeatureLL) {
    para |= A2DP_LHDCV5_FEATURE_LL;
  }
  if (p_ie->hasFeatureLLESS48K) {
    para |= A2DP_LHDCV5_FEATURE_LLESS48K;
  }
  if (p_ie->hasFeatureLLESS24Bit) {
    para |= A2DP_LHDCV5_FEATURE_LLESS24BIT;
  }
  if (p_ie->hasFeatureLLESS96K) {
    para |= A2DP_LHDCV5_FEATURE_LLESS96K;
  }
  // update P9
  *p_result++ = para; para = 0;

  #ifdef LHDC_LOSSLESS_RAW_SUPPORT
  // P10[7] HaslosslessRaw
  if (p_ie->hasFeatureLLESSRaw) {
    para |= A2DP_LHDCV5_FEATURE_LLESS_RAW;
  }
#endif
  // update P10
  *p_result++ = para; para = 0;

  // // Channel Mode
  // *p_result = (uint8_t)(p_ie->channelMode & A2DP_LHDCV5_CHANNEL_MODE_MASK);
  // if (*p_result == 0) return A2DP_INVALID_PARAMS;

  LOG_INFO( "LHDCV5: codec info built = H0-H2:{%02x} {%02x} {%02x} P0-P3:{%02x} "
      "{%02x} {%02x} {%02x} P4-P5:{%02x} {%02x} P6:{%02x} P7:{%02x} P8:{%02x} P9:{%02x} P10:{%02x}", 
      tmpInfo[0], tmpInfo[1], tmpInfo[2], tmpInfo[3], tmpInfo[4], tmpInfo[5], tmpInfo[6], tmpInfo[7],
      tmpInfo[8], tmpInfo[9], tmpInfo[10], tmpInfo[11], tmpInfo[12], tmpInfo[A2DP_LHDCV5_CODEC_LEN]);

  return A2DP_SUCCESS;
}

// Parses the LHDC Media Codec Capabilities byte sequence beginning from the
// LOSC octet. The result is stored in |p_ie|. The byte sequence to parse is
// |p_codec_info|. If |is_capability| is true, the byte sequence is
// codec capabilities, otherwise is codec configuration.
// Returns A2DP_SUCCESS on success, otherwise the corresponding A2DP error
// status code.
tA2DP_STATUS A2DP_ParseInfoLhdcv5(tA2DP_LHDCV5_CIE* p_ie,
    const uint8_t* p_codec_info,
    bool is_capability,
    bool is_source) {
  uint8_t losc;
  uint8_t media_type;
  tA2DP_CODEC_TYPE codec_type;
  const uint8_t* tmpInfo = p_codec_info;
  const uint8_t* p_codec_Info_save = p_codec_info;
  // bool is_source = false; // 默认做sink

  if (p_ie == NULL || p_codec_info == NULL) {
    LOG_ERROR( "LHDCV5: nullptr input");
    return AVDT_ERR_UNSUP_CFG;
  }

  // Codec capability length
  losc = *p_codec_info++;
  if (losc != A2DP_LHDCV5_CODEC_LEN) {
    LOG_ERROR( "LHDCV5: wrong length {%d}",  losc);
    return AVDT_ERR_UNSUP_CFG;
  }

  media_type = (*p_codec_info++) >> 4;
  codec_type = (tA2DP_CODEC_TYPE)(*p_codec_info++);

  // Media Type and Media Codec Type
  if (media_type != AVDT_MEDIA_TYPE_AUDIO ||
      codec_type != A2DP_MEDIA_CT_NON_A2DP) {
    LOG_ERROR( "LHDCV5: invalid media type 0x{%02x} codec_type 0x{%02x}",  media_type, codec_type);
    return AVDT_ERR_UNSUP_CFG;
  }

  // Vendor ID(P0-P3) and Codec ID(P4-P5)
  p_ie->vendorId = (*p_codec_info & 0x000000FF) |
      (*(p_codec_info + 1) << 8 & 0x0000FF00) |
      (*(p_codec_info + 2) << 16 & 0x00FF0000) |
      (*(p_codec_info + 3) << 24 & 0xFF000000);
  p_codec_info += 4;
  p_ie->codecId = (*p_codec_info & 0x00FF) | (*(p_codec_info + 1) << 8 & 0xFF00);
  p_codec_info += 2;
  if (p_ie->vendorId != A2DP_LHDC_VENDOR_ID ||
      p_ie->codecId != A2DP_LHDCV5_CODEC_ID) {
    LOG_ERROR( "LHDCV5: invalid vendorId 0x{%02x} codecId 0x{%02x}",
        p_ie->vendorId, p_ie->codecId);
    return AVDT_ERR_UNSUP_CFG;
  }

  // P6[5:0] Sampling Frequency
  p_ie->sampleRate = (*p_codec_info & A2DP_LHDCV5_SAMPLING_FREQ_MASK);
  if (p_ie->sampleRate == A2DP_LHDCV5_SAMPLING_FREQ_NS) {
    LOG_ERROR( "LHDCV5: invalid sample rate 0x{%02x}",  p_ie->sampleRate);
    return AVDT_ERR_UNSUP_CFG;
  }
  p_codec_info += 1;

  // P7[2:0] Bits Per Sample
  p_ie->bits_per_sample = (*p_codec_info & A2DP_LHDCV5_BIT_FMT_MASK);
  if (p_ie->bits_per_sample == A2DP_LHDCV5_BIT_FMT_NS) {
    LOG_ERROR( "LHDCV5: invalid bit per sample 0x{%02x}",  p_ie->bits_per_sample);
    return AVDT_ERR_UNSUP_CFG;
  }
  // P7[5:4] Max Target Bit Rate
  p_ie->maxTargetBitrate = (*p_codec_info & A2DP_LHDCV5_MAX_BIT_RATE_MASK);
  // P7[7:6] Min Target Bit Rate
  p_ie->minTargetBitrate = (*p_codec_info & A2DP_LHDCV5_MIN_BIT_RATE_MASK);
  p_codec_info += 1;

  // Channel Mode: stereo only
  p_ie->channelMode = A2DP_LHDCV5_CHANNEL_MODE_STEREO;

  // P8[3:0] Codec SubVersion
  p_ie->version = (*p_codec_info & A2DP_LHDCV5_VERSION_MASK);
  if (p_ie->version == A2DP_LHDCV5_VER_NS) {
    LOG_ERROR( "LHDCV5: invalid version 0x{%02x}",  p_ie->version);
    return AVDT_ERR_UNSUP_CFG;
  } else {
    if (!is_codec_version_supported(p_ie->version, is_source)) {
      LOG_ERROR( "LHDCV5: unsupported version 0x{%02x}",  p_ie->version);
      return AVDT_ERR_UNSUP_CFG;
    }
  }
  // P8[5:4] Frame Length Type
  p_ie->frameLenType = (*p_codec_info & A2DP_LHDCV5_FRAME_LEN_MASK);
  if (p_ie->frameLenType == A2DP_LHDCV5_FRAME_LEN_NS) {
    LOG_ERROR( "LHDCV5: invalid frame length mode 0x{%02x}",  p_ie->frameLenType);
    return AVDT_ERR_UNSUP_CFG;
  }
  p_codec_info += 1;

  // Features:
  // P9[0] HasAR
  // P9[1] HasJAS
  // P9[2] HasMeta
  // P9[4] HasLossless96K
  // P9[5] HasLossless24bit
  // P9[6] HasLL
  // P9[7] HasLossless
  p_ie->hasFeatureAR = ((*p_codec_info & A2DP_LHDCV5_FEATURE_AR) != 0) ? true : false;
  p_ie->hasFeatureJAS = ((*p_codec_info & A2DP_LHDCV5_FEATURE_JAS) != 0) ? true : false;
  p_ie->hasFeatureMETA = ((*p_codec_info & A2DP_LHDCV5_FEATURE_META) != 0) ? true : false;
  p_ie->hasFeatureLL = ((*p_codec_info & A2DP_LHDCV5_FEATURE_LL) != 0) ? true : false;
  p_ie->hasFeatureLLESS48K = ((*p_codec_info & A2DP_LHDCV5_FEATURE_LLESS48K) != 0) ? true : false;
  p_ie->hasFeatureLLESS24Bit = ((*p_codec_info & A2DP_LHDCV5_FEATURE_LLESS24BIT) != 0) ? true : false;
  p_ie->hasFeatureLLESS96K = ((*p_codec_info & A2DP_LHDCV5_FEATURE_LLESS96K) != 0) ? true : false;
  p_codec_info += 1;

#ifdef LHDC_LOSSLESS_RAW_SUPPORT
  // P10[7] HasLosslessRaw
  p_ie->hasFeatureLLESSRaw = ((*p_codec_info & A2DP_LHDCV5_FEATURE_LLESS_RAW) != 0) ? true : false;
#endif

  LOG_INFO( "LHDCV5:: codec info parsed = H0-H2:{%02x} {%02x} {%02x} P0-P3:{%02x} "
      "{%02x} {%02x} {%02x} P4-P5:{%02x} {%02x} P6:{%02x} P7:{%02x} P8:{%02x} P9:{%02x} P10:{%02x}", 
      tmpInfo[0], tmpInfo[1], tmpInfo[2], tmpInfo[3], tmpInfo[4], tmpInfo[5], tmpInfo[6], tmpInfo[7],
      tmpInfo[8], tmpInfo[9], tmpInfo[10], tmpInfo[11], tmpInfo[12], tmpInfo[A2DP_LHDCV5_CODEC_LEN]);

  LOG_INFO( "LHDCV5:  isCap{%s} SR{%d} BPS{%d} Ver{%d} FL{%d} "
      "MBR{%d} mBR{%d} FeatureAR({%d}) JAS({%d}) META({%d}) LL({%d}) LLESS({%d}) LLESS24({%d}) LLESS96K({%d}) LLESSRaw({%d})",
      
      (is_source?"SRC":"SNK"),
      is_capability,
      p_ie->sampleRate,
      p_ie->bits_per_sample,
      p_ie->version,
      p_ie->frameLenType,
      p_ie->maxTargetBitrate,
      p_ie->minTargetBitrate,
      p_ie->hasFeatureAR,
      p_ie->hasFeatureJAS,
      p_ie->hasFeatureMETA,
      p_ie->hasFeatureLL,
      p_ie->hasFeatureLLESS48K,
      p_ie->hasFeatureLLESS24Bit,
      p_ie->hasFeatureLLESS96K,
#ifdef LHDC_LOSSLESS_RAW_SUPPORT
      p_ie->hasFeatureLLESSRaw);
#else
      -1);
#endif

  return A2DP_SUCCESS;
}

tA2D_STATUS A2DP_IsVendorPeerSourceCodecValidLhdcv5(const uint8_t* p_codec_info) {
  tA2DP_LHDCV5_CIE cfg_cie;
  /* Use a liberal check when parsing the codec info */
  return (A2DP_ParseInfoLhdcv5(&cfg_cie, p_codec_info, false, true) == A2DP_SUCCESS) ||
      (A2DP_ParseInfoLhdcv5(&cfg_cie, p_codec_info, true, true) == A2DP_SUCCESS);
}

bool A2DP_IsVendorPeerSinkCodecValidLhdcv5(const uint8_t* p_codec_info) {
  tA2DP_LHDCV5_CIE cfg_cie;
  /* Use a liberal check when parsing the codec info */
  return (A2DP_ParseInfoLhdcv5(&cfg_cie, p_codec_info, false, false) == A2DP_SUCCESS) ||
      (A2DP_ParseInfoLhdcv5(&cfg_cie, p_codec_info, true, false) == A2DP_SUCCESS);
}

// Checks whether A2DP LHDC codec configuration matches with a device's codec
// capabilities.
//  |p_cap| is the LHDC local codec capabilities.
//  |p_codec_info| is peer's codec capabilities acting as an A2DP source.
// If |is_capability| is true, the byte sequence is codec capabilities,
// otherwise is codec configuration.
// Returns A2DP_SUCCESS if the codec configuration matches with capabilities,
// otherwise the corresponding A2DP error status code.
tA2DP_STATUS A2DP_CodecInfoMatchesCapabilityLhdcv5(
    const tA2DP_LHDCV5_CIE* p_cap, const uint8_t* p_codec_info, bool is_capability) {
  tA2DP_STATUS status;
  tA2DP_LHDCV5_CIE cfg_cie;

  if (p_cap == NULL || p_codec_info == NULL) {
    LOG_ERROR( "LHDCV5: nullptr input");
    return A2DP_INVALID_PARAMS;
  }

  // parse configuration
  status = A2DP_ParseInfoLhdcv5(&cfg_cie, p_codec_info, is_capability, true); // sink
  if (status != A2DP_SUCCESS) {
    LOG_ERROR( "LHDCV5: parsing failed {%d}",  status);
    return status;
  }

  // verify that each parameter is in range
  LOG_INFO( "LHDCV5: FREQ peer: 0x{%02x}, capability 0x{%02x}",
      cfg_cie.sampleRate, p_cap->sampleRate);

  LOG_INFO( "LHDCV5: BIT_FMT peer: 0x{%02x}, capability 0x{%02x}",
      cfg_cie.bits_per_sample, p_cap->bits_per_sample);

  // sampling frequency
  if ((cfg_cie.sampleRate & p_cap->sampleRate) == 0) return A2DP_INVALID_PARAMS;

  // bits per sample
  if ((cfg_cie.bits_per_sample & p_cap->bits_per_sample) == 0) return A2DP_INVALID_PARAMS;

  return A2DP_SUCCESS;
}

btav_a2dp_codec_index_t A2DP_VendorSinkCodecIndexLhdcv5(const uint8_t* p_codec_info) {
    UNUSED(p_codec_info);
    return BTAV_A2DP_CODEC_INDEX_SINK_LHDCV5;
}

btav_a2dp_codec_index_t A2DP_VendorSourceCodecIndexLhdcv5(const uint8_t* p_codec_info) {
    UNUSED(p_codec_info);
    return BTAV_A2DP_CODEC_INDEX_SOURCE_LHDCV5;
}

const char* A2DP_VendorCodecNameLhdcv5(const uint8_t* p_codec_info) {
    UNUSED(p_codec_info);
    return "LHDC V5";
}

bool A2DP_VendorCodecTypeEqualsLhdcv5(const uint8_t* p_codec_info_a,
    const uint8_t* p_codec_info_b) {
  tA2DP_LHDCV5_CIE lhdc_cie_a;
  tA2DP_LHDCV5_CIE lhdc_cie_b;
  tA2DP_STATUS a2dp_status;

  if (p_codec_info_a == NULL || p_codec_info_b == NULL) {
    LOG_ERROR( "LHDCV5: nullptr input");
    return false;
  }

  // Check whether the codec info contains valid data
  a2dp_status =
      A2DP_ParseInfoLhdcv5(&lhdc_cie_a, p_codec_info_a, true, false); // source
  if (a2dp_status != A2DP_SUCCESS) {
    LOG_ERROR( "LHDCV5: cannot decode codec information: {%d}", 
        a2dp_status);
    return false;
  }
  a2dp_status = A2DP_ParseInfoLhdcv5(&lhdc_cie_b, p_codec_info_b, true, false); // source
  if (a2dp_status != A2DP_SUCCESS) {
    LOG_ERROR( "LHDCV5: cannot decode codec information: {%d}", 
        a2dp_status);
    return false;
  }

  return true;
}

bool A2DP_VendorInitCodecConfigLhdcv5(btav_a2dp_codec_index_t codec_index, UINT8* p_result) {
    switch(codec_index) {
        case BTAV_A2DP_CODEC_INDEX_SINK_LHDCV5:
            return A2DP_VendorInitCodecConfigLhdcv5Sink(p_result);
        default:
            break;
    }
    return false;
}

bool A2DP_VendorInitCodecConfigLhdcv5Sink(UINT8* p_cfg) {
  if (p_cfg == NULL) {
    LOG_ERROR( "LHDCV5: nullptr input");
    return false;
  }

  if (A2DP_BuildInfoLhdcv5(AVDT_MEDIA_TYPE_AUDIO, &a2dp_lhdcv5_sink_caps,
      p_cfg) != A2DP_SUCCESS) {
    return false;
  }

  return true;
}

bool A2DP_VendorBuildCodecConfigLhdcv5(UINT8 *p_src_cap, UINT8 *p_result) {
  tA2DP_LHDCV5_CIE src_cap;
  tA2DP_LHDCV5_CIE pref_cap = a2dp_lhdcv5_default_config;
  tA2D_STATUS status;

  if ((status = A2DP_ParseInfoLhdcv5(&src_cap, p_src_cap, TRUE, true)) != 0) {
    APPL_TRACE_ERROR("%s: Cant parse src cap ret = %d", __func__, status);
    return false;
  }

  if (src_cap.sampleRate & A2DP_LHDCV5_SAMPLING_FREQ_192000) {
    pref_cap.sampleRate = A2DP_LHDCV5_SAMPLING_FREQ_192000;
  } else if (src_cap.sampleRate & A2DP_LHDCV5_SAMPLING_FREQ_96000) {
    pref_cap.sampleRate = A2DP_LHDCV5_SAMPLING_FREQ_96000;
  } else if (src_cap.sampleRate & A2DP_LHDCV5_SAMPLING_FREQ_48000) {
    pref_cap.sampleRate = A2DP_LHDCV5_SAMPLING_FREQ_48000;
  } else if (src_cap.sampleRate & A2DP_LHDCV5_SAMPLING_FREQ_44100) {
    pref_cap.sampleRate = A2DP_LHDCV5_SAMPLING_FREQ_44100;
  } else {
    APPL_TRACE_ERROR("%s: Unsupported sample rate 0x%x", __func__,
                     src_cap.sampleRate);
    return false;
  }

  if (src_cap.channelMode & A2DP_LHDCV5_CHANNEL_MODE_STEREO) {
    pref_cap.channelMode = A2DP_LHDCV5_CHANNEL_MODE_STEREO;
  } else if (src_cap.channelMode & A2DP_LHDCV5_CHANNEL_MODE_DUAL) {
    pref_cap.channelMode = A2DP_LHDCV5_CHANNEL_MODE_DUAL;
  } else if (src_cap.channelMode & A2DP_LHDCV5_CHANNEL_MODE_MONO) {
    pref_cap.channelMode = A2DP_LHDCV5_CHANNEL_MODE_MONO;
  } else {
    APPL_TRACE_ERROR("%s: Unsupported channel mode 0x%x", __func__,
                     src_cap.channelMode);
    return false;
  }

  A2DP_BuildInfoLhdcv5(A2D_MEDIA_TYPE_AUDIO, (tA2DP_LHDCV5_CIE *) &pref_cap, p_result);
  return true;
}

const tA2DP_DECODER_INTERFACE* A2DP_GetVendorDecoderInterfaceLhdcv5(const uint8_t* p_codec_info) {
    if (!A2DP_IsVendorPeerSinkCodecValidLhdcv5(p_codec_info)) return NULL;
    return &a2dp_decoder_interface_lhdcv5;
}

#endif /* defined(LHDCV5_DEC_INCLUDED) && LHDCV5_DEC_INCLUDED == TRUE */