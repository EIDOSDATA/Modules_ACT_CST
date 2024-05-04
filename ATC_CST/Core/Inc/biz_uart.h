/*
 * biz_uart.h
 *
 *  Created on: Mar 20, 2024
 *      Author: admin
 */

#ifndef INC_BIZ_UART_H_
#define INC_BIZ_UART_H_

#include "main.h"

#define TX_DATA_SIZE		100
#define RX_DATA_SIZE		500


/* USER CODE BEGIN Prototypes */
#define STX_MSB_Check  				0x01
#define STX_LSB_Check  				0x02
#define ADDR_MSB_Check 				0x03
#define ADDR_LSB_Check 				0x04
#define LEN_MSB_Check 				0x05
#define LEN_LSB_Check 				0x06
#define CMD_MSB_Check 				0x07
#define CMD_LSB_Check 				0x08
#define DATA_Check 					0x09
#define ETX_MSB_Check 				0x0A
#define ETX_LSB_Check 				0x0B

#define Uart_WAIT			0x01
#define Uart_ing			0x02
#define Uart_End			0x03
#define Uart_Fail			0x04
#define Uart_TimeOut		0x05

#define UART1_RX_MAXLEN 	100				//Debug Port
#define UART2_RX_MAXLEN 	1024			//422 Port

#define RX_MIN_LEN 10

#define RECV_WAIT_TIME 5


typedef struct
{
	uint8_t step;
	uint8_t status;
	uint16_t CMD;
	uint16_t Data_len;
	uint16_t Addr;
	uint8_t data[RX_DATA_SIZE];
	uint32_t RecvTime;
}Uart_Rx_data;

extern Uart_Rx_data Packet;




typedef struct
{
	uint16_t CMD;
	uint16_t Data_len;
	uint8_t data[TX_DATA_SIZE];
}Uart_Tx_data;

extern Uart_Tx_data Tx_Packet;
extern volatile uint8_t Rx_uart1[UART1_RX_MAXLEN];
extern volatile uint16_t Rx_uart1_cnt;
extern volatile uint16_t Rx_uart1_Pcnt;

extern volatile uint8_t Rx_uart2[UART2_RX_MAXLEN];
extern volatile uint16_t Rx_uart2_cnt;
extern volatile uint16_t Rx_uart2_Pcnt;

#endif /* INC_BIZ_UART_H_ */
