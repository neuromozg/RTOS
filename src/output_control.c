/*
 * output_control.c
 *
 *  Created on: 18.04.2016
 *      Author: max
 */

#include <stdio.h>
#include "include/32m1_uart.h"
#include "include/output_control.h"

//добавление данных о выходе в список
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

//инициализация портов на выход
void OutputInit(void)
{
	for (uint8_t i=0; i < outputAmount; i++)
	{
		(*(outputList[i].port-1)) |= (1 << outputList[i].bit); //инициализируем порт на выход, PORTx-1 = DDRx
	}
	if (UseUART())
		printf_P(PSTR("Outputs: %u\n"), outputAmount);
}

//установка состояния порта из списка портов
void SetOutputStatus(uint8_t nOutput, uint8_t status)
{
	//volatile uint8_t* port = outputList[outN].port;

	if (nOutput < outputAmount)
	{
		if (UseUART())
			printf_P(PSTR("OUT %S:"), outputList[nOutput].outputName);

		if (status)
		{
			(*(outputList[nOutput].port)) |= 1<<outputList[nOutput].bit; //устанавливаем порт в 1
			if (UseUART())
				printf_P(PSTR("1\n"));
		}
		else
		{
			(*(outputList[nOutput].port)) &= ~(1<<outputList[nOutput].bit); //сбрасываем порт в 0
			if (UseUART())
				printf_P(PSTR("0\n"));
		}
	}
}

//состояние порта
uint8_t GetOutputStatus(uint8_t outN)
{
	return (*(outputList[outN].port-2) & (1<<outputList[outN].bit)); //смотрим состояние бита в порту PIN, (PORT-2)=PIN
}
