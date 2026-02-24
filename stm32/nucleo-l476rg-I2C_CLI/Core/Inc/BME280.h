/*
 * BME280.h
 *
 *  Created on: Nov 26, 2025
 *      Author: szymo
 */

#ifndef INC_BME280_H_
#define INC_BME280_H_

#include "main.h"
#include <stdint.h>

// BME280 I2C address
#define BME280_ADDR (0x76 << 1)

typedef struct {
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;
    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;
    uint8_t  dig_H1;
    int16_t  dig_H2;
    uint8_t  dig_H3;
    int16_t  dig_H4;
    int16_t  dig_H5;
    int8_t   dig_H6;
} BME280_CalibData;

//
typedef struct {
	// calculated readings
    float temperature;
    float pressure;
    float humidity;

    // raw readings
    int32_t adc_T;
    int32_t adc_P;
    int32_t adc_H;
} BME280_Data;

HAL_StatusTypeDef BME280_Init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef BME280_ReadAll(void);   // reads all registers

float BME280_ReadTemperature(void);
float BME280_ReadPressure(void);
float BME280_ReadHumidity(void);

#endif /* INC_BME280_H_ */
