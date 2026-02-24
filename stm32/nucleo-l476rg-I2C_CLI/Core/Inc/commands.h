/*
 * commands.h
 *
 *  Created on: Nov 27, 2025
 *      Author: szymo
 */

#ifndef INC_COMMANDS_H_
#define INC_COMMANDS_H_

#include "main.h"
#include "BME280.h"

void Commands_Init(UART_HandleTypeDef *huart);
void Commands_Process(uint8_t byte);

#endif /* INC_COMMANDS_H_ */
