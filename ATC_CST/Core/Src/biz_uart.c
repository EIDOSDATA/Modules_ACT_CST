/*
 * biz_uart.c
 *
 *  Created on: Mar 20, 2024
 *      Author: admin
 */

#include "stdio.h"
#include "stdint.h"

#include "biz_uart.h"
#include "biz_main.h"

#include "usart.h"

volatile uint8_t Rx_uart1[UART1_RX_MAXLEN];
volatile uint16_t Rx_uart1_cnt;
volatile uint16_t Rx_uart1_Pcnt;

volatile uint8_t Rx_uart2[UART2_RX_MAXLEN];
volatile uint16_t Rx_uart2_cnt;
volatile uint16_t Rx_uart2_Pcnt;

Uart_Rx_data Rx_Packet;
Uart_Tx_data Tx_Packet;

void Uart_init(void)
{
	Rx_Packet.status = Uart_WAIT;
	Rx_Packet.step = STX_MSB_Check;
	Rx_uart2_cnt = 0;
	Rx_uart2_Pcnt = 0;
}

uint16_t Uart_len_Check(void)
{

	if (Rx_Packet.RecvTime + RECV_WAIT_TIME <= HAL_GetTick())
	{
		if (Rx_uart2_cnt < Rx_uart2_Pcnt)
		{
			return (UART2_RX_MAXLEN - Rx_uart2_Pcnt) + Rx_uart2_cnt;
		}
		else
		{
			return Rx_uart2_cnt - Rx_uart2_Pcnt;
		}

	}
	else
	{
		return 0;
	}

}

void Uart_Tx_Packet(void)
{
	uint8_t Data_Buffer[100];
	uint8_t cnt = 0;

	//STX
	Data_Buffer[cnt++] = 0xFF;
	Data_Buffer[cnt++] = 0x02;
	//ADDR
	Data_Buffer[cnt++] = (Cst.Addr >> 8);
	Data_Buffer[cnt++] = (uint8_t) Cst.Addr;
	//LEN
	Data_Buffer[cnt++] = (Tx_Packet.Data_len >> 8);
	Data_Buffer[cnt++] = (uint8_t) Tx_Packet.Data_len;
	//CMD
	Data_Buffer[cnt++] = (Tx_Packet.CMD >> 8);
	Data_Buffer[cnt++] = (uint8_t) Tx_Packet.CMD;
	//DATA
	if (Tx_Packet.Data_len != 0)
	{
		for (uint8_t cnt2 = 0; cnt2 < Tx_Packet.Data_len; cnt2++)
		{
			Data_Buffer[cnt++] = Tx_Packet.data[cnt2];
		}
	}

	//ETX
	Data_Buffer[cnt++] = 0xFF;
	Data_Buffer[cnt++] = 0x03;

	HAL_GPIO_WritePin(TX_EN_GPIO_Port, TX_EN_Pin, SET);
	HAL_UART_Transmit(&huart2, Data_Buffer, cnt, 100);
	HAL_GPIO_WritePin(TX_EN_GPIO_Port, TX_EN_Pin, RESET);

	Rx_Packet.status = Uart_WAIT;

}

uint8_t Uart_read(void)
{
	uint8_t data = Rx_uart2[Rx_uart2_Pcnt++];
	if (Rx_uart2_Pcnt == UART2_RX_MAXLEN)
		Rx_uart2_Pcnt = 0;

	return data;
}

volatile uint16_t Data_len;
void Uart_Parsser(void)
{
	uint8_t data = 0;
	if (Uart_len_Check() >= RX_MIN_LEN)
	{
//		printf("Loop in\r\n");
		while (Rx_uart2_cnt != Rx_uart2_Pcnt)
		{
			HAL_Delay(10);
			data = Uart_read();
//			printf("data = 0x%02X\r\n",data);
			switch (Rx_Packet.step)
			{
			case STX_MSB_Check:
			{
				if (data == 0xFF)
				{
					Rx_Packet.step = STX_LSB_Check;
					Rx_Packet.status = Uart_ing;
					Data_len = 0;
				}
				break;
			}
			case STX_LSB_Check:
			{
				if (data == 0x02)
				{
					Rx_Packet.step = ADDR_MSB_Check;
				}
				else
				{
					Rx_Packet.step = STX_MSB_Check;
					Rx_Packet.status = Uart_WAIT;
				}
				break;
			}
			case ADDR_MSB_Check:
			{
				Rx_Packet.Addr = data << 8;
				Rx_Packet.step = ADDR_LSB_Check;
				break;
			}
			case ADDR_LSB_Check:
			{
				Rx_Packet.Addr |= data;
				Rx_Packet.step = LEN_MSB_Check;
				break;
			}
			case LEN_MSB_Check:
			{
				Rx_Packet.Data_len = data << 8;
				Rx_Packet.step = LEN_LSB_Check;
				break;
			}
			case LEN_LSB_Check:
			{
				Rx_Packet.Data_len |= data;
				Rx_Packet.step = CMD_MSB_Check;
				break;
			}
			case CMD_MSB_Check:
			{
				Rx_Packet.CMD = data << 8;
				Rx_Packet.step = CMD_LSB_Check;
				break;
			}
			case CMD_LSB_Check:
			{
				Rx_Packet.CMD |= data;

				if (Rx_Packet.Data_len != 0)
				{
					Rx_Packet.step = DATA_Check;
				}
				else
				{
					Rx_Packet.step = ETX_MSB_Check;
				}
				break;
			}

			case DATA_Check:
			{

				Rx_Packet.data[Data_len++] = data;
				if (Data_len == Rx_Packet.Data_len)
				{
					Rx_Packet.step = ETX_MSB_Check;
				}
				break;
			}
			case ETX_MSB_Check:
			{
				if (data == 0xFF)
				{
					Rx_Packet.step = ETX_LSB_Check;

				}
				else
				{
					Rx_Packet.step = STX_MSB_Check;
					Rx_Packet.status = Uart_WAIT;
				}
				break;
			}
			case ETX_LSB_Check:
			{
				if (data == 0x03)
				{
					Rx_Packet.status = Uart_End;
				}
				else
				{
					Rx_Packet.status = Uart_WAIT;
				}
				Rx_Packet.step = STX_MSB_Check;
				break;
			}
			default:
			{
				break;
			}
			}
		}
	}
}

void Uart_Buffer_Debug(void)
{
	if (Rx_uart2_cnt != Rx_uart2_Pcnt)
	{
		printf("Recv Packet = ");
		while (Rx_uart2_cnt != Rx_uart2_Pcnt)
		{
			printf("%c ", Rx_uart2[Rx_uart2_Pcnt++]);
			if (Rx_uart2_cnt == UART2_RX_MAXLEN)
				Rx_uart2_cnt = 0;
		}
		printf("\t End \r\n");

	}
}
