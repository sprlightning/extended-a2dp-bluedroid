# liblhdcv5-test

An extended encoding test of A2DP on Bluedroid，目的是整合已有资源，在sf32/esp32上验证LHDCV5等A2DP拓展编码，目前AAC、aptX[-LL & -HD]、LDAC、LC3 Plus、OPUS已验证可用，还剩下LHDC未测试。

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

值得注意的是，由于cfint的杰出贡献，使得esp32不仅仅支持SBC，还支持AAC、aptX[-LL & -HD]、LDAC、LC3 Plus、OPUS；这些编码我都已经在esp32上成功验证了。相较于原生esp-idf，其改动点如下：

- esp-idf/components/bt目录：CMakeLists.txt：增加了各种编码的条件编译，如“if(CONFIG_BT_A2DP_XXX_DECODER) ... endif()”；注意该目录下的Kconfig未改变，和原生IDF内容一致；
- esp-idf/components/bt/host/bluedroid目录：Kconfig.in：开启了各种编码的menuconfig选项，如“config BT_A2DP_XXX_DECODER”，可在menuconfig中控制是否启用该编码；
- esp-idf/components/bt/host/bluedroid/stack/a2dp目录：是包括LDAC在内的各种拓展编码的源文件；
- esp-idf/components/bt/host/bluedroid/stack/include/stack目录：是包括LDAC在内的各种拓展编码的头文件；
- esp-idf/components/bt/host/bluedroid/external目录：是包括LDAC在内的各种拓展编码的external库；
- esp-idf/components/bt/host/bluedroid/api/include/api目录：esp_a2dp_api.h：增加了esp_a2d_mcc_t结构体（A2DP media codec capabilities union）;
- esp-idf/components/bt/host/bluedroid/btc/profile/std/a2dp/include目录：bt_av_co.h：增加了XXX_DEC_INCLUDE定义；
- esp-idf/components/bt/host/bluedroid/common/include/common目录：bluedroid_user_config.h：增加了条件判断，如“ifdef CONFIG_BT_A2DP_XXX_DECODER define ... else ... endif”；
- esp-idf/components/bt/host/bluedroid/common/include/common目录：bt_target.h：
... 未完待续

## Plan

目前的计划是参考btstack_app_sf32、android_external_lhdc、android_packages_modules_Bluetooth、bluez-alsa库的LHDCV5编码相关内容，依据cfint的esp-idf修改版的蓝牙部分中A2DP拓展方式（上述内容都已在本库中已集成），为这个修改版esp-idf拓展LHDCV5编码，初步在esp32上依靠修改的esp-idf实现LHDC编码。

目前还处于试验阶段，欢迎各位朋友一同测试验证。
