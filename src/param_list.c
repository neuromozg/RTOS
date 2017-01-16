/*
 * param_list.c
 *
 *  Created on: 25 ����. 2016 �.
 *      Author: ��������
 */

/*
 * ParamList.c
 *
 *  Created on: 12 ����. 2016 �.
 *      Author: ��������
 */
#include <string.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "include/32m1_can.h"
#include "include/32m1_uart.h"
#include "include/32m1_rtos.h"
#include "include/param_list.h"


void ClearParamList()
{
	//������� ������ ����������
	for(uint8_t i = 0; i<paramAmount; i++)
	{
		paramList[i].pData = NULL;
		paramList[i].size = 0;
		paramList[i].attr = 0;
		paramList[i].prmName = PSTR("");
	}
}

uint8_t AddParamList (const uint8_t paramN, void *pParam, const uint8_t size, PGM_P prmName, const uint8_t attr)
{
	if (paramN < paramAmount)
	{
		if ((size > 0)  && (size <= 6))
		{
			paramList[paramN].pData = pParam;
			paramList[paramN].size = size;
			paramList[paramN].attr = attr;
			paramList[paramN].prmName = prmName;
			return 1;
		}
		else
		{
			if (UseUART() && (debugLevel > 0))
				printf_P(PSTR("ERR: Add paramList PRM:%d wrong size(%u)\n"), paramN, size);
			return 0;
		}

	}
	else
	{
		if (UseUART() && (debugLevel > 0))
			printf_P(PSTR("ERR: Add paramList PRM:%d excess number\n"), paramN);
		return 0;
	}
}

uint8_t UpdateParam (const uint8_t paramN, const uint8_t prmData[], const uint8_t size)
{
	//�������� �� ������� ������� ��������� � ������� � ��� ������� Read/Write
	if ((paramN < paramAmount) && (paramList[paramN].pData != NULL))
	{
		if (paramList[paramN].size == size)
		{
			if (paramList[paramN].attr & (1<<RW))
			{
				//������ ��������
				memcpy(paramList[paramN].pData, prmData, size);

				//���������� �������� ��������
				if (UseUART())
					SendParam(paramN);
				if (UseUART())
					PrintParam(paramN);
				return 1;
			}
			else
			{
				//������ ������� ������ �������� ������ ��� ������
				//PrintTickCount();
				if (UseUART() && (debugLevel > 0))
					printf_P(PSTR("ERR: Set the read only parametr %d\n"), paramN);
				return 0;
			}
		}
		else
		{
			//������ �������� ������ ����������� ���������
			//PrintTickCount();
			if (UseUART() && (debugLevel > 0))
				printf_P(PSTR("ERR: Wrong size parametr %d\n"), paramN);
			return 0;
		}

	}
	else
	{
		//������� ������ �������� ������� �� ���������� � �������
		//��� ����� ����� ���
		//PrintTickCount();
		if (UseUART() && (debugLevel > 0))
			printf_P(PSTR("ERR: Unknown parametr %d\n"), paramN);
		return 0;
	}
}

//�������� �������� ���������� �������
void SendImpParamsRTOS(void *data)
{
	//���������� ��� ��������� ������� � ��������� IMP
	for (uint8_t i=0; i<paramAmount; i++)
	{
		if ((paramList[i].pData != NULL) && (paramList[i].attr & (1<<IMP)))//���� �������� ����� � �������� ��������, ����� ���������� �������� ���������
		{
			if (UseUART())
				SendParam(i);
			if (UseUART())
				PrintParam(i);
		}
	}
}

void SendImpParams()
{
	SetTask(SendImpParamsRTOS);
}

//�������� ��������� � CAN ����, ���������� RTOS
void SendParamRTOS(void *data)
{
	uint8_t canBuff[8];
	uint16_t paramN = (uint16_t)data;

	if ((paramN < paramAmount) && (paramList[paramN].pData != NULL)) //�������� ������� ��������� � ������ ����������
	{
		canBuff[0] = paramN; //����� ���������
		canBuff[1] = paramList[paramN].size; //������ ���������

		cli();
		memcpy(&canBuff[2], paramList[paramN].pData, paramList[paramN].size); //�������� ����� ��������� � ������ CAN ���������
		sei();

		SendCANmsg(deviceInformation.responseCanID, canBuff, (2 + paramList[paramN].size)); //�������� ���������
	}
	else
	{
		canBuff[0] = 255; //ERROR;
		SendCANmsg(deviceInformation.responseCanID, canBuff, 1);
	}
}


void SendParam(uint8_t prm)
{
	uint16_t tmpPrm = prm;
	SetTaskParam(SendParamRTOS, (void*)tmpPrm);
}

//�������� ��������� � ��������
void PrintParamRTOS(void *data)
{
	//char buffer[MaxPrmNameLen];
	//strlcpy_P(buffer, paramList[paramN].prmName, MaxPrmNameLen);
	//PrintTickCount();
	//uint8_t *pTmpArray;
	uint16_t paramN = (uint16_t)data;

	printf_P(PSTR("PRM %u:%S "), paramN, paramList[paramN].prmName);

	switch(paramList[paramN].attr & 0b00001111) //�������� �� �������� ��������� ������ �� ��� ����
	{
		case DT_UINT8:
			printf_P(PSTR("%u"), *(uint8_t*)(paramList[paramN].pData));
			break;

		case DT_INT8:
			printf_P(PSTR("%d"), *(int8_t*)(paramList[paramN].pData));
			break;

		case DT_UINT16:
			printf_P(PSTR("%u"), *(uint16_t*)(paramList[paramN].pData));
			break;

		case DT_INT16:
			printf_P(PSTR("%d"), *(int16_t*)(paramList[paramN].pData));
			break;
		case DT_FLOAT:
			printf_P(PSTR("%.4f"), *(float*)(paramList[paramN].pData));
			break;

		case DT_UINT32:
			printf_P(PSTR("%lu"), *(uint32_t*)(paramList[paramN].pData));
			break;

		case DT_INT32:
			printf_P(PSTR("%ld"), *(int32_t*)(paramList[paramN].pData));
			break;

		case DT_STRING:
			printf_P(PSTR("%s"), *(char*)(paramList[paramN].pData));
			break;

		case DT_PTR:
			printf_P(PSTR("%p"), *(int16_t*)(paramList[paramN].pData));
			break;

		case DT_NONE: case DT_RAW:
			//pTmpArray = (uint8_t*) paramList[paramN].pData;
			//printf_P(PSTR("Param %s:"), buffer);
			for(uint8_t j=0; j<paramList[paramN].size; j++)
			{
				printf_P(PSTR(" [0x%X]"), ((uint8_t*)paramList[paramN].pData)[j]);
				//printf_P(PSTR(" [0x%X]"), pTmpArray[j]);
			}
			break;

		case DT_BITMASK:
			//printf_P(PSTR("Param %s: 0b"), buffer);
			printf_P(PSTR("0b"));
			for(uint8_t j=8; j>0; j--)
			{
				if(*(uint8_t*)(paramList[paramN].pData) & (1<<(j-1)))
					printf_P(PSTR("1"));
				else
					printf_P(PSTR("0"));
			}
			//printf_P(PSTR("\n"));
			break;

	}
	printf_P(PSTR("\n"));
}

void PrintParam(uint8_t prm)
{
	uint16_t tmpPrm = prm;
	SetTaskParam(PrintParamRTOS, (void*)tmpPrm);
}





