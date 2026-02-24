/*
 * BME280.c
 *
 *  Created on: Nov 26, 2025
 *      Author: szymo
 */

#include "BME280.h"

static I2C_HandleTypeDef *bme_i2c;
static BME280_CalibData bme_calib;
static BME280_Data bme_data;

static int32_t t_fine;  // used by compensation formulas

// Helpers
static HAL_StatusTypeDef BME_ReadReg(uint8_t reg, uint8_t *data, uint16_t size)
{
    return HAL_I2C_Mem_Read(bme_i2c, BME280_ADDR, reg, 1, data, size, 100);
}

static HAL_StatusTypeDef BME_WriteReg(uint8_t reg, uint8_t value)
{
    return HAL_I2C_Mem_Write(bme_i2c, BME280_ADDR, reg, 1, &value, 1, 100);
}



static void BME_ReadCalibrationData(void)
{
    uint8_t buf[32];

    // temp + pressure calib (0x88 -> 0xA1)
    BME_ReadReg(0x88, buf, 26);

    bme_calib.dig_T1 = (buf[1] << 8) | buf[0];
    bme_calib.dig_T2 = (buf[3] << 8) | buf[2];
    bme_calib.dig_T3 = (buf[5] << 8) | buf[4];

    bme_calib.dig_P1 = (buf[7]  << 8) | buf[6];
    bme_calib.dig_P2 = (buf[9]  << 8) | buf[8];
    bme_calib.dig_P3 = (buf[11] << 8) | buf[10];
    bme_calib.dig_P4 = (buf[13] << 8) | buf[12];
    bme_calib.dig_P5 = (buf[15] << 8) | buf[14];
    bme_calib.dig_P6 = (buf[17] << 8) | buf[16];
    bme_calib.dig_P7 = (buf[19] << 8) | buf[18];
    bme_calib.dig_P8 = (buf[21] << 8) | buf[20];
    bme_calib.dig_P9 = (buf[23] << 8) | buf[22];
    bme_calib.dig_H1 = buf[25];

    // humidity calib (0xE1 -> 0xE7)
    BME_ReadReg(0xE1, buf, 7);

    bme_calib.dig_H2 = (buf[1] << 8) | buf[0];
    bme_calib.dig_H3 = buf[2];

    bme_calib.dig_H4 = (buf[3] << 4) | (buf[4] & 0x0F);
    bme_calib.dig_H5 = (buf[5] << 4) | (buf[4] >> 4);

    bme_calib.dig_H6 = buf[6];
}



HAL_StatusTypeDef BME280_Init(I2C_HandleTypeDef *hi2c)
{
    bme_i2c = hi2c;

    uint8_t id;
    if (BME_ReadReg(0xD0, &id, 1) != HAL_OK)
        return HAL_ERROR;

    if (id != 0x60) // check chip ID
        return HAL_ERROR;

    // reset
    BME_WriteReg(0xE0, 0xB6);
    HAL_Delay(10);

    BME_ReadCalibrationData();

    // humidity oversampling ×1
    BME_WriteReg(0xF2, 0x01);

    // temp + pressure oversampling ×1, forced mode
    BME_WriteReg(0xF4, 0x27);

    return HAL_OK;
}

HAL_StatusTypeDef BME280_ReadAll(void)
{
    uint8_t data[8];

    // Read ALL measurement registers: 0xF7 -> 0xFE
    if (BME_ReadReg(0xF7, data, 8) != HAL_OK)
        return HAL_ERROR;

    // Raw readings
    bme_data.adc_P = (int32_t)(data[0] << 12) | (data[1] << 4) | (data[2] >> 4);
    bme_data.adc_T = (int32_t)(data[3] << 12) | (data[4] << 4) | (data[5] >> 4);
    bme_data.adc_H = (int32_t)(data[6] << 8)  | data[7];


    // ---- Temperature compensation ----
    int32_t var1 = ((((bme_data.adc_T >> 3) - ((int32_t)bme_calib.dig_T1 << 1))) *
                    (int32_t)bme_calib.dig_T2) >> 11;

    int32_t var2 = (((((bme_data.adc_T >> 4) - (int32_t)bme_calib.dig_T1) *
                      ((bme_data.adc_T >> 4) - (int32_t)bme_calib.dig_T1)) >> 12) *
                    (int32_t)bme_calib.dig_T3) >> 14;

    t_fine = var1 + var2;
    bme_data.temperature = (float)((t_fine * 5 + 128) >> 8) / 100.0f;


    // ---- Pressure compensation ----
    int64_t p;
    int64_t var1p = ((int64_t)t_fine) - 128000;
    int64_t var2p = var1p * var1p * (int64_t)bme_calib.dig_P6;
    var2p = var2p + ((var1p * (int64_t)bme_calib.dig_P5) << 17);
    var2p = var2p + (((int64_t)bme_calib.dig_P4) << 35);
    var1p = ((var1p * var1p * (int64_t)bme_calib.dig_P3) >> 8) +
            ((var1p * (int64_t)bme_calib.dig_P2) << 12);
    var1p = (((((int64_t)1) << 47) + var1p)) * ((int64_t)bme_calib.dig_P1) >> 33;

    if (var1p == 0) return HAL_ERROR; // avoid divide by zero

    p = 1048576 - bme_data.adc_P;
    p = (((p << 31) - var2p) * 3125) / var1p;
    var1p = (((int64_t)bme_calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2p = (((int64_t)bme_calib.dig_P8) * p) >> 19;
    p = ((p + var1p + var2p) >> 8) + (((int64_t)bme_calib.dig_P7) << 4);

    bme_data.pressure = (float)p / 256.0f;


    // ---- Humidity compensation ----
    int32_t v_x = t_fine - 76800;
    v_x = (((((bme_data.adc_H << 14) -
        ((int32_t)bme_calib.dig_H4 << 20) - ((int32_t)bme_calib.dig_H5 * v_x)) +
        16384) >> 15) *
        (((((((v_x * (int32_t)bme_calib.dig_H6) >> 10) *
        (((v_x * (int32_t)bme_calib.dig_H3) >> 11) + 32768)) >> 10) + 2097152) *
        (int32_t)bme_calib.dig_H2 + 8192) >> 14));

    v_x -= (((v_x >> 15) * (v_x >> 15)) >> 7) * (int32_t)bme_calib.dig_H1 >> 4;
    v_x = (v_x < 0 ? 0 : v_x);
    v_x = (v_x > 419430400 ? 419430400 : v_x);

    bme_data.humidity = (float)(v_x >> 12) / 1024.0f;

    return HAL_OK;
}



float BME280_ReadTemperature(void)
{
	return bme_data.temperature;
}

float BME280_ReadPressure(void)
{
	return bme_data.pressure;
}

float BME280_ReadHumidity(void)
{
	return bme_data.humidity;
}
