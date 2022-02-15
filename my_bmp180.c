/*
 * my_bmp180.c
 *
 *  Created on: Feb 16, 2022
 *      Author: Hamza TAMER
 */

#include"my_bmp180.h"


/*			THE VALUES MICROPROCESSOR WILL READ			*/
short AC1 = 0;
short AC2 = 0;
short AC3 = 0;
unsigned short AC4 = 0;
unsigned short AC5 = 0;
unsigned short AC6 = 0;
short B1 = 0;
short B2 = 0;
short MB = 0;
short MC = 0;
short MD = 0;

/*		THE VALUES WE WILL CALCULATE 	*/
long UT = 0;
long UP = 0;
long X1 = 0;
long X2 = 0;
long X3 = 0;
long B3 = 0;
long B5 = 0;
unsigned long B4 = 0;
long B6 = 0;
unsigned long B7 = 0;

short oss = 0;
double real_temp,real_pressure;


void led_alert()
{
	for(int i = 0;i<10;i++)				//FOR ATTENTION
	{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
		HAL_Delay(300);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
		HAL_Delay(300);
	}
}


bool MY_BMP180_Init()
{
	uint8_t Datas[22] = {0};

	if(HAL_I2C_IsDeviceReady(&hi2c1, BMP180_READ_REGISTER, 1, 100) == HAL_OK)
	{
		led_alert();
		HAL_I2C_Mem_Read(&hi2c1, BMP180_READ_REGISTER, BMP180_DATA_START_REG, 1, Datas, 22, 100);	//WE READ THE STARTING REGISTERS OF BMP180, WE HAVE 11 REGISTERS
																									//11 * 16bits = 22 * 8bits THIS IS THE REASON OF Datas[22]
		AC1 = (short)(Datas[0] << 8 | Datas[1]);
		AC2 = (short)(Datas[2] << 8 | Datas[3]);
		AC3 = (short)(Datas[4] << 8 | Datas[5]);
		AC4 = (unsigned short)(Datas[6] << 8 | Datas[7]);
		AC5 = (unsigned short)(Datas[8] << 8 | Datas[9]);
		AC6 = (unsigned short)(Datas[10] << 8 | Datas[11]);

		B1 = (short)(Datas[12] << 8 | Datas[13]);
		B2 = (short)(Datas[14] << 8 | Datas[15]);

		MB = (short)(Datas[16] << 8 | Datas[17]);
		MC = (short)(Datas[18] << 8 | Datas[19]);
		MD = (short)(Datas[20] << 8 | Datas[21]);
		return 0;
	}
	return 1;
}


uint16_t MY_BMP180_READ_UNCOMP_TEMP_VALUE()
{
	uint16_t rData[2] = {0};
	uint16_t wData = 0x2E;


	HAL_I2C_Mem_Write(&hi2c1, BMP180_WRITE_REGISTER, 0xF4, 1, &wData, 1, 100);
	HAL_Delay(5);
	HAL_I2C_Mem_Read(&hi2c1, BMP180_READ_REGISTER, 0xF6, 1, &rData, 2, 100);

	return ((rData[0] << 8) + rData[1]);
}

uint32_t MY_BMP180_READ_UNCOMP_PRESSURE_VALUE(int oss)
{
	uint8_t wData = 0x34 + (oss<<6);
	uint8_t rData[3];
	HAL_I2C_Mem_Write(&hi2c1, BMP180_WRITE_REGISTER, 0xF4, 1, &wData, 1, 100);
	switch(oss)
	{
		case 0:
			HAL_Delay(5);
		case 1:
			HAL_Delay(8);
		case 2:
			HAL_Delay(14);
		case 3:
			HAL_Delay(26);
	}

	HAL_I2C_Mem_Read(&hi2c1, BMP180_READ_REGISTER, 0xF6, 1, rData, 3, 100);
	return (((rData[0] << 16) + (rData[1] << 8) + rData[2]) >> (8-oss));
}


double MY_BMP180_READ_TEMPERATURE()
{
	UT = MY_BMP180_READ_UNCOMP_TEMP_VALUE();
	X1 = (UT-AC6) * AC5 / pow(2,15);
	X2 = MC * pow(2,11) / (X1 + MD);
	B5 = X1 + X2;
	real_temp = (B5+8) / 16;
	real_temp *= 0.1;
	return real_temp;

}

double MY_BMP180_READ_PRESSURE(int oss)
{
	UP = MY_BMP180_READ_UNCOMP_PRESSURE_VALUE(oss);

	X1 = ((UT-AC6) * (AC5/(pow(2,15))));
	X2 = ((MC*(pow(2,11))) / (X1+MD));
	B5 = X1+X2;
	B6 = B5-4000;
	X1 = (B2 * (B6*B6/(pow(2,12))))/(pow(2,11));
	X2 = AC2*B6/(pow(2,11));
	X3 = X1+X2;
	B3 = (((AC1*4+X3)<<oss)+2)/4;
	X1 = AC3*B6/pow(2,13);
	X2 = (B1 * (B6*B6/(pow(2,12))))/(pow(2,16));
	X3 = ((X1+X2)+2)/pow(2,2);
	B4 = AC4*(unsigned long)(X3+32768)/(pow(2,15));
	B7 = ((unsigned long)UP-B3)*(50000>>oss);

	if(B7 < 0x80000000)
		real_pressure = (B7 * 2) / B4;
	else
		real_pressure = (B7 / B4) * 2;

	X1 = (real_pressure / pow(2,8)) * (real_pressure / pow(2,8));
	X1 = (X1 * 3038) / pow(2,16);
	X2 = (-7357 * real_pressure) / pow(2,16);
	real_pressure = real_pressure + (X1 + X2 + 3791) / 16;

	return real_pressure;

}

double MY_BMP180_GET_ATM_PRESSURE()
{
	double atm;
	atm = real_pressure / atmPress;
	return atm;
}

double MY_BMP180_GET_ALTITUDE()
{
	double alt;
	alt = 44330 * (1 - pow(MY_BMP180_GET_ATM_PRESSURE(),0.1902949571836346));
	return alt;
}





