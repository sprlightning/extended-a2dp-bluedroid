#include "rtthread.h"
#include "bf0_hal.h"
#include "drv_io.h"
#include "stdio.h"
#include "string.h"
#include "board.h"

#define DBG_TAG "dac_i2c"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define DAC_I2C_ADDRESS 0x90 >> 1 // 7bit device address of EEPROM
I2C_HandleTypeDef i2c_Handle;
void es9038q2m_write(uint16_t regx, uint8_t *data)
{
    HAL_StatusTypeDef ret = 0;
    HAL_I2C_Mem_Write(&i2c_Handle, DAC_I2C_ADDRESS, regx, I2C_MEMADD_SIZE_8BIT,
                      data, 1, 1000);

    if (ret && regx != 0)
    {
        rt_kprintf("HAL_I2C_Mem_Write %d\n", regx);
    }
}
void es9038q2m_read(uint16_t regx, uint8_t *data)
{
    HAL_StatusTypeDef ret = 0;
    HAL_I2C_Mem_Read(&i2c_Handle, DAC_I2C_ADDRESS, regx, I2C_MEMADD_SIZE_8BIT,
                     data, 1, 1000);

    if (ret)
    {
        rt_kprintf("HAL_I2C_Mem_Read %d\n", regx);
    }
}
void es9038q2m_init(void)
{
    if (i2c_Handle.Instance != 0)
    {
        return;
    }
    
    HAL_PIN_Set(PAD_PA41, I2C2_SCL, PIN_PULLUP, 1); // i2c io select
    HAL_PIN_Set(PAD_PA42, I2C2_SDA, PIN_PULLUP, 1);

    HAL_RCC_EnableModule(RCC_MOD_I2C2);

    HAL_StatusTypeDef ret = 0;
    i2c_Handle.Instance = I2C2;
    i2c_Handle.Mode = HAL_I2C_MODE_MASTER; // i2c master mode
    i2c_Handle.Init.AddressingMode =
        I2C_ADDRESSINGMODE_7BIT;         // i2c 7bits device address mode
    i2c_Handle.Init.ClockSpeed = 100000; // i2c speed (hz)
    i2c_Handle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    i2c_Handle.Init.OwnAddress1 = 0;

    ret = HAL_I2C_Init(&i2c_Handle);
    if (ret)
    {
        rt_kprintf("HAL_I2C_Init\n");
    }

    __HAL_I2C_ENABLE(&i2c_Handle);
    uint8_t data = 1;

    es9038q2m_write(0, &data);

    data = 0b10000010;
    es9038q2m_write(14, &data);
    // es9038q2m_read(1, &data);
    data = 0b00010000;
    es9038q2m_write(1, &data);
    data = 30 * 2;
    es9038q2m_write(15, &data);
    data = 30 * 2;
    es9038q2m_write(16, &data);
}

void es9038q2m_set_data_width(uint8_t bit_width)
{
    uint8_t data = 0;
    es9038q2m_read(1, &data);
    switch (bit_width)
    {
    case 16:
        data |= (0 << 6);
        break;
    case 24:
        data |= (1 << 6);
        break;
    case 32:
        data |= (2 << 6);
        break;
    default:
        data |= (3 << 6);
        rt_kprintf("Unsupport bit width\r\n");
        break;
    }
    es9038q2m_write(1, &data);
}
#define Volume_limit 255
uint8_t volume_table[] = {
    0,   35,  56,  72,  83,  93,  101, 108, 114, 120, 125, 130, 134, 138, 141,
    145, 148, 151, 154, 157, 159, 162, 164, 166, 168, 170, 172, 174, 176, 178,
    180, 181, 183, 185, 186, 188, 189, 190, 192, 193, 194, 196, 197, 198, 199,
    201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215,
    215, 216, 217, 218, 219, 220, 220, 221, 222, 223, 223, 224, 225, 226, 226,
    227, 228, 228, 229, 230, 230, 231, 232, 232, 233, 234, 234, 235, 235, 236,
    236, 237, 238, 238, 239, 239, 240, 240, 241, 241, 242, 243, 243, 244, 244,
    245, 245, 246, 246, 247, 247, 247, 248, 248, 249, 249, 250, 250, 251, 251,
    252, 252, 252, 253, 253, 254, 254, 255};
void es9038q2m_set_volume(int volume)
{
    if (volume < 0)
    {
        volume = 0;
    }
    volume = volume_table[volume];
    if (volume > Volume_limit)
    {
        volume = Volume_limit;
    }

    uint8_t left_vol = Volume_limit - volume;
    uint8_t right_vol = Volume_limit - volume;
    es9038q2m_write(15, &left_vol);
    es9038q2m_write(16, &right_vol);
}