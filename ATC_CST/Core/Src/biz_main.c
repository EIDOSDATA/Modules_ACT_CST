/*
 * biz_main.c
 *
 *  Created on: Mar 20, 2024
 *      Author: admin
 */
#include "stdio.h"
#include "stdint.h"

#include "adc.h"
#include "tim.h"

#include "main.h"
#include "biz_uart.h"
#include "biz_main.h"

#define DUMMY() dummy_fnc();
#define CST_HARDCODING_ADDR 0x0001
Cst_Struct Cst;

#if 0
#define Sensitivity  8
#define Low_Sensitivity  10
#else
#define Sensitivity  9
#define Low_Sensitivity  9
#endif
uint32_t sensor1;
uint32_t sensor2;
uint32_t sensor3;
uint32_t sensor4;

__IO uint16_t adc_buffer_ch[4];

uint32_t sensor_count;
uint8_t ref;

uint8_t dispose;
uint16_t sen_ref1;
uint16_t sen_ref2;
uint16_t sen_ref3;
uint16_t sen_ref4;

uint32_t mean_ch0;
uint32_t mean_ch1;
uint32_t mean_ch2;
uint32_t mean_ch3;


void Motor_On(void)
{
	HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, RESET); //ON
	HAL_GPIO_WritePin(MOTOR_GPIO_Port, MOTOR_Pin, RESET); //ON
}

void Motor_Off(void)
{
	HAL_GPIO_WritePin(MOTOR_GPIO_Port, MOTOR_Pin, SET); //OFF
	HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, SET); //OFF
}

void Dispense(void)
{
	printf(">>Switch1 pushed!!\r\n");
	printf(">>Motor On!!\r\n");
	dispose = 0;
	for (int i = 0; i < 2000; i++)
	{
		Motor_On();
		if (check_dispose())
			break;
	}

	Motor_Off();
	dispose = 0;
	printf(">>Motor Off!!\r\n");

}

void Sensor_On(void)
{
	HAL_GPIO_WritePin(PHOTO_PULSE0_GPIO_Port, PHOTO_PULSE0_Pin, SET); //ON
	HAL_GPIO_WritePin(PHOTO_PULSE1_GPIO_Port, PHOTO_PULSE1_Pin, SET); //ON
	HAL_GPIO_WritePin(PHOTO_PULSE2_GPIO_Port, PHOTO_PULSE2_Pin, SET); //ON
	HAL_GPIO_WritePin(PHOTO_PULSE3_GPIO_Port, PHOTO_PULSE3_Pin, SET); //ON

	HAL_GPIO_WritePin(IR_PULSE0_GPIO_Port, IR_PULSE0_Pin, RESET); //ON
	HAL_GPIO_WritePin(IR_PULSE1_GPIO_Port, IR_PULSE1_Pin, RESET); //ON
	HAL_GPIO_WritePin(IR_PULSE2_GPIO_Port, IR_PULSE2_Pin, RESET); //ON
	HAL_GPIO_WritePin(IR_PULSE3_GPIO_Port, IR_PULSE3_Pin, RESET); //ON

}

void Sensor_Off(void)
{

	HAL_GPIO_WritePin(PHOTO_PULSE0_GPIO_Port, PHOTO_PULSE0_Pin, RESET); //OFF
	HAL_GPIO_WritePin(PHOTO_PULSE1_GPIO_Port, PHOTO_PULSE1_Pin, RESET); //OFF
	HAL_GPIO_WritePin(PHOTO_PULSE2_GPIO_Port, PHOTO_PULSE2_Pin, RESET); //OFF
	HAL_GPIO_WritePin(PHOTO_PULSE3_GPIO_Port, PHOTO_PULSE3_Pin, RESET); //OFF

	HAL_GPIO_WritePin(IR_PULSE0_GPIO_Port, IR_PULSE0_Pin, SET); //OFF
	HAL_GPIO_WritePin(IR_PULSE1_GPIO_Port, IR_PULSE1_Pin, SET); //OFF
	HAL_GPIO_WritePin(IR_PULSE2_GPIO_Port, IR_PULSE2_Pin, SET); //OFF
	HAL_GPIO_WritePin(IR_PULSE3_GPIO_Port, IR_PULSE3_Pin, SET); //OFF

}

void get_sensor(void)
{
	uint16_t number = 20;

	mean_ch0 = 0;
	mean_ch1 = 0;
	mean_ch2 = 0;
	mean_ch3 = 0;

	uint32_t sum_ch0 = 0;
	uint32_t sum_ch1 = 0;
	uint32_t sum_ch2 = 0;
	uint32_t sum_ch3 = 0;

	for (int i = 0; i < number; i++)
	{
		Sensor_On();
		HAL_ADC_Start_DMA(&hadc, (uint32_t*) adc_buffer_ch, 4);
		//HAL_ADC_Start_DMA(&hadc, (uint16_t*) adc_buffer_ch, 4);
		delay_nop(10);
		Sensor_Off();

		sum_ch0 += adc_buffer_ch[0];
		sum_ch1 += adc_buffer_ch[1];
		sum_ch2 += adc_buffer_ch[2];
		sum_ch3 += adc_buffer_ch[3];

		adc_buffer_ch[0] = 0;
		adc_buffer_ch[1] = 0;
		adc_buffer_ch[2] = 0;
		adc_buffer_ch[3] = 0;

		delay_nop(100);
	}

	mean_ch0 = sum_ch0 / number;
	mean_ch1 = sum_ch1 / number;
	mean_ch2 = sum_ch2 / number;
	mean_ch3 = sum_ch3 / number;

	if (ref == 1)
	{
		sen_ref1 = mean_ch0;
		sen_ref2 = mean_ch1;
		sen_ref3 = mean_ch2;
		sen_ref4 = mean_ch3;
	}

}

#if 0
void delay_nop(unsigned int cnt)
#else
int delay_nop(unsigned int cnt)
#endif
{
	for (int i = 0; i < cnt * 2; i++)
	{
		//DUMMY();
		__NOP();
	}
	return 0;
}

void dummy_fnc(void)
{
	//int dummy = 1; // unused data
	//if(dummy == 1) dummy=0;
}

uint8_t check_dispose(void)
{
	if (mean_ch0 > (sen_ref1 + Sensitivity))
	{
		dispose = 1;
		printf(">>sensor1 high detect!!\r\n");
		printf("sen_ref1:%d, sen_ref2:%d, sen_ref3:%d, sen_ref4:%d\r\n", sen_ref1, sen_ref2, sen_ref3, sen_ref4);
		printf("sensor_1:%ld\r\n", mean_ch0);

	}
	else if (mean_ch0 < (sen_ref1 - Low_Sensitivity))
	{
		dispose = 1;
		printf(">>sensor1 low detect!!\r\n");
		printf("sen_ref1:%d, sen_ref2:%d, sen_ref3:%d, sen_ref4:%d\r\n", sen_ref1, sen_ref2, sen_ref3, sen_ref4);
		printf("sensor_1:%ld\r\n", mean_ch0);

	}
	else if (mean_ch1 > sen_ref2 + Sensitivity)
	{
		dispose = 1;
		printf(">>sensor2 high detect!!\r\n");
		printf("sen_ref1:%d, sen_ref2:%d, sen_ref3:%d, sen_ref4:%d\r\n", sen_ref1, sen_ref2, sen_ref3, sen_ref4);
		printf("sensor_2:%ld\r\n", mean_ch1);

	}
	else if (mean_ch1 < sen_ref2 - Low_Sensitivity)
	{
		dispose = 1;
		printf(">>sensor2 low detect!!\r\n");
		printf("sen_ref1:%d, sen_ref2:%d, sen_ref3:%d, sen_ref4:%d\r\n", sen_ref1, sen_ref2, sen_ref3, sen_ref4);
		printf("sensor_2:%ld\r\n", mean_ch1);

	}
	else if (mean_ch2 > sen_ref3 + Sensitivity)
	{
		dispose = 1;
		printf(">>sensor3 high detect!!\r\n");
		printf("sen_ref1:%d, sen_ref2:%d, sen_ref3:%d, sen_ref4:%d\r\n", sen_ref1, sen_ref2, sen_ref3, sen_ref4);
		printf("sensor_3:%ld\r\n", mean_ch2);

	}
	else if (mean_ch2 < sen_ref3 - Low_Sensitivity)
	{
		dispose = 1;
		printf(">>sensor3 low detect!!\r\n");
		printf("sen_ref1:%d, sen_ref2:%d, sen_ref3:%d, sen_ref4:%d\r\n", sen_ref1, sen_ref2, sen_ref3, sen_ref4);
		printf("sensor_3:%ld\r\n", mean_ch2);

	}

	else if (mean_ch3 > sen_ref4 + Sensitivity)
	{
		dispose = 1;
		printf(">>sensor4 high detect!!\r\n");
		printf("sen_ref1:%d, sen_ref2:%d, sen_ref3:%d, sen_ref4:%d\r\n", sen_ref1, sen_ref2, sen_ref3, sen_ref4);
		printf("sensor_4:%ld\r\n", mean_ch3);

	}
	else if (mean_ch3 < sen_ref4 - Sensitivity)
	{
		dispose = 1;
		printf(">>sensor4 low detect!!\r\n");
		printf("sen_ref1:%d, sen_ref2:%d, sen_ref3:%d, sen_ref4:%d\r\n", sen_ref1, sen_ref2, sen_ref3, sen_ref4);
		printf("sensor_4:%ld\r\n", mean_ch3);

	}
	else
	{
		printf("sensor_1:%ld, sensor_2:%ld, sensor_3:%ld, sensor_4:%ld\r\n", mean_ch0, mean_ch1, mean_ch2, mean_ch3);
//		  delay_nop(1);
		HAL_Delay(1);
	}
	return dispose;
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim2)
{
	get_sensor();
}

void Cmd_Runner(void)
{

#if 0
	printf("\r\nPasser END \r\n");
	printf("Addr = 0x%04X \r\n",Rx_Packet.Addr);
	printf("Cmd = 0x%04X \r\n",Rx_Packet.CMD);
	printf("LEN = %d [dec]\r\n",Rx_Packet.Data_len);
	if(Rx_Packet.Data_len != 0)
	{
		printf("Data[HEX] =");
		for(uint8_t cnt = 0 ; cnt <Rx_Packet.Data_len ; cnt++ )
		{
			printf("%02X ",Rx_Packet.data[cnt]);
		}
		printf("\t\t END \r\n");
	}
#endif

	switch (Rx_Packet.CMD)
	{
	case CMD_DISPENSE:
	{
		printf("Dispense go \r\n");

		if (Cst.Cur_status == STU_DISPENSE_ING)
		{
			Tx_Packet.data[0] = 0x00;	//NAK
		}
		else
		{
			Cst.Cur_status = STU_DISPENSE_ING;
			Cst.Target_num = Rx_Packet.data[0];
			Cst.Cur_num = 0;
			dispose = 0;				//개선 필요
			Tx_Packet.data[0] = 0x01;	//ACK
			Motor_On();
		}

		Tx_Packet.CMD = CMD_DISPENSE;
		Tx_Packet.Data_len = 1;

		Uart_Tx_Packet();
		break;
	}
	case CMD_DISPENSE_CHK:
	{
		//printf("Dispense Check \r\n");
		Tx_Packet.CMD = CMD_DISPENSE_CHK;
		Tx_Packet.Data_len = 2;
		Tx_Packet.data[0] = 0x00;
		Tx_Packet.data[1] = 0x02;
		Uart_Tx_Packet();
		break;
	}
	case CMD_STATUS_CHK:
	{
		//printf("Status Check \r\n");
		Tx_Packet.CMD = CMD_STATUS_CHK;
		Tx_Packet.Data_len = 1;
		Tx_Packet.data[0] = Cst.Cur_status;
		Uart_Tx_Packet();

		break;
	}
	}

	//응답 보내는 루틴 추가
}

void Cst_setup(void)
{
	Cst.Addr = 0x0001;			//<-HardCoding
	Cst.Cur_status = STU_WAIT;

	Uart_init();

	sensor_count = 0;
	Sensor_Off();
	//=================================
	HAL_TIM_Base_Start_IT(&htim2);
	ref = 1;
	for (int i = 0; i < 50; i++)
	{
		HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, RESET);			//ON
		printf("sen_ref1:%d, sen_ref2:%d, sen_ref3:%d, sen_ref4:%d\r\n", sen_ref1, sen_ref2, sen_ref3, sen_ref4);

		HAL_Delay(50);
		HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, SET);			//OFF
		HAL_Delay(50);

	}
	ref = 0;
	HAL_TIM_Base_Start_IT(&htim2);

	printf(">>Ready!!\r\n");
	printf("sensor_1:%ld, sensor_2:%ld, sensor_3:%ld, sensor_4:%ld\r\n", mean_ch0, mean_ch1, mean_ch2, mean_ch3);
	//==================================

}

void biz_loop(void)
{
	Cst_setup();

	printf("Biz go\r\n");

	while (1)
	{
		//422 Check
		if (Rx_Packet.status != Uart_End)
		{
			Uart_Parsser();
		}
		else if (Rx_Packet.status == Uart_End)
		{
			if (Rx_Packet.Addr == Cst.Addr)
				Cmd_Runner();
			else
				Rx_Packet.status = Uart_WAIT;
		}

		//배출 관리
		if (Cst.Cur_status == STU_DISPENSE_ING)
		{
			if (check_dispose() == 1)
			{
				Cst.Cur_num++;
				if (Cst.Cur_num != Cst.Target_num)
				{
					printf("keep going\r\n");
					dispose = 0;
					Motor_Off();
					HAL_Delay(10);
					Motor_On();
				}
				else
				{
					Motor_Off();
					Cst.Cur_status = STU_DISPENSE_OK;
				}
			}

		}

	}
	printf("Biz Loop Error.... Reset....3s.... \r\n");
	HAL_Delay(3000);
	HAL_NVIC_SystemReset();

}
