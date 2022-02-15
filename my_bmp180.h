/*
 * my_bmp180.h
 *
 *  Created on: Feb 16, 2022
 *      Author: Hamza
 */

#ifndef INC_MY_BMP180_H_
#define INC_MY_BMP180_H_

#include"stm32f0xx_hal.h"			/*			DETERMINE THIS LINE ACCORDING TO YOUR STM32 MODEL			*/
#include"math.h"
#include"stdbool.h"

extern I2C_HandleTypeDef hi2c1;

#define BMP180_READ_REGISTER 0xEF
#define BMP180_WRITE_REGISTER 0xEE

#define BMP180_TEMPERATURE_REG 0x2E
#define BMP180_PRESSURE_REG 0xF4 //oss 3 delay 26ms

#define BMP180_DATA_START_REG 0xAA

#define atmPress 101325 //Pa

void led_alert();
bool MY_BMP180_Init();

uint16_t MY_BMP180_READ_UNCOMP_TEMP_VALUE();
uint32_t MY_BMP180_READ_UNCOMP_PRESSURE_VALUE(int oss);

double MY_BMP180_READ_TEMPERATURE();
double MY_BMP180_READ_PRESSURE(int oss);

double MY_BMP180_GET_ATM_PRESSURE();
double MY_BMP180_GET_ALTITUDE();




#endif /* INC_MY_BMP180_H_ */
