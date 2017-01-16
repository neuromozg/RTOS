/*
 * param_list.h
 *
 *  Created on: 25 февр. 2016 г.
 *      Author: Алексеев
 */

#ifndef SRC_INCLUDE_PARAM_LIST_H_
#define SRC_INCLUDE_PARAM_LIST_H_

#include <avr/pgmspace.h>
#include "about_device.h"

//аттрибуты параметра
#define TYPE0 	0 //тип параметра 4бита
#define TYPE1 	1
#define TYPE2 	2
#define TYPE3 	3
#define RW 		4 //параметр Read/Write, по умолчанию все параметры Read Only
#define IMP 	5 //важный параметр, используется при выводе SendParams
#define SYS 	6 //системный параметр

//типы параметров, используется при задании аттрибута TYPE0..TYPE3
#define DT_NONE 	0
#define DT_UINT8 	1
#define DT_INT8 	2
#define DT_UINT16	3
#define DT_INT16	4
#define DT_UINT32	5
#define DT_INT32	6
#define DT_FLOAT	7
#define DT_STRING	8
#define DT_PTR		9
#define DT_BITMASK	10 //битовая маска,
#define DT_RAW		11 //просто байты

#define MaxPrmNameLen 10 //максимальная длина имени мараметра

extern uint8_t debugLevel; //уровень выдачи отладочной информации

//массив задаваемых параметров
typedef struct {
	void *pData;
	uint8_t size;
	uint8_t attr;
	PGM_P prmName;
} paramListItem;

extern paramListItem paramList[];
extern const uint8_t paramAmount;
extern DeviceInformation_t deviceInformation;

#include <avr/io.h>
#include <avr/pgmspace.h>

void ClearParamList();
uint8_t AddParamList (const uint8_t paramN, void *pParam, const uint8_t size, PGM_P prmName, const uint8_t attr);
uint8_t UpdateParam (const uint8_t paramN, const uint8_t prmData[], const uint8_t size);

//функции вызываемые из RTOS
void SendImpParams(); //отправка всех важных парамеиров (с выставленным флагом IMP)
void SendParam(uint8_t prm); //Отправка параметра paramN по CAN
void PrintParam(uint8_t prm); //Отправка параметра paramN по UART

#endif /* SRC_INCLUDE_PARAM_LIST_H_ */
