import re
import math
from enum import Enum
from typing import List, Dict, Tuple, Optional
import numpy as np
class BiQuadType(Enum):
    LOW_PASS=0
    HIGH_PASS=1
    BAND_PASS=2
    NOTCH=3
    ALL_PASS=4
    PEAKING=5
    LOW_SHELF=6
    HIGH_SHELF=7
class BiQuad:
    def __init__(self, filter_type, db_gain, freq, srate, bandwidth_or_q_or_s, is_bandwidth_or_s):
        # 计算增益A
        if filter_type in [BiQuadType.PEAKING, BiQuadType.LOW_SHELF, BiQuadType.HIGH_SHELF]:
            A = math.pow(10, db_gain / 40.0)
        else:
            A = math.pow(10, db_gain / 20.0)
            
        omega = 2 * math.pi * freq / srate
        sn = math.sin(omega)
        cs = math.cos(omega)
        
        # 计算alpha值
        if not is_bandwidth_or_s:  # Q值
            alpha = sn / (2 * bandwidth_or_q_or_s)
        elif filter_type in [BiQuadType.LOW_SHELF, BiQuadType.HIGH_SHELF]:  # S值
            alpha = sn / 2 * math.sqrt((A + 1 / A) * (1 / bandwidth_or_q_or_s - 1) + 2)
        else:  # 带宽
            alpha = sn * math.sinh(math.log(2) / 2 * bandwidth_or_q_or_s * omega / sn)

        beta = 2 * math.sqrt(A) * alpha

        # 初始化滤波器系数
        b0, b1, b2, a0, a1, a2 = 0, 0, 0, 0, 0, 0

        # 根据滤波器类型计算系数
        if filter_type == BiQuadType.LOW_PASS:
            b0 = (1 - cs) / 2
            b1 = 1 - cs
            b2 = (1 - cs) / 2
            a0 = 1 + alpha
            a1 = -2 * cs
            a2 = 1 - alpha
        elif filter_type == BiQuadType.HIGH_PASS:
            b0 = (1 + cs) / 2
            b1 = -(1 + cs)
            b2 = (1 + cs) / 2
            a0 = 1 + alpha
            a1 = -2 * cs
            a2 = 1 - alpha
        elif filter_type == BiQuadType.BAND_PASS:
            b0 = alpha
            b1 = 0
            b2 = -alpha
            a0 = 1 + alpha
            a1 = -2 * cs
            a2 = 1 - alpha
        elif filter_type == BiQuadType.NOTCH:
            b0 = 1
            b1 = -2 * cs
            b2 = 1
            a0 = 1 + alpha
            a1 = -2 * cs
            a2 = 1 - alpha
        elif filter_type == BiQuadType.ALL_PASS:
            b0 = 1 - alpha
            b1 = -2 * cs
            b2 = 1 + alpha
            a0 = 1 + alpha
            a1 = -2 * cs
            a2 = 1 - alpha
        elif filter_type == BiQuadType.PEAKING:
            b0 = 1 + (alpha * A)
            b1 = -2 * cs
            b2 = 1 - (alpha * A)
            a0 = 1 + (alpha / A)
            a1 = -2 * cs
            a2 = 1 - (alpha / A)
        elif filter_type == BiQuadType.LOW_SHELF:
            b0 = A * ((A + 1) - (A - 1) * cs + beta)
            b1 = 2 * A * ((A - 1) - (A + 1) * cs)
            b2 = A * ((A + 1) - (A - 1) * cs - beta)
            a0 = (A + 1) + (A - 1) * cs + beta
            a1 = -2 * ((A - 1) + (A + 1) * cs)
            a2 = (A + 1) + (A - 1) * cs - beta
        elif filter_type == BiQuadType.HIGH_SHELF:
            b0 = A * ((A + 1) + (A - 1) * cs + beta)
            b1 = -2 * A * ((A - 1) + (A + 1) * cs)
            b2 = A * ((A + 1) + (A - 1) * cs - beta)
            a0 = (A + 1) - (A - 1) * cs + beta
            a1 = 2 * ((A - 1) - (A + 1) * cs)
            a2 = (A + 1) - (A - 1) * cs - beta

        self.a0 = b0 / a0
        self.a1 = b1 / a0
        self.a2 = b2 / a0
        self.a3 = a1 / a0
        self.a4 = a2 / a0

class BiQuadFilter:
    def __init__(self, filter_type: BiQuadType, gain: float, freq: float, 
                 bandwidth_or_q_or_s: float, is_bandwidth_or_s: bool, is_corner_freq: bool):
        self.type = filter_type
        self.gain = gain
        self.bandwidth_or_q_or_s = bandwidth_or_q_or_s
        self.is_bandwidth_or_s = is_bandwidth_or_s
        self.is_corner_freq = is_corner_freq
        self.biquad_freq = freq
        if (is_corner_freq and (filter_type == BiQuadType.LOW_SHELF or filter_type == BiQuadType.HIGH_SHELF)):
            s = bandwidth_or_q_or_s
            if not is_bandwidth_or_s:
                q = bandwidth_or_q_or_s
                a = pow(10, gain / 40)
                s = 1.0 / ((1.0 / (q * q) - 2.0) / (a + 1.0 / a) + 1.0)
            
            centerFreqFactor = pow(10.0, abs(gain) / 80.0 / s)
            if filter_type == BiQuadType.LOW_SHELF:
                self.biquad_freq *= centerFreqFactor
            else:
                self.biquad_freq /= centerFreqFactor
        
	
    def __str__(self):
        type_str = self.type.name.replace('_', ' ').title()
        params = f"Fc {self.biquad_freq} Hz, Gain {self.gain} dB"
        if self.is_bandwidth_or_s:
            if self.type in [BiQuadType.LOW_SHELF, BiQuadType.HIGH_SHELF]:
                params += f", Slope {self.bandwidth_or_q_or_s * 12} dB"
            else:
                params += f", BW {self.bandwidth_or_q_or_s} Oct"
        else:
            params += f", Q {self.bandwidth_or_q_or_s}"
        
        if self.is_corner_freq:
            params += " (Corner Frequency)"
        
        return f"{type_str} Filter: {params}"
    def get_iir_param(self,srate):
        bq = BiQuad(self.type,self.gain,self.biquad_freq,srate,self.bandwidth_or_q_or_s,self.is_bandwidth_or_s)
        return [bq.a0,bq.a1,bq.a2,bq.a3,bq.a4]
class BiQuadFilterFactory:
    def __init__(self):
        # 滤波器类型映射
        self.filter_name_to_type_map = {
            "PK": BiQuadType.PEAKING,
            "PEQ": BiQuadType.PEAKING,
            "Modal": BiQuadType.PEAKING,
            "LP": BiQuadType.LOW_PASS,
            "HP": BiQuadType.HIGH_PASS,
            "LPQ": BiQuadType.LOW_PASS,
            "HPQ": BiQuadType.HIGH_PASS,
            "BP": BiQuadType.BAND_PASS,
            "LS": BiQuadType.LOW_SHELF,
            "HS": BiQuadType.HIGH_SHELF,
            "LSC": BiQuadType.LOW_SHELF,
            "HSC": BiQuadType.HIGH_SHELF,
            "NO": BiQuadType.NOTCH,
            "AP": BiQuadType.ALL_PASS
        }
        
        # 滤波器类型描述
        self.filter_type_to_description_map = {
            BiQuadType.PEAKING: "peaking",
            BiQuadType.LOW_PASS: "low-pass",
            BiQuadType.HIGH_PASS: "high-pass",
            BiQuadType.BAND_PASS: "band-pass",
            BiQuadType.LOW_SHELF: "low-shelf",
            BiQuadType.HIGH_SHELF: "high-shelf",
            BiQuadType.NOTCH: "notch",
            BiQuadType.ALL_PASS: "all-pass"
        }
        
        # 正则表达式模式
        self.regex_type = re.compile(r"^\s*ON\s+([A-Za-z]+)")
        self.regex_freq = re.compile(r"\s+Fc\s*([-+0-9.eE\u00A0]+)\s*H\s*z")
        self.regex_gain = re.compile(r"\s+Gain\s*([-+0-9.eE]+)\s*dB")
        self.regex_q = re.compile(r"\s+Q\s*([-+0-9.eE]+)")
        self.regex_bw = re.compile(r"\s+BW\s+Oct\s*([-+0-9.eE]+)")
        self.regex_slope = re.compile(r"^\s*([-+0-9.eE]+)\s*dB")
    def create_filter(self, config_line: str) -> Optional[BiQuadFilter]:
        """
        根据配置行创建滤波器
        
        参数:
            config_line (str): 滤波器配置行
            
        返回:
            Optional[BiQuadFilter]: 解析后的滤波器对象，如果解析失败则返回None
        """
        # 检查是否是滤波器配置行
        if not config_line.startswith("Filter"):
            return None
        
        # 分离命令和参数
        parts = config_line.split(":", 1)
        if len(parts) < 2:
            return None
            
        command = parts[0].strip()
        parameters = parts[1].strip()
        
        # 转换小数点和千位分隔符
        parameters = parameters.replace(",", ".").replace("\u00A0", "")
        
        # 匹配滤波器类型
        type_match = self.regex_type.search(parameters)
        if not type_match:
            return None
            
        type_string = type_match.group(1)
        if type_string not in self.filter_name_to_type_map:
            if type_string != "None":
                print(f"Invalid filter type: {type_string}")
            return None
            
        filter_type = self.filter_name_to_type_map[type_string]
        type_description = self.filter_type_to_description_map[filter_type]
        
        # 移除已匹配的类型部分
        parameters = parameters[type_match.end():]
        
        # 初始化参数
        freq = 0.0
        gain = 0.0
        bandwidth_or_q_or_s = 0.0
        is_bandwidth_or_s = False
        is_corner_freq = False
        error = False
        
        # 匹配频率
        freq_match = self.regex_freq.search(parameters)
        if freq_match:
            freq_string = freq_match.group(1)
            freq = self._get_freq(freq_string)
            if freq < 0:
                print(f"No frequency given in filter string {type_string}{parameters}")
                error = True
        else:
            print(f"No frequency given in filter string {type_string}{parameters}")
            error = True
        
        # 匹配增益
        gain_match = self.regex_gain.search(parameters)
        if gain_match:
            if filter_type in [BiQuadType.LOW_PASS, BiQuadType.HIGH_PASS, 
                              BiQuadType.NOTCH, BiQuadType.ALL_PASS]:
                print(f"Ignoring gain for filter of type {type_description}")
            else:
                gain_string = gain_match.group(1)
                gain = float(gain_string)
        elif filter_type in [BiQuadType.PEAKING, BiQuadType.LOW_SHELF, BiQuadType.HIGH_SHELF]:
            print(f"No gain given in filter string {type_string}{parameters}")
            error = True
        
        # 匹配Q值
        q_match = self.regex_q.search(parameters)
        if q_match:
            q_string = q_match.group(1)
            bandwidth_or_q_or_s = float(q_string)
        
        # 匹配带宽
        bw_match = self.regex_bw.search(parameters)
        if bw_match:
            if filter_type in [BiQuadType.LOW_SHELF, BiQuadType.HIGH_SHELF]:
                print(f"Ignoring bandwidth for filter of type {type_description}")
            else:
                bw_string = bw_match.group(1)
                bandwidth_or_q_or_s = float(bw_string)
                is_bandwidth_or_s = True
        
        # 匹配斜率
        slope_match = self.regex_slope.search(parameters)
        if slope_match:
            if filter_type not in [BiQuadType.LOW_SHELF, BiQuadType.HIGH_SHELF]:
                print(f"Ignoring slope for filter of type {type_description}")
            else:
                slope_string = slope_match.group(1)
                bandwidth_or_q_or_s = float(slope_string)
                is_bandwidth_or_s = True
        
        # 设置默认值
        if bandwidth_or_q_or_s == 0:
            if filter_type in [BiQuadType.PEAKING, BiQuadType.ALL_PASS]:
                print(f"No Q or bandwidth given in filter string {type_string}{parameters}")
                error = True
            elif filter_type in [BiQuadType.LOW_PASS, BiQuadType.HIGH_PASS, BiQuadType.BAND_PASS]:
                bandwidth_or_q_or_s = math.sqrt(0.5)  # M_SQRT1_2
            elif filter_type in [BiQuadType.LOW_SHELF, BiQuadType.HIGH_SHELF]:
                bandwidth_or_q_or_s = 0.9  # 通过RoomEQWizard实验得出
                is_bandwidth_or_s = True
            elif filter_type == BiQuadType.NOTCH:
                bandwidth_or_q_or_s = 30.0  # 通过RoomEQWizard实验得出
        
        # 处理低架和高架滤波器的斜率和角频率
        if filter_type in [BiQuadType.LOW_SHELF, BiQuadType.HIGH_SHELF]:
            if is_bandwidth_or_s:
                # 最大S为1对应12 dB
                bandwidth_or_q_or_s /= 12.0
            if not type_string.endswith('C'):
                is_corner_freq = True
        
        if error:
            return None
        
        # 创建滤波器对象
        return BiQuadFilter(filter_type, gain, freq, bandwidth_or_q_or_s, 
                           is_bandwidth_or_s, is_corner_freq)
    
    def _get_freq(self, freq_string: str) -> float:
        """
        解析频率字符串，处理千位分隔符
        
        参数:
            freq_string (str): 频率字符串
            
        返回:
            float: 解析后的频率值，如果解析失败则返回-1.0
        """
        # 移除千位分隔符（不换行空格）
        s = freq_string.replace("\u00A0", "")
        
        try:
            result = float(s)
            
            # 检查是否需要乘以1000（处理千位分隔符）
            if len(s) >= 5 and 'e' not in s.lower() and s[-4] == '.':
                result *= 1000.0
                
            return result
        except ValueError:
            return -1.0
def to_int24(input_array):
    input_array = np.asarray(input_array)
    input_array /= 4

    if input_array.shape != (5,):
        raise ValueError("输入数组必须包含5个元素")
    input_array[3] = -input_array[3]
    input_array[4] = -input_array[4]
    # 定义常量
    SCALE = 2**23  # 8388608.0
    MAX_POS = 2**23 - 1  # 8388607
    MAX_NEG = -2**23     # -8388608
    UINT24_OFFSET = 2**24  # 16777216，用于处理负数转换

    # 检查输入值是否在[-1.0, 1.0]范围内
    if np.any(np.abs(input_array) > 1.0):
        return None
    
    output_array = np.zeros(5, dtype=np.int32)
    
    for i in range(5):
        value = input_array[i]
        # 缩放和舍入
        scaled_value = value * SCALE
        rounded_value = np.floor(scaled_value+0.5)
        #rounded_value = np.round(scaled_value)
        
        # 钳位到24位有符号整数范围
        clamped_value = np.clip(rounded_value, MAX_NEG, MAX_POS)
        
        # 如果为负数，转换为无符号表示
        if clamped_value < 0:
            clamped_value += UINT24_OFFSET
        
        output_array[i] = int(clamped_value)
    tmp_out = output_array[3]
    output_array[3] = output_array[2]
    output_array[2] = tmp_out
    return output_array
def parse_filter_config(config_text: str) -> List[BiQuadFilter]:
    """
    解析滤波器配置文本
    
    参数:
        config_text (str): 包含滤波器配置的文本
        
    返回:
        List[BiQuadFilter]: 解析后的滤波器对象列表
    """
    factory = BiQuadFilterFactory()
    filters = []
    
    lines = config_text.strip().split('\n')

    for line in lines:
        filter_obj = factory.create_filter(line)
        if filter_obj:
            filters.append(filter_obj)
            int24_array = to_int24(filter_obj.get_iir_param(48000))
            for int24 in int24_array:
                print(f'0x{int24:x}',end=',')
            print('')
        else:
            print(f"Warning: Failed to parse line: {line}")
    return filters

def main():
    # 配置文本
    config_text = """Filter 1: ON LSC Fc 105 Hz Gain 6.3 dB Q 0.70
Filter 2: ON PK Fc 1857 Hz Gain 4.1 dB Q 2.91
Filter 3: ON PK Fc 775 Hz Gain -1.3 dB Q 1.77
Filter 4: ON PK Fc 152 Hz Gain -1.1 dB Q 0.38
Filter 5: ON PK Fc 5262 Hz Gain 2.1 dB Q 4.08
Filter 6: ON HSC Fc 10000 Hz Gain -3.8 dB Q 0.70
Filter 7: ON PK Fc 7790 Hz Gain -1.4 dB Q 3.20
Filter 8: ON PK Fc 2843 Hz Gain -1.7 dB Q 4.33
Filter 9: ON PK Fc 5778 Hz Gain 1.3 dB Q 5.98
Filter 10: ON PK Fc 2222 Hz Gain 1.1 dB Q 6.00"""
    
    # 解析配置
    filters = parse_filter_config(config_text)
    
    # 打印结果
    print("Parsed filter configurations:")
    for i, filter_obj in enumerate(filters, 1):
        print(f"{i}. {filter_obj}")
    
    return filters

if __name__ == "__main__":
    main()