/*
 * commands.c
 *
 *  Created on: Nov 27, 2025
 *      Author: szymo
 */

#include "commands.h"
#include <string.h>
#include <stdio.h>

static UART_HandleTypeDef *cmd_huart;
static uint8_t cmd_buffer[64];
static uint8_t cmd_index = 0;

void Commands_Init(UART_HandleTypeDef *huart)
{
	cmd_huart = huart;
}

static void Commands_Execute(char *cmd)
{
	if (strcmp(cmd, "getInfo") == 0)
	{
		char msg[] = "STM32: System OK\r\n";
		HAL_UART_Transmit(cmd_huart, (uint8_t*)msg, strlen(msg), 100);
	}
	else if (strcmp(cmd, "getTemp") == 0)
	{

		char msg[32];
		float T = BME280_ReadTemperature();

		snprintf(msg, sizeof(msg), "Temp: %.2f C \r\n", T);
		HAL_UART_Transmit(cmd_huart, (uint8_t*)msg, strlen(msg), 100);

	}
	else if (strcmp(cmd, "getPres") == 0)
	{
		char msg[32];
		float P = BME280_ReadPressure();

		snprintf(msg, sizeof(msg), "Pressure: %.2f Pa \r\n", P);
		HAL_UART_Transmit(cmd_huart, (uint8_t*)msg, strlen(msg), 100);
	}
	else if (strcmp(cmd, "getHumid") == 0)
	{
		char msg[32];
		float H = BME280_ReadHumidity();

		snprintf(msg, sizeof(msg), "Humidity: %.2f  %% \r\n", H);
		HAL_UART_Transmit(cmd_huart, (uint8_t*)msg, strlen(msg), 100);
	}
	else if (strcmp(cmd, "getAll") == 0)
	{
		float T = BME280_ReadTemperature();
		float P = BME280_ReadPressure();
		float H = BME280_ReadHumidity();

		char msg[100];
		snprintf(msg, sizeof(msg), "T: %.2f C, P: %.2f Pa, H: %.2f %%\r\n", T, P, H);

		HAL_UART_Transmit(cmd_huart, (uint8_t*)msg, strlen(msg), 100);
	}
	else
	{
		char msg[] = "Unknown command\r\n";
		HAL_UART_Transmit(cmd_huart, (uint8_t*)msg, strlen(msg), 100);
	}
}



void Commands_Process(uint8_t byte)
{
	if (byte == '\n')
	{
		cmd_buffer[cmd_index] = '\0';
		Commands_Execute((char*)cmd_buffer);
		cmd_index = 0;
		return;
	}

	if (cmd_index < sizeof(cmd_buffer) - 1)
	{
		cmd_buffer[cmd_index++] = byte;
	}
}
