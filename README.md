# extended-a2dp-bluedroid

An extended encoding test of A2DP on Bluedroid，目的是整合已有资源，在sf32/esp32上验证LHDCV5等A2DP拓展编码，目前AAC、aptX[-LL & -HD]、LDAC、LC3 Plus、OPUS已验证可用，还剩下LHDC未移植也未测试。

## btstack_app_sf32

由O2C14创建，提供了适用于sf32的基于btstack蓝牙协议栈的lhdcv5应用，详见：[btstack_app_sf32](https://github.com/O2C14/btstack_app_sf32)。  

关于LHDCV5解码相关的内容是位于：btstack_app_sf32/src/codecs/LHDCV5的lhdcv5_coder.h和lhdcv5_decoder.c，以及btstack_app_sf32/src的a2dp_decoder.c和a2dp_decoder.h；

不过注意，其缺少"lhdc_v5_dec.h"、"lhdc_v5_dec_workspace.h"、"lhdcv5BT_dec.h"这三个文件，咨询过作者，这部分目前不会被开源。

## android_external_lhdc

由mvaisakh创建，提供了"lhdcv5BT_dec.h"和"lhdcv5BT_dec.c"等内容，适用于Android，详见：[android_external_lhdc](https://github.com/mvaisakh/android_external_lhdc)。

关于lhdcv5的内容是android_external_lhdc/liblhdcv5/inc的lhdcv5BT.h；android_external_lhdc/liblhdcv5/include的lhdcv5BT_ext_func.h和lhdcv5_api.h；android_external_lhdc/liblhdcv5/src的lhdcv5BT_enc.c；

关于lhdcv5dev的内容是：android_external_lhdc/liblhdcv5dec/inc的lhdcv5BT_dec.h；android_external_lhdc/liblhdcv5dec/include的lhdcv5_util_dec.h；android_external_lhdc/liblhdcv5dec/src的lhdcv5BT_dec.c；

此外还有liblhdc和liblhdcdec，推测是小于V5的版本，但是在这里保留；

## android_packages_modules_Bluetooth

由web1n创建，提供了lhdc在内的多个编码的完整协议，包括编解码器，适用于Android，详见：[android_packages_modules_Bluetooth](https://github.com/web1n/android_packages_modules_Bluetooth)。

很明显这个包集成了绝大多数A2DP编码，其源文件位于android_packages_modules_Bluetooth/system/stack/a2dp目录，包括LHDCV5和LHDC其他版本以及大多数主流编码的编码器和解码器的源文件；目录android_packages_modules_Bluetooth/system/stack/include则是对应这些编解码器的头文件。

## bluez-alsa

由arkq创建，也提供了多种编码，详见[bluez-alsa](https://github.com/arkq/bluez-alsa)。

该项目主要适用于Linux环境。bluez-alsa/src目录便是这些拓展编码的源文件及头文件。

## esp-idf修改版

由cfint创建，修改了esp-idf 5.1.4的bt部分，集成了大多数主流A2DP解码器，目前是没有LHDC，详见[esp-idf](https://github.com/cfint/esp-idf/tree/v5.1.4-a2dp-codecs)。esp-idf实现A2DP拓展编码的方式是Bluedroid。

值得注意的是，由于cfint的杰出贡献，使得esp32不仅仅支持SBC，还支持AAC、aptX[-LL & -HD]、LDAC、LC3 Plus、OPUS；这些编码我都已经在esp32上成功验证了。  

esp-idf/components/bt/host/bluedroid/stack/a2dp目录：是包括LDAC在内的各种拓展编码的源文件；

esp-idf/components/bt/host/bluedroid/stack/include/stack目录：是包括LDAC在内的各种拓展编码的头文件；

esp-idf/components/bt/host/bluedroid/external目录：是包括LDAC在内的各种拓展编码的external库；

相较于原生esp-idf，cfint修改版esp-idf的具体改动点如下(以LDAC为例，完整展示了目录及文件结构，可以看见内容是挺多的)：

- esp-idf/components/bt目录：CMakeLists.txt：增加了各种编码的条件编译，如“if(CONFIG_BT_A2DP_LDAC_DECODER) ... endif()”，目的是当条件满足时包含对应的头文件目录和对应的源文件；注意该目录下的Kconfig未改变，和原生IDF内容一致；
- esp-idf/components/bt/host/bluedroid目录：Kconfig.in：开启了各种编码的menuconfig选项，如“config BT_A2DP_LDAC_DECODER ...”，可在menuconfig中控制是否启用该编码；
- esp-idf/components/bt/host/bluedroid/stack/a2dp目录：a2dp_vendor_ldac_decoder.c：ldac解码器的源文件（主要文件），这个文件引入了ldacdec.h这个external库；整个文件由宏定义LDAC_DEC_INCLUDED是否为TRUE控制内容是否启用；启用的内容包括tA2DP_LDAC_DECODER_CB结构体的定义和声明、ldac解码器初始化函数、packet_header函数、寻找同步word函数、packet函数、配置函数；
- esp-idf/components/bt/host/bluedroid/stack/a2dp目录：a2dp_vendor_ldac.c：也是ldac解码器源文件，整个文件由宏定义LDAC_DEC_INCLUDED是否为TRUE控制内容是否启用；启用的内容包括LDAC Source codec capabilities（tA2DP_LDAC_CIE a2dp_ldac_source_caps，包括厂商ID、编码ID、采样率、通道模式、位深）、LDAC Sink codec capabilities（tA2DP_LDAC_CIE a2dp_ldac_sink_caps）等更多内容，无法一一提及，需要阅读整个文件；
- esp-idf/components/bt/host/bluedroid/stack/a2dp目录：a2dp_vendor_ldacbt_decoder.c：也是ldac解码器源文件，这个文件引入了ldacBT.h这个external库；整个文件由宏定义LDAC_DEC_INCLUDED是否为TRUE控制内容是否启用；内容类似a2dp_vendor_ldac_decoder.c；
- esp-idf/components/bt/host/bluedroid/stack/a2dp目录：a2dp_vendor.c：包含"stack/a2dp_vendor_ldac.h"在内的各个解码器的头文件；由宏定义LDAC_DEC_INCLUDED是否为TRUE控制厂商ID、编码ID是否采用LDAC的值，以及返回对应的CIE，其余编码也有类似的操作；还有更多内容，无法一一提及，需要阅读整个文件；
- esp-idf/components/bt/host/bluedroid/stack/a2dp/include目录：bt_av.h：由宏定义LDAC_DEC_INCLUDED控制BTAV_A2DP_CODEC_INDEX_SOURCE_LDAC和BTAV_A2DP_CODEC_INDEX_SINK_LDAC的定义；
- esp-idf/components/bt/host/bluedroid/stack/include/stack目录：a2dp_vendor_ldac_constants.h：包括LDAC Quality Mode Index、ABR mode质量定义、Length of the LDAC Media Payload header、LDAC Media Payload Header、LDAC codec specific settings、厂商ID、编码ID、采样率、通道模式的定义；
- esp-idf/components/bt/host/bluedroid/stack/include/stack目录：a2dp_vendor_ldac_decoder.h：ldac解码器的主要头文件，对外声明的函数包括：a2dp_ldac_decoder_init、a2dp_ldac_decoder_cleanup、a2dp_ldac_decoder_decode_packet_header、a2dp_ldac_decoder_decode_packet、a2dp_ldac_decoder_configure；
- esp-idf/components/bt/host/bluedroid/stack/include/stack目录：a2dp_vendor_ldac.h：此文件涉及大量ldac函数声明，不一一列举，详见文件本身；
- esp-idf/components/bt/host/bluedroid/external目录：是包括LDAC在内的各种拓展编码的external库；
- esp-idf/components/bt/host/bluedroid/api/include/api目录：esp_a2dp_api.h：定义了esp_a2d_mcc_t结构体（A2DP media codec capabilities union），包括LDAC在内的各种编码的cie联合体，用途是可以告知本设备支持哪些编码器，也可据此判断当前是什么编码;
- esp-idf/components/bt/host/bluedroid/btc/profile/std/a2dp/include目录：bt_av_co.h：当BT_AV_INCLUDE为TRUE时，增加了对LDAC_DEC_INCLUDE是否定义为TRUE的判断，然后会据此控制启用一些BTC_SV_AV_AA_XXX_INDEX或者BTC_SV_AV_AA_XXX_SINK_INDEX枚举，涉及多个编码；
- esp-idf/components/bt/host/bluedroid/common/include/common目录：bluedroid_user_config.h：增加了条件判断，如“#ifdef CONFIG_BT_A2DP_LDAC_DECODER #define UC_BT_A2DP_LDAC_DECODER_ENABLED    CONFIG_BT_A2DP_LDAC_DECODER #define UC_BT_A2DP_LDAC_DECODER_ENABLED    FALSE #endif”，用来控制相关的解码器是否启用；
- esp-idf/components/bt/host/bluedroid/common/include/common目录：bt_target.h：当经典蓝牙启用时，判断对应的解码器是否有启用的宏定义（bluedroid_user_config.h中的宏定义），然后据此来定义LDAC_DEC_INCLUDE是否为TRUE；然后还有依据CONFIG_BT_A2DP_LDAC_DECODER是否定义来定义AVDT_LDAC_SEPS的值，进而定义AVDT_NUM_SEPS的值；

## Plan

目前的计划是参考btstack_app_sf32、android_external_lhdc、android_packages_modules_Bluetooth、bluez-alsa库的LHDCV5编码相关内容，依据cfint的esp-idf修改版的蓝牙部分中A2DP拓展方式（上述内容都已在本库中已集成），为这个修改版esp-idf拓展LHDCV5编码，初步在esp32上依靠修改的esp-idf实现LHDC编码。

目前还处于试验阶段，欢迎各位朋友一同测试验证。
