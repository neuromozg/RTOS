/*
 * cmd_list.h
 *
 *  Created on: 25 ����. 2016 �.
 *      Author: ��������
 */

#ifndef SRC_INCLUDE_CMD_LIST_H_
#define SRC_INCLUDE_CMD_LIST_H_

#define MaxCmdNameLen 20 //������������ ����� ����� �������
#define CmdStartNumber 200 //����� � �������� ���������� ������ ������

#include <avr/io.h>
#include <avr/pgmspace.h>

typedef void (*pHandler_t)(const uint8_t cmdData[]); //�������� ������� ����������� �������

//������ �������
typedef struct {
	pHandler_t cmdHandler; // ���������� �������
	PGM_P cmdName; // ��� �������
	uint8_t amountPrm; //���������� ���� ����������
} cmdListItem;

extern uint8_t debugLevel; //������� ������ ���������� ����������
extern cmdListItem cmdList[];
extern const uint8_t cmdAmount;
//extern const uint16_t responseCanID;

void ClearCmdList();
uint8_t AddCmdList (const uint8_t nCmd, pHandler_t cmdHandler, const uint8_t prmAmount, PGM_P cmdName);
uint8_t RunCommand(const uint8_t nCmd, const uint8_t cmdPrm[], const uint8_t prmAmount);

//������� ���������� �� RTOS
void PrintExecCmd(void *data);

#endif /* SRC_INCLUDE_CMD_LIST_H_ */
