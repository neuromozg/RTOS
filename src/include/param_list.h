/*
 * param_list.h
 *
 *  Created on: 25 ����. 2016 �.
 *      Author: ��������
 */

#ifndef SRC_INCLUDE_PARAM_LIST_H_
#define SRC_INCLUDE_PARAM_LIST_H_

#include <avr/pgmspace.h>
#include "about_device.h"

//��������� ���������
#define TYPE0 	0 //��� ��������� 4����
#define TYPE1 	1
#define TYPE2 	2
#define TYPE3 	3
#define RW 		4 //�������� Read/Write, �� ��������� ��� ��������� Read Only
#define IMP 	5 //������ ��������, ������������ ��� ������ SendParams
#define SYS 	6 //��������� ��������

//���� ����������, ������������ ��� ������� ��������� TYPE0..TYPE3
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
#define DT_BITMASK	10 //������� �����,
#define DT_RAW		11 //������ �����

#define MaxPrmNameLen 10 //������������ ����� ����� ���������

extern uint8_t debugLevel; //������� ������ ���������� ����������

//������ ���������� ����������
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

//������� ���������� �� RTOS
void SendImpParams(); //�������� ���� ������ ���������� (� ������������ ������ IMP)
void SendParam(uint8_t prm); //�������� ��������� paramN �� CAN
void PrintParam(uint8_t prm); //�������� ��������� paramN �� UART

#endif /* SRC_INCLUDE_PARAM_LIST_H_ */
