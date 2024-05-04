/*
 * biz_main.h
 *
 *  Created on: Mar 20, 2024
 *      Author: admin
 */

#ifndef INC_BIZ_MAIN_H_
#define INC_BIZ_MAIN_H_

#define MY_ADDR 1



//=============422 CMD LIST
#define CMD_DISPENSE 		0x0000
#define CMD_DISPENSE_CHK 	0x1000
#define CMD_STATUS_CHK 	    0x1100

//=============CST Status
#define STU_WAIT			0x00
#define STU_DISPENSE_ING	0x01
#define STU_DISPENSE_OK		0x02
#define STU_DISPENSE_ERROR	0x03



typedef struct
{
	uint16_t Addr;
	uint16_t Cur_status;	//카세트 현재 상태
	uint8_t Target_num;		//배출 알약 갯수
	uint8_t Cur_num;		//현재 배출 된 알약
	uint32_t Run_time;		//배출 시작 시간
}Cst_Struct;


extern Cst_Struct Cst;

extern void biz_loop(void);
#endif /* INC_BIZ_MAIN_H_ */
