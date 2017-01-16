/*
 * output_control.c
 *
 *  Created on: 18.04.2016
 *      Author: max
 */

#include <stdio.h>
#include "include/32m1_uart.h"
#include "include/output_control.h"

//���������� ������ � ������ � ������
uint8_t AddOutputList(uint8_t nOutput, volatile uint8_t* port, uint8_t bit, PGM_P outputName)
{
	if (nOutput < outputAmount)
	{
		outputList[nOutput].port = port;
		outputList[nOutput].bit = bit;
		outputList[nOutput].outputName = outputName;
		return 1;
	}
	else
		return 0;
}

//������������� ������ �� �����
void OutputInit(void)
{
	for (uint8_t i=0; i < outputAmount; i++)
	{
		(*(outputList[i].port-1)) |= (1 << outputList[i].bit); //�������������� ���� �� �����, PORTx-1 = DDRx
	}
	if (UseUART())
		printf_P(PSTR("Outputs: %u\n"), outputAmount);
}

//��������� ��������� ����� �� ������ ������
void SetOutputStatus(uint8_t nOutput, uint8_t status)
{
	//volatile uint8_t* port = outputList[outN].port;

	if (nOutput < outputAmount)
	{
		if (UseUART())
			printf_P(PSTR("OUT %S:"), outputList[nOutput].outputName);

		if (status)
		{
			(*(outputList[nOutput].port)) |= 1<<outputList[nOutput].bit; //������������� ���� � 1
			if (UseUART())
				printf_P(PSTR("1\n"));
		}
		else
		{
			(*(outputList[nOutput].port)) &= ~(1<<outputList[nOutput].bit); //���������� ���� � 0
			if (UseUART())
				printf_P(PSTR("0\n"));
		}
	}
}

//��������� �����
uint8_t GetOutputStatus(uint8_t outN)
{
	return (*(outputList[outN].port-2) & (1<<outputList[outN].bit)); //������� ��������� ���� � ����� PIN, (PORT-2)=PIN
}
