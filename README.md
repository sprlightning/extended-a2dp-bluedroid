# extended-a2dp-bluedroid

An extended encoding test of A2DP on Bluedroid，目的是整合已有资源，在sf32/esp32上验证LHDCV5等A2DP拓展编码，目前AAC、aptX[-LL & -HD]、LDAC、LC3 Plus、OPUS已验证可用；

LHDCV5相关内容已移植，但缺乏关键文件lhdcv5_util_dec.c，目前只是根据现有内容写出了lhdcv5_util_dec.c的框架，正在逐步分析，试图写出lhdcv5_util_dec.c的完整内容；

不过分析后发现，Savitech不太可能会随意给出核心解码的源文件，而自己写出又非常困难（不排除有非官方的高人能实现），解决问题的另一个思路是获得Savitech分发的lhdcv5_util_dec.so动态库，又或者是获得lhdcv5BT_dec.so，直接一步到位:)

抛开LHDC V5编码核心文件的问题，现在有个新的问题，就是蓝牙连接后会自动断开，详见下方最新的日志；

## btstack_app_sf32

由O2C14创建，提供了适用于sf32的基于btstack蓝牙协议栈的lhdcv5应用，详见：[btstack_app_sf32](https://github.com/O2C14/btstack_app_sf32)。  

关于LHDCV5解码相关的内容是位于：btstack_app_sf32/src/codecs/LHDCV5的lhdcv5_coder.h和lhdcv5_decoder.c，以及btstack_app_sf32/src的a2dp_decoder.c和a2dp_decoder.h；

不过注意，其缺少"lhdc_v5_dec.h"、"lhdc_v5_dec_workspace.h"、"lhdcv5BT_dec.h"这三个文件，咨询过作者，这部分暂时不会被开源。

## android_external_lhdc

由mvaisakh创建，提供了"lhdcv5BT_dec.h"和"lhdcv5BT_dec.c"等内容，适用于Android，详见：[android_external_lhdc](https://github.com/mvaisakh/android_external_lhdc)。

关于lhdcv5的内容是android_external_lhdc/liblhdcv5/inc的lhdcv5BT.h；android_external_lhdc/liblhdcv5/include的lhdcv5BT_ext_func.h和lhdcv5_api.h；android_external_lhdc/liblhdcv5/src的lhdcv5BT_enc.c；

关于lhdcv5dev的内容是：android_external_lhdc/liblhdcv5dec/inc的lhdcv5BT_dec.h；android_external_lhdc/liblhdcv5dec/include的lhdcv5_util_dec.h；android_external_lhdc/liblhdcv5dec/src的lhdcv5BT_dec.c；

此外还有liblhdc和liblhdcdec，推测是小于V5的版本，但是在这里保留；

## android_packages_modules_Bluetooth

由web1n创建，提供了lhdc在内的多个编码的完整协议，包括编解码器，适用于Android，相对标准化，详见：[android_packages_modules_Bluetooth](https://github.com/web1n/android_packages_modules_Bluetooth)。

很明显这个包集成了绝大多数A2DP编码，其源文件位于android_packages_modules_Bluetooth/system/stack/a2dp目录，包括LHDCV5和LHDC其他版本以及大多数主流编码的编码器和解码器的源文件；目录android_packages_modules_Bluetooth/system/stack/include则是对应这些编解码器的头文件。

## bluez-alsa

由arkq创建，也提供了多种编码，详见[bluez-alsa](https://github.com/arkq/bluez-alsa)。

该项目主要适用于Linux环境。bluez-alsa/src目录便是这些拓展编码的源文件及头文件。

## HP-H300BT

Fanjianghua的HP-H300BT也具有lhdcv5的解码功能，详见[HP-H300BT](https://github.com/Fanjianghua/HP-H300BT)；

该项目本质上是“BES2600IHC TWS耳机”，属于ARM架构；其LHDC编码文件位于：HP-H300BT/apps/audioplayers/a2dp_decoder目录：如a2dp_decoder_lhdc.cpp、a2dp_decoder_lhdcv5.cpp等；

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

## 250916-My-Plan

目前的计划是参考android_external_lhdc、android_packages_modules_Bluetooth、btstack_app_sf32、bluez-alsa库的LHDCV5编码相关内容，依据cfint的esp-idf修改版的蓝牙部分中A2DP拓展方式（上述内容都已在本库中已集成），为这个修改版esp-idf拓展LHDCV5编码，初步在esp32上依靠修改的esp-idf实现LHDC编码。

分析如下，其中关于esp-idf的A2DP拓展详见本章节的**esp-idf的A2DP拓展**

### android_packages_modules_Bluetooth

适用于Android环境，毫无疑问其中的LHDC是比较正式且相对完整的，据有很大的参考价值，不过要注意移植；

比较有趣的是，esp-idf的Bluedroid目录结构与Android高度相似，甚至命名方式也高度相似；

都可以在stack/a2dp目录找到各种编码的源码，包括解码器（Android还有编码器），以及a2dp_vendor等文件；

都可以在stack/a2dp/include目录找到各种编码的头文件，包括解码器（Android还有编码器），以及各种编码的constants宏定义文件，以及a2dp_vendor文件；

从cfint的修改版esp-idf中现存的各种编码，以及O2C14的btstack_app_sf32中的部分编码来看，都有Android的constants库文件的影子，显然从Android移植各种A2DP编码是最靠谱的；

以lhdc为例，android_packages_modules_Bluetooth提供了lhdcv5和lhdcv3两个版本的编解码器源文件及对应的头文件，我们这里只关注lhdcv5的解码器即可，待成功后再考虑lhdcv3的解码器；

lhdcv5解码器源文件位于android_packages_modules_Bluetooth/system/stack/a2dp目录，包括a2dp_vendor_lhdcv5.cc，和a2dp_vendor_lhdcv5_decoder.cc，esp-idf中也采用一致的命名；

lhdcv5解码器头文件位于android_packages_modules_Bluetooth/system/stack/include目录，包括a2dp_vendor_lhdcv5.h和a2dp_vendor_lhdcv5_decoder.h，以及多个版本公用的宏定义a2dp_vendor_lhdc_constants.h，以及lhdcv5专用的宏定义a2dp_vendor_lhdcv5_constants.h；

总之这个Android包，具有很大的参考价值，移植时应当优先考虑；

### bluez-alsa

这是Linux方面的A2DP库，其中的lhdc看起来不是最新版。不过也可以用参考，毕竟有些逻辑也是通用的，如果Android那边找不到可以来这看看。

### android_external_lhdc

也是适用于Android/Linux环境，相当于是一个LHDC拓展库，也据有很大的参考价值。

其中liblhdcv5dec目录的lhdcv5BT_dec.c/.h显然是lhdcv5解码器拓展库，其在很多LHDCV5应用中都被调用过。

此外还有liblhdcv5目录lhdcv5BT_enc.c/.h显然是lhdcv5编码器拓展库；

此外还有liblhdcdec和liblhdc，显然是其它lhdc版本的编解码器拓展库；

考虑到众多案例都有使用这个库的情况，这个库应该具有很大的参考价值，作为拓展库，一般情况移植只需要更换log输出函数即可；

### btstack_app_sf32O2C14

其中的LHDC实现基于btstack蓝牙协议栈，由于他只开源了部分LHDCV5代码，缺乏核心功能，不能直接用，但也可用于参考；

a2dp_decoder.c/.h，这看起来是管理各种解码器，从各种解码器解析出采样率、位深、通道等配置的文件；

lhdcv5_decoder.c很明显相当于Android的a2dp_vendor_lhdcv5.cc，是配置lhdc解码器的部分函数。不过lhdcv5_decoder.c借助了一些未公开的外部库；

lhdcv5_coder.h同时包含了两部分：lhdc统一的宏定义（相当于Android的a2dp_vendor_lhdc_constants.h）、lhdc解码器配置函数的声明和结构体定义（相当于Android的a2dp_vendor_lhdcv5.h）；

未公开的lhdc_v5_dec.h和lhdc_v5_workspace.h，极有起到了类似Android中a2dp_vendor_lhdcv5_decoder.h的功能；

未公开的lhdcv5BT_dec.h则极有可能是Android环境lhdcv5BT_dec.h的移植版本（如mvaisakh的android_external_lhdc）；

### esp-idf的A2DP拓展

这个修改版esp-idf使用的是Bluedroid蓝牙协议栈，由于是我用于测试的环境，这里要重点分析，这个分析逻辑理论上**可以在获得解码器函数的前提下为esp-idf无限制地拓展任意A2DP编码**。

#### 核心文件移植

esp-idf中现存的A2DP编码也具有很高的参考价值，比如可以观察实现这些编码需要哪些特定的函数，从而根据需要从Android代码包中移植过来； 

为esp-idf拓展LHDCV5编码时，务必以esp-idf中Bluedroid-A2DP的编码拓展方式为准，毕竟要在这个框架下运行，统一的函数命名风格和编写方式是很有必要的；

移植的来源优先考虑Android包，毕竟它是最完整且最规范的；

在esp-idf中统一管理多个编码的文件是a2dp_vendor.c/.h，基本每个编码都会用到至少10个函数（由每个编码各自的宏定义控制），他们通常是在a2dp_vendor中返回的时候调用，实现不同的功能，详见a2dp_vendor.h：

如LDAC会用到return A2DP_ParseInfoLdac((tA2DP_LDAC_CIE*)p_ie, p_codec_info, is_capability);那这个函数就是A2DP_ParseInfoLdac(xxx, xxx, xxx); 它应当放到a2dp_vendor_ldac.c/.h；

因此我类比LDAC给出了LHDCV5需要的这10个函数，其位于a2dp_vendor_lhdcv5.h的声明应当如下(函数名已经统一规范，但是参数要核对，功能要依据Android那边移植进来，或参考现有编码从头编写)：
```c
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
```

参考esp-idf中现存的A2DP编码，以下几个lhdc文件是必须要编写出来的，命名如下：
- lhdc对外主要文件，直接与a2dp_vendor进行交互："a2dp_vendor_lhdcv5.h"、"a2dp_vendor_lhdcv5.c，需要移植；
- lhdc解码器相关文件："a2dp_vendor_lhdcv5_decoder.h"、"a2dp_vendor_lhdcv5_decoder.c"，需要移植；
- lhdc宏定义文件："a2dp_vendor_lhdcv5_constants.h"，"a2dp_vendor_lhdc_constants.h"，这种宏定义直接复制Android的就行，无需任何改动；
	值得注意的是，"a2dp_vendor_lhdc_constants.h"更像是传统的宏定义，而"a2dp_vendor_lhdcv5_constants.h"还增加了些新的东西，如额外定义了192k采样率等内容，所以这两个都要保留；

上述文件源文件应当存放到esp-idf-test1/components/bt/host/bluedroid/stack/a2dp目录，头文件应当存放到esp-idf-test1/components/bt/host/bluedroid/stack/a2dp/include/stack目录；

#### lhdcv5拓展库

前面提到众多LHDC案例都“习惯性”使用android_external_lhdc库的内容，一定是有他的存在价值的，所以这里也将其作为拓展库，按照下面的方式配置；

要启用拓展库，则是将android_external_lhdc中的lhdcv5dec的内容放到components/bt/host/bluedroid/external/liblhdcv5dec目录，并按照inc、include、src进行分类，并编写CMakeLists.txt：

- inc：公共头文件；
- include：私有头文件，其实就是“lhdcv5_util_dec.h”；
- src：所有的源文件；

我这里为拓展库做了份CMakeLists.txt，内容如下：
```c
cmake_minimum_required(VERSION 3.15)

idf_component_register(SRCS src/lhdcv5_util_dec.c
							src/lhdcv5BT_dec.c
                       INCLUDE_DIRS inc
                       PRIV_INCLUDE_DIRS inc include src)

target_compile_options(${COMPONENT_LIB} PRIVATE -Werror=implicit-function-declaration)
```

拓展库移植起来应该很容易，主要是修改log函数，用esp-idf的ESP_LOG?+TAG对应代替Android的ALOG?即可；

#### esp-idf修改现有文件  

这是最主要的步骤，目的是把引入的lhdcv5编码文件接入到esp-idf Bluedroid A2DP环境；

- **修改 components/bt/CMakeLists.txt：**  
	添加 lhdcv5 相关文件编译，风格尽量与其余编码保持一致，根据实际所用的文件添加；
	这里可以参考LDAC的，LDAC如下：
	```c
	if(CONFIG_BT_A2DP_LDAC_DECODER)
		list(APPEND priv_include_dirs host/bluedroid/external/libldac-dec/src
									  host/bluedroid/external/libldac-dec/inc)
		list(APPEND ldacbt_dec_srcs "host/bluedroid/external/libldac-dec/src/ldacBT.c"
									"host/bluedroid/external/libldac-dec/src/ldaclib.c")
		list(APPEND srcs ${ldacbt_dec_srcs})

		list(APPEND srcs "host/bluedroid/stack/a2dp/a2dp_vendor_ldac.c"
						 "host/bluedroid/stack/a2dp/a2dp_vendor_ldacbt_decoder.c")
	endif()
	```
	那么LHDCV5则应当如下（这里是使用了external库liblhdcv5dec的情况）：
	```c
	if(CONFIG_BT_A2DP_LHDCV5_DECODER)
		list(APPEND priv_include_dirs host/bluedroid/external/liblhdcv5dec/inc
									  host/bluedroid/external/liblhdcv5dec/include)
		list(APPEND lhdcv5bt_dec_srcs "host/bluedroid/external/liblhdcv5dec/src/lhdcv5BT_dec.c"
									"host/bluedroid/external/liblhdcv5dec/src/lhdcv5_util_dec.c")
		list(APPEND srcs ${lhdcv5bt_dec_srcs})

		list(APPEND srcs "host/bluedroid/stack/a2dp/a2dp_vendor_lhdcv5.c"
						 "host/bluedroid/stack/a2dp/a2dp_vendor_lhdcv5_decoder.c")
	endif()
	```

- **修改 components/bt/host/bluedroid/Kconfig.in：**  
	添加 menuconfig 选项，风格也是尽量与其余编码保持一致，便于识别；
	以LDAC为例，其选项是：
	```c
	config BT_A2DP_LDAC_DECODER
		bool "LDAC decoder"
		depends on BT_A2DP_ENABLE
		default n
		help
			A2DP LDAC decoder
	```
	那么LHDCV5选项是：
	```c
	config BT_A2DP_LHDCV5_DECODER
		bool "LHDCV5 decoder"
		depends on BT_A2DP_ENABLE
		default n
		help
			A2DP LHDCV5 decoder
	```

- **修改 components/bt/host/bluedroid/stack/a2dp/a2dp_vendor.c：**  
	添加 lhdcv5 支持，添加一个头文件“a2dp_vendor_lhdcv5.h”，和用宏定义包起来的十对有特定名称和功能的函数，风格自然也是与其余编码保持一致；
	以LDAC为例，其ParseInfo如下：
	```c
	#if (defined(LDAC_DEC_INCLUDED) && LDAC_DEC_INCLUDED == TRUE)
	  // Check for LDAC
	  if (vendor_id == A2DP_LDAC_VENDOR_ID &&
		  codec_id == A2DP_LDAC_CODEC_ID) {
		return A2DP_ParseInfoLdac((tA2DP_LDAC_CIE*)p_ie, p_codec_info, is_capability);
	  }
	#endif /* defined(LDAC_DEC_INCLUDED) && LDAC_DEC_INCLUDED == TRUE) */
	```
	那么对应的LHDCV5函数就是：
	```c
	#if (defined(LHDCV5_DEC_INCLUDED) && LHDCV5_DEC_INCLUDED == TRUE)
	  // Check for LHDCV5
	  if (vendor_id == A2DP_LHDC_VENDOR_ID &&
		  codec_id == A2DP_LHDCV5_CODEC_ID) {
		return A2DP_ParseInfoLhdcv5((tA2DP_LHDCV5_CIE*)p_ie, p_codec_info, is_capability);
	  }
	#endif /* defined(LHDCV5_DEC_INCLUDED) && LHDCV5_DEC_INCLUDED == TRUE) */
	```

- **修改 components/bt/host/bluedroid/stack/a2dp/include/bt_av.h：**  
	添加 lhdcv5 编码索引，都是用宏定义包含的，具体是：BTAV_A2DP_CODEC_INDEX_SOURCE_LHDCV5和BTAV_A2DP_CODEC_INDEX_SINK_LHDCV5；
	以LDAC为例是下面两个：
	```c
	#if (defined(LDAC_DEC_INCLUDED) && LDAC_DEC_INCLUDED == TRUE)
	  BTAV_A2DP_CODEC_INDEX_SOURCE_LDAC,
	#endif /* LDAC_DEC_INCLUDED */
	
	#if (defined(LDAC_DEC_INCLUDED) && LDAC_DEC_INCLUDED == TRUE)
	  BTAV_A2DP_CODEC_INDEX_SINK_LDAC,
	#endif /* LDAC_DEC_INCLUDED */
	```
	同理LHDCV5是下面两个：
	```c
	#if (defined(LHDCV5_DEC_INCLUDED) && LHDCV5_DEC_INCLUDED == TRUE)
	  BTAV_A2DP_CODEC_INDEX_SOURCE_LHDCV5,
	#endif /* LHDCV5_DEC_INCLUDED */
	
	#if (defined(LHDCV5_DEC_INCLUDED) && LHDCV5_DEC_INCLUDED == TRUE)
	  BTAV_A2DP_CODEC_INDEX_SINK_LHDCV5,
	#endif /* LHDCV5_DEC_INCLUDED */
	```

- **修改 components/bt/host/bluedroid/common/include/common/bluedroid_user_config.h：**  
	增加宏定义配置，这个务必要统一命名风格，以LDAC为例是：
	```c
	#ifdef CONFIG_BT_A2DP_LDAC_DECODER
	#define UC_BT_A2DP_LDAC_DECODER_ENABLED    CONFIG_BT_A2DP_LDAC_DECODER
	#else
	#define UC_BT_A2DP_LDAC_DECODER_ENABLED    FALSE
	#endif
	```
	那么LHDCV5必须是下面的内容：
	```c
	#ifdef CONFIG_BT_A2DP_LHDCV5_DECODER
	#define UC_BT_A2DP_LHDCV5_DECODER_ENABLED    CONFIG_BT_A2DP_LHDCV5_DECODER
	#else
	#define UC_BT_A2DP_LHDCV5_DECODER_ENABLED    FALSE
	#endif
	```

- **修改 components/bt/host/bluedroid/common/include/common/bt_target.h：**
	增加宏定义配置，并配置AVDT_NUM_SEPS；  
	以LDAC为例，其宏定义如下，会应用到绝大多数文件中：
	```c
	#if (UC_BT_A2DP_LDAC_DECODER_ENABLED == TRUE)
	#define LDAC_DEC_INCLUDED           TRUE
	#endif /* (UC_BT_A2DP_LDAC_DECODER_ENABLED == TRUE) */
	```
	那么LHDCV5的宏定义必须是下面的内容：
	```c
	#if (UC_BT_A2DP_LHDCV5_DECODER_ENABLED == TRUE)
	#define LHDCV5_DEC_INCLUDED           TRUE
	#endif /* (UC_BT_A2DP_LHDCV5_DECODER_ENABLED == TRUE) */
	```

- **修改 components/bt/host/bluedroid/api/include/api/esp_a2dp_api.h：**
	添加 LHDCV5 到联合体，这个需要修改一下结构体__attribute__((packed)) esp_a2d_mcc_t；
	这段源文件内容如下：
	```c
	/**
	 * @brief A2DP media codec capabilities union
	 */
	typedef struct {
		esp_a2d_mct_t type;                        /*!< A2DP media codec type */
	#define ESP_A2D_CIE_LEN_SBC          (4)
	#define ESP_A2D_CIE_LEN_M12          (4)
	#define ESP_A2D_CIE_LEN_M24          (10)
	#define ESP_A2D_CIE_LEN_ATRAC        (7)
	#define ESP_A2D_CIE_LEN_APTX         (7)
	#define ESP_A2D_CIE_LEN_APTX_HD      (11)
	#define ESP_A2D_CIE_LEN_APTX_LL      (7)
	#define ESP_A2D_CIE_LEN_LDAC         (8)
	#define ESP_A2D_CIE_LEN_OPUS         (26)
	#define ESP_A2D_CIE_LEN_LC3PLUS         (12)
		union {
			uint8_t sbc[ESP_A2D_CIE_LEN_SBC];      /*!< SBC codec capabilities */
			uint8_t m12[ESP_A2D_CIE_LEN_M12];      /*!< MPEG-1,2 audio codec capabilities */
			uint8_t m24[ESP_A2D_CIE_LEN_M24];      /*!< MPEG-2, 4 AAC audio codec capabilities */
			uint8_t atrac[ESP_A2D_CIE_LEN_ATRAC];  /*!< ATRAC family codec capabilities */
			uint8_t aptx[ESP_A2D_CIE_LEN_APTX];    /*!< APTX codec capabilities */
			uint8_t aptx_hd[ESP_A2D_CIE_LEN_APTX_HD];    /*!< APTX-HD codec capabilities */
			uint8_t aptx_ll[ESP_A2D_CIE_LEN_APTX_LL];    /*!< APTX-LL codec capabilities */
			uint8_t ldac[ESP_A2D_CIE_LEN_LDAC];    /*!< LDAC codec capabilities */
			uint8_t opus[ESP_A2D_CIE_LEN_OPUS];    /*!< OPUS codec capabilities */
			uint8_t lc3plus[ESP_A2D_CIE_LEN_LC3PLUS];    /*!< LC3 Plus codec capabilities */
		} cie;                                     /*!< A2DP codec information element */
	} __attribute__((packed)) esp_a2d_mcc_t;
	```
	这玩意有啥用？它很重要，它通过同一地址的cie联合体存储编码信息，通常会在不同位存储当前编码的厂商ID、编码ID、采样率、位深、声道等信息，读写这个联合体就能传播编码信息了；
	而这个联合体的长度就是在这里定义的，我观察了大多数编码的CIE_LEN，通常是等于CODEC_LEN - 2，而CODEC_LEN通常是由对应编码的constants.h文件提供；
	以LDAC为例，其宏定义文件a2dp_vendor_ldac_constants.h定义的CODEC_LEN如下：
	```c
	// LDAC codec specific settings
	#define A2DP_LDAC_CODEC_LEN 10
	```
	A2DP_LDAC_CODEC_LEN - 2就是9，刚好等于ESP_A2D_CIE_LEN_LDAC；
	a2dp_vendor_aptx_constants.h中A2DP_APTX_CODEC_LEN是9，那A2DP_APTX_CODEC_LEN - 2就是7，刚好等于ESP_A2D_CIE_LEN_APTX；
	查询Android包里的a2dp_vendor_lhdc_constants.h可知A2DP_LHDCV5_CODEC_LEN是13，那么ESP_A2D_CIE_LEN_LHDCV5 = A2DP_LHDCV5_CODEC_LEN - 2 = 11；
	所以这个结构体可修改为：
	```c
	/**
	 * @brief A2DP media codec capabilities union
	 * 大部分CIE_LEN = CODEC_LEN - 2
	 */
	typedef struct {
		esp_a2d_mct_t type;                        /*!< A2DP media codec type */
	#define ESP_A2D_CIE_LEN_SBC          (4)
	#define ESP_A2D_CIE_LEN_M12          (4)
	#define ESP_A2D_CIE_LEN_M24          (10)
	#define ESP_A2D_CIE_LEN_ATRAC        (7)
	#define ESP_A2D_CIE_LEN_APTX         (7)
	#define ESP_A2D_CIE_LEN_APTX_HD      (11)
	#define ESP_A2D_CIE_LEN_APTX_LL      (7)
	#define ESP_A2D_CIE_LEN_LDAC         (8)
	#define ESP_A2D_CIE_LEN_OPUS         (26)
	#define ESP_A2D_CIE_LEN_LC3PLUS      (12)
	#define ESP_A2D_CIE_LEN_LHDCV5       (11)
		union {
			uint8_t sbc[ESP_A2D_CIE_LEN_SBC];      /*!< SBC codec capabilities */
			uint8_t m12[ESP_A2D_CIE_LEN_M12];      /*!< MPEG-1,2 audio codec capabilities */
			uint8_t m24[ESP_A2D_CIE_LEN_M24];      /*!< MPEG-2, 4 AAC audio codec capabilities */
			uint8_t atrac[ESP_A2D_CIE_LEN_ATRAC];  /*!< ATRAC family codec capabilities */
			uint8_t aptx[ESP_A2D_CIE_LEN_APTX];    /*!< APTX codec capabilities */
			uint8_t aptx_hd[ESP_A2D_CIE_LEN_APTX_HD];    /*!< APTX-HD codec capabilities */
			uint8_t aptx_ll[ESP_A2D_CIE_LEN_APTX_LL];    /*!< APTX-LL codec capabilities */
			uint8_t ldac[ESP_A2D_CIE_LEN_LDAC];    /*!< LDAC codec capabilities */
			uint8_t opus[ESP_A2D_CIE_LEN_OPUS];    /*!< OPUS codec capabilities */
			uint8_t lc3plus[ESP_A2D_CIE_LEN_LC3PLUS];    /*!< LC3 Plus codec capabilities */
			uint8_t lhdcv5[ESP_A2D_CIE_LEN_LHDCV5];    /*!< LHDCV5 codec capabilities */
		} cie;                                     /*!< A2DP codec information element */
	} __attribute__((packed)) esp_a2d_mcc_t;
	```
	
esp-idf现存文件要修改的就是上面这些，本质上是依葫芦画瓢，没什么难度；  

难的是上面说的从Android到esp-idf Bluedroid的LHDCV5的解码器相关内容的移植，既要考虑esp-idf的需求（a2dp_vendor.c明确提出了10个函数），同时要考虑Android中LHDCV5的完整逻辑；

欢迎感兴趣的朋友一同测试交流经验；

## 250917-Migrate Files

### 发现新内容

- **Fanjianghua的HP-H300BT**

	注意到Fanjianghua的HP-H300BT也具有lhdcv5的解码功能，其本质上是“BES2600IHC TWS耳机”的项目，详见[HP-H300BT](https://github.com/Fanjianghua/HP-H300BT)；

	我已将HP-H300BT集成到本仓库中，用于分析；BES2600IHC是ARM架构，其LHDC编码文件位于：HP-H300BT/apps/audioplayers/a2dp_decoder目录：如a2dp_decoder_lhdc.cpp、a2dp_decoder_lhdcv5.cpp等；

	所以HP-H300BT也有一定的参考价值；

- **Savitech LHDC Codec for AOSP**

	此外我注意到Savitech一直是开源了其LHDC系列编码，详见[Savitech LHDC Codec for AOSP](https://gitlab.com/savitech-lhdc)，可惜他们仍然未开源其核心组件如lhdcv5_util_dec等内容；

	Savitech LHDC最新版本是15.0.0r4，详见[Savitech LHDC Codec AOSP 15.0.0r4](https://gitlab.com/savitech-lhdc/savitech-lhdc-codec-aosp-15.0.0r4)；

	其蓝牙编码的内容与本仓库已集成的android_packages_modules_Bluetooth没有太大差异，故不重复集成了，可访问对于地址直接阅读；

### AOSP移植

我已经从AOSP（android_packages_modules_Bluetooth）移植与esp-idf-Bluedroid相关的内容，包括下列6个文件，并**已更新到本仓库**，均编译通过：
```c
extended-a2dp-bluedroid/250917_migrated_files/Bluedroid-a2dp-srcs：
- a2dp_vendor_lhdcv5.c
- a2dp_vendor_lhdcv5_decoder.c

extended-a2dp-bluedroid/250917_migrated_files/Bluedroid-a2dp-incs：
- a2dp_vendor_lhdcv5.h
- a2dp_vendor_lhdcv5_decoder.h
- a2dp_vendor_lhdc_constants.h
- a2dp_vendor_lhdcv5_constants.h
```

我已经从android_external_lhdc移植了与lhdcv5解码器相关的库，包括下面个文件，**已更新到本仓库**，均编译通过：
```c
extended-a2dp-bluedroid/250917_migrated_files/external-lib/liblhdcv5dec：
|- inc：
|-- lhdcv5BT_dec.h
|- include：
|-- lhdcv5_util_dec.h
|- src：
|-- lhdcv5_util_dec.c
|-- lhdcv5BT_dec.c
|- CMakeLists.txt
```

现在的问题就是位于extended-a2dp-bluedroid/250917_migrated_files/external-lib/liblhdcv5dec的lhdcv5_util_dec.c文件是我依据lhdcv5_util_dec.h模拟出来的，不具备解码功能，仅解决了编译方面的问题；

哪位朋友能提供真实的lhdcv5_util_dec.c文件？我会非常感谢！

### 缺失的核心文件

我说说是为什么会从esp-idf定位到lhdcv5_util_dec.c这个文件：
- 从esp-idf的a2dp_vendor.c分析出需要a2dp_vendor_lhdcv5.h这个文件，并分析出需要10个lhdc方面的函数，因此在a2dp_vendor_lhdcv5.c中结合AOSP实现了这10个函数，其声明如下，位于a2dp_vendor_lhdcv5.h；
	```c
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
	```
- 最后一个函数A2DP_GetVendorDecoderInterfaceLhdcv5()需要调用A2DP_LHDCV5_DecoderInterface()：
	```c
	const tA2DP_DECODER_INTERFACE* A2DP_GetVendorDecoderInterfaceLhdcv5(const uint8_t* p_codec_info) {
		return A2DP_LHDCV5_DecoderInterface();
	}
	```
- 函数A2DP_LHDCV5_DecoderInterface()定义在a2dp_vendor_lhdcv5_decoder.c，而这个函数会用到lhdcv5_decoder_interface，本质上是7个关于lhdcv5_decoder的函数：
	```c
	static const tA2DP_DECODER_INTERFACE lhdcv5_decoder_interface = {
		a2dp_lhdcv5_decoder_init,
		a2dp_lhdcv5_decoder_cleanup,
		a2dp_lhdcv5_decoder_start,
		a2dp_lhdcv5_decoder_suspend,
		a2dp_lhdcv5_decoder_configure,
		a2dp_lhdcv5_decoder_decode_header,
		a2dp_lhdcv5_decoder_decode_packet
	};
	
	const tA2DP_DECODER_INTERFACE* A2DP_LHDCV5_DecoderInterface(void) {
		return &lhdcv5_decoder_interface;
	}
	```
- lhdcv5_decoder_interface中的7个关于lhdcv5_decoder的函数，有3个函数依赖lhdcv5BT_dec.h：
	```c
	函数a2dp_lhdcv5_decoder_cleanup()需要lhdcv5BT_dec_deinit_decoder()；
	函数a2dp_lhdcv5_decoder_decode_packet需要lhdcv5BT_dec_decode()和LHDCV5BT_DEC_API_SUCCEED；
	函数a2dp_lhdcv5_decoder_configure需要tLHDCV5_DEC_CONFIG、lhdcv5BT_dec_init_decoder、LHDCV5BT_DEC_API_SUCCEED；
	同时a2dp_vendor_lhdcv5_decoder.h的结构体tA2DP_LHDCV5_DECODER需要HANDLE_LHDCV5_BT；
	```
- 函数lhdcv5BT_dec_deinit_decoder()来自lhdcv5BT_dec.c，参数是HANDLE_LHDCV5_BT handle，这个HANDLE_LHDCV5_BT又来自lhdcv5_util.h，其实是个void *型定义；
- 函数lhdcv5BT_dec_decode()也来自lhdcv5BT_dec.c；
- 宏定义LHDCV5BT_DEC_API_SUCCEED来自lhdcv5BT_dec.h，本质上等效于ESP_OK，值为0；
- 结构体tLHDCV5_DEC_CONFIG来自lhdcv5BT_dec.h，其实就是配置lhdc的基本参数，如采样率、位深等等；
- 函数lhdcv5BT_dec_init_decoder()来自lhdcv5BT_dec.c；
- lhdcv5BT_dec.c/.h都很容易获得，其中lhdcv5BT_dec.h包含了lhdcv5_util_dec.h，这个也能获得；
- 那么看看，lhdcv5BT_dec.c/.h使用了哪些与lhdcv5_util_dec.c/.h的内容：
	```c
	lhdcv5BT_dec.h：类型lhdc_ver_t、类型HANDLE_LHDCV5_BT，都还好，不是什么大问题；
	
	lhdcv5BT_dec.c：主要是宏定义、类型、函数：
	宏定义A2DP_LHDC_HDR_LATENCY_MASK、宏定义A2DP_LHDC_HDR_FRAME_NO_MASK、类型HANDLE_LHDCV5_BT、
	宏定义LHDCV5_UTIL_DEC_SUCCESS、宏定义VERSION_5、宏定义LHDC_OUTPUT_STEREO、类型lhdc_frame_Info_t、宏定义LHDCV5_UTIL_DEC_ERROR_PARAM、
	函数lhdcv5_util_dec_fetch_frame_info()、函数lhdcv5_util_dec_get_sample_size、函数lhdcv5_util_dec_process；
	```
下面逐一分析lhdcv5BT_dec.c/.h使用的与lhdcv5_util_dec.c/.h相关的内容：
- 枚举lhdc_ver_t来自lhdcv5_util_dec.h，里面就一个东西：VERSION_5 = 550，内容如下：
	```c
	typedef enum {
	  VERSION_5 = 550
	}lhdc_ver_t;
	```
- 类型HANDLE_LHDCV5_BT来自lhdcv5_util_dec.h，内容如下：
	```c
	typedef void * HANDLE_LHDCV5_BT;
	```
- 宏定义A2DP_LHDC_HDR_LATENCY_MASK、宏定义A2DP_LHDC_HDR_FRAME_NO_MASK来自lhdcv5_util_dec.h，内容如下：
	```c
	#define A2DP_LHDC_HDR_LATENCY_MID   0x01
	#define A2DP_LHDC_HDR_LATENCY_HIGH  0x02
	#define A2DP_LHDC_HDR_LATENCY_MASK  (A2DP_LHDC_HDR_LATENCY_MID | A2DP_LHDC_HDR_LATENCY_HIGH)

	#define A2DP_LHDC_HDR_FRAME_NO_MASK 0xfc
	```
- 宏定义LHDCV5_UTIL_DEC_SUCCESS来自lhdcv5_util_dec.h，也等效于ESP_OK，值为0；
- 宏定义VERSION_5其实是个枚举，前面说了，它来自lhdcv5_util_dec.h，值为550；
- 宏定义LHDC_OUTPUT_STEREO也是个枚举，来自lhdcv5_util_dec.h，值为0，具体如下：
	```c
	typedef enum {
	  LHDC_OUTPUT_STEREO = 0,
	  LHDC_OUTPUT_LEFT_CAHNNEL,
	  LHDC_OUTPUT_RIGHT_CAHNNEL,
	} lhdc_channel_t;
	```
- 类型lhdc_frame_Info_t在上面说了，来自lhdcv5_util_dec.h，属于声道模式枚举；
- 宏定义LHDCV5_UTIL_DEC_ERROR_PARAM来自lhdcv5_util_dec.h，值为-2；
- 函数lhdcv5_util_dec_fetch_frame_info()声明于lhdcv5_util_dec.h，显然其被定义在lhdcv5_util_dec.c，这个未知文件中；
- 函数lhdcv5_util_dec_get_sample_size()声明于lhdcv5_util_dec.h，显然其被定义在lhdcv5_util_dec.c，这个未知文件中；
- 函数lhdcv5_util_dec_process()声明于lhdcv5_util_dec.h，显然其被定义在lhdcv5_util_dec.c，这个未知文件中；

OK，分析到这里大致确定缺少三个函数lhdcv5_util_dec_fetch_frame_info()、lhdcv5_util_dec_get_sample_size()、lhdcv5_util_dec_process()；他们都来源于未知文件lhdcv5_util_dec.c；

观察lhdcv5_util_dec.h，其中声明了9个函数，具体如下：  
```c
1int32_t lhdcv5_util_init_decoder(uint32_t *ptr, uint32_t bitPerSample, uint32_t sampleRate, uint32_t scaleTo16Bits, uint32_t is_lossless_enable, lhdc_ver_t version);
int32_t lhdcv5_util_dec_process(uint8_t * pOutBuf, uint8_t * pInput, uint32_t InLen, uint32_t *OutLen);
char *lhdcv5_util_dec_get_version();
int32_t lhdcv5_util_dec_destroy();
void lhdcv5_util_dec_register_log_cb(print_log_fp cb);
int32_t lhdcv5_util_dec_get_sample_size (uint32_t *frame_samples);
int32_t lhdcv5_util_dec_fetch_frame_info(uint8_t *frameData, uint32_t frameDataLen, lhdc_frame_Info_t *frameInfo);
int32_t lhdcv5_util_dec_channel_selsect(lhdc_channel_t channel_type);
int32_t lhdcv5_util_dec_get_mem_req(lhdc_ver_t version, uint32_t *mem_req_bytes);
```
我继续测试移除lhdcv5_util_dec.c，提示缺少6个函数：  
```c
void lhdcv5_util_dec_register_log_cb(print_log_fp cb);
int32_t lhdcv5_util_dec_get_mem_req(lhdc_ver_t version, uint32_t *mem_req_bytes);
int32_t lhdcv5_util_init_decoder(uint32_t *ptr, uint32_t bitPerSample, uint32_t sampleRate, uint32_t scaleTo16Bits, uint32_t is_lossless_enable, lhdc_ver_t version);
int32_t lhdcv5_util_dec_channel_selsect(lhdc_channel_t channel_type);
int32_t lhdcv5_util_dec_fetch_frame_info(uint8_t *frameData, uint32_t frameDataLen, lhdc_frame_Info_t *frameInfo);
int32_t lhdcv5_util_dec_process(uint8_t * pOutBuf, uint8_t * pInput, uint32_t InLen, uint32_t *OutLen);
```
但是lhdcv5_util_dec.h声明9个函数，另外三个是什么情况？：  
```c
函数char *lhdcv5_util_dec_get_version();显然也被需要，其被用于lhdcv5BT_dec.c的编码器版本打印；
函数int32_t lhdcv5_util_dec_destroy();显然也被需要，其被用于lhdcv5BT_dec.c的lhdcv5BT_dec_deinit_decoder()函数，用来销毁解码器；
函数int32_t lhdcv5_util_dec_get_sample_size (uint32_t *frame_samples);显然也需要，其被用在lhdcv5BT_dec.c中，用于更新变量frame_samples；
```
所以现在很明确了，lhdcv5_util_dec.h中声明的9个函数都是有用的（虽然看起来和没说一样...）。

那任务就很明确了，依据现有关系，先在lhdcv5_util_dec.c中实现这9个函数的框架，然后再此次编写出这9个函数实现功能的具体内容；

### Next Plan1

实现lhdcv5_util_dec.c中这9个函数的框架很容易（当前已实现），因为声明中函数的变量及其类型都给出了，函数的类型也给出了，返回值的类型确定了，返回值的具体内容则依据此函数被调用的地方逆推；

然后实现这9个函数的具体功能，那就是要熟悉LHDC或类似的编码，结合其余关联的库、被调用的函数，推导出其实现功能的具体内容；

目前正在逐步分析，试图写出lhdcv5_util_dec.c中对应函数的具体内容；

所有移植&更新的内容已同步到本仓库，目录结构详见本章节“AOSP移植”，欢迎感兴趣的朋友一同测试交流经验；

## 新的方向

之前我一直试图寻找lhdcv5_util_dec.c，或者试图实现lhdcv5_util_dec.c的完整内容，在我观察了大多数这种LHDC案例后，发现底层的东西往往已动态库的形式分发；

我就见到过类似lhdcBT.so的文件，所以不排除那些与Savitech合作的公司用的是lhdcv5_util_dec.so这种动态库，或者静态库lhdcv5_util_dec.a；

因为直接提供源文件会使Savitech的专利没有价值，他们不太可能会干这种事...

不过话说回来，非官方写出lhdcv5_util_dec.c也是完全有可能的，他清楚LHDCV5编码的实现方式便可...

总之现在解决问题多了一条途径：获得Savitech分发的lhdcv5_util_dec.so动态库，又或者是获得lhdcv5BT_dec.so，直接一步到位:)

## 新的问题

抛开LHDC V5编码核心文件的问题，现在有个新的问题，就是蓝牙连接后会自动断开，esp-idf终端日志如下：
```c
W (2051271) BT_APPL: new conn_srvc id:19, app_id:0
I (2051741) WeiLeng-Player: bt连接状态: 2, 已连接
W (2054471) BT_HCI: hci cmd send: sniff: hdl 0x81, intv(400 800)
W (2054531) BT_HCI: hcif mode change: hdl 0x81, mode 2, intv 768, status 0x0
W (2064391) BT_HCI: hcif mode change: hdl 0x81, mode 0, intv 0, status 0x0
W (2064401) BT_APPL: bta_dm_act no entry for connected service cbs
W (2064401) BT_AVCT: avct_lcb_last_ccb
W (2064401) BT_AVCT: 0: aloc:1, lcb:0x0/0x3ffcf7fc, ccb:0x3ffcf864/0x3ffcf87c
W (2064411) BT_AVCT: 1: aloc:1, lcb:0x3ffcf7fc/0x3ffcf7fc, ccb:0x3ffcf87c/0x3ffcf87c
W (2064421) BT_AVCT: 2: aloc:0, lcb:0x0/0x3ffcf7fc, ccb:0x3ffcf894/0x3ffcf87c
I (2064431) WeiLeng-Player: bt连接状态: 0, 已断开
W (2068421) BT_HCI: hci cmd send: disconnect: hdl 0x81, rsn:0x13
W (2068511) BT_HCI: hcif disc complete: hdl 0x81, rsn 0x16
```
reset & retry：
```c
W (4581) BT_APPL: new conn_srvc id:19, app_id:0
I (5161) WeiLeng-Player: bt连接状态: 2, 已连接
W (7781) BT_HCI: hci cmd send: sniff: hdl 0x81, intv(400 800)
W (7781) BT_HCI: hcif mode change: hdl 0x81, mode 2, intv 768, status 0x0
W (20041) BT_HCI: hci cmd send: unsniff: hdl 0x81
W (20051) BT_APPL: bta_dm_act no entry for connected service cbs
W (20051) BT_AVCT: avct_lcb_last_ccb
W (20051) BT_AVCT: 0: aloc:1, lcb:0x0/0x3ffcf7fc, ccb:0x3ffcf864/0x3ffcf87c
W (20061) BT_AVCT: 1: aloc:1, lcb:0x3ffcf7fc/0x3ffcf7fc, ccb:0x3ffcf87c/0x3ffcf87c
W (20061) BT_AVCT: 2: aloc:0, lcb:0x0/0x3ffcf7fc, ccb:0x3ffcf894/0x3ffcf87c
I (20081) WeiLeng-Player: bt连接状态: 0, 已断开
W (20521) BT_HCI: hci cmd send: sniff: hdl 0x81, intv(400 800)
W (20521) BT_HCI: hcif mode change: hdl 0x81, mode 0, intv 0, status 0x0
W (20531) BT_HCI: hcif mode change: hdl 0x81, mode 2, intv 768, status 0x0
W (24071) BT_HCI: hci cmd send: disconnect: hdl 0x81, rsn:0x13
W (24361) BT_HCI: hcif mode change: hdl 0x81, mode 0, intv 0, status 0x0
W (24441) BT_HCI: hcif disc complete: hdl 0x81, rsn 0x16
```
注意到问题TAG指向：BT_HCI、BT_APPL、BT_AVCT

在menuconfig关掉LHDCV5的选项，再次编译，则蓝牙正常连接，日志是这样：
```c
W (5019) BT_APPL: new conn_srvc id:19, app_id:0
I (5619) WeiLeng-Player: bt连接状态: 2, 已连接
W (8229) BT_HCI: hci cmd send: sniff: hdl 0x81, intv(400 800)
W (8229) BT_HCI: hcif mode change: hdl 0x81, mode 2, intv 768, status 0x0
```
所以后面准备分析一下，在menuconfig启用LHDCV5时，为什么蓝牙连接后会断开；

