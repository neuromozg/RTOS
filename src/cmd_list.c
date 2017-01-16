/*
 * cmd_list.c
 *
 *  Created on: 25 ����. 2016 �.
 *      Author: ��������
 */

#include <string.h>
#include <stdio.h>
#include "include/32m1_rtos.h"
#include "include/32m1_uart.h"
#include "include/cmd_list.h"


void ClearCmdList()
{
	//������� ������ ����������
	for(uint8_t i = 0; i < cmdAmount; i++)
	{
		cmdList[i].cmdHandler = NULL;
		cmdList[i].cmdName = PSTR("");
		cmdList[i].amountPrm = 0;
	}
}

uint8_t AddCmdList(const uint8_t cmdN, pHandler_t cmdHandler, const uint8_t prmAmount, PGM_P cmdName)
{
	if ((cmdN >= CmdStartNumber) && (cmdN < (CmdStartNumber+cmdAmount)))
	{
		if (prmAmount < 8)
		{
			cmdList[cmdN - CmdStartNumber].cmdHandler = cmdHandler; //����������
			cmdList[cmdN - CmdStartNumber].amountPrm = prmAmount; //���������� ����������
			cmdList[cmdN - CmdStartNumber].cmdName = cmdName; //���
			return 1;
		}
		else
		{
			if (UseUART() && (debugLevel > 0))
				printf_P(PSTR("ERR: Add cmdList CMD:%d wrong number of arguments(%u)\n"), cmdN, prmAmount);
			return 0;
		}
	}
	else
	{
		if (UseUART() && (debugLevel > 0))
			printf_P(PSTR("ERR: Add cmdList CMD:%d excess number\n"), cmdN);
		return 0;
	}
}

uint8_t RunCommand(const uint8_t cmdN, const uint8_t cmdPrm[], const uint8_t amountPrm)
{
	if (((cmdN - CmdStartNumber) < cmdAmount) && //����� ������� � �������� ������ ������
		(cmdList[cmdN - CmdStartNumber].cmdHandler != NULL)) //���� ��� ������ ���� ����������
		if (amountPrm == cmdList[cmdN - CmdStartNumber].amountPrm) //���� ���������� ������������ ���������� ��������� � �������� �����������
		{
			if (UseUART() && (debugLevel > 0))
			{
				uint16_t tmpCmd = cmdN;
				SetTaskParam(PrintExecCmd, (void*)tmpCmd);
			}
			cmdList[cmdN - CmdStartNumber].cmdHandler(cmdPrm); //��������� ����������
			return 1;
		}
		else
		{
			//�� ������������ ���������� ������������ ����������
			if (UseUART() && (debugLevel > 0))
				printf_P(PSTR("ERR: Execute CMD:%u wrong number of arguments(%u)\n"), cmdN, amountPrm);
			return 0;
		}
	else
	{
		//����������� �������
		if (UseUART() && (debugLevel > 0))
			printf_P(PSTR("ERR: Execute CMD:%u unknown command\n"), cmdN);
		return 0;
	}
}

void PrintExecCmd(void *cmdN)
{
	printf_P(PSTR("Execute CMD:%u (%S)\n"), (uint16_t)cmdN, cmdList[(uint16_t)cmdN - CmdStartNumber].cmdName);
}
