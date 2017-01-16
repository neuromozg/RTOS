/*
 * online.c
 *
 *  Created on: 26 февр. 2016 г.
 *      Author: Алексеев
 */

#include <stdio.h>
#include <avr/pgmspace.h>
#include "include/32m1_uart.h"
#include "include/online.h"

uint8_t tagOnline; //признак онлайна
uint8_t countOffline = 0; //счетчик пропусков
volatile uint8_t* ledPort;
uint8_t ledMaskBit = 0;
uint8_t useLed = 0; //использование светодиода для индикации
uint16_t pauseIndicationOnline = 100; //задежка в мс для световой индикации при онлайне
uint16_t pauseIndicationOffline = 1000; //задежка в мс для световой индикации при оффлайне

void InitOnlineLed(volatile uint8_t* port, uint8_t bit)
{
	ledPort = port;
	ledMaskBit =  _BV(bit);

	(*(port-1)) |= ledMaskBit; //инициализируем порт на выход, PORTx-1 = DDRx
	useLed = 1; //порт инициализирован
	if (UseUART())
		printf_P(PSTR("Online LED\n"));
}

void IndicationOnLine(void *data)
{
	(*ledPort) ^= ledMaskBit; //инвертируем светодиод

	if (tagOnline)
		SetTimerTask(IndicationOnLine, pauseIndicationOnline); //если онлайн то мигаем быстро
	else
		SetTimerTask(IndicationOnLine, pauseIndicationOffline); //если оффлайн мигаем раз в секунду
}

void ControlOnline(void *data)
{
	if ((countOffline < WaitOnLineSec) && tagOnline)
		countOffline++;
	else
	{
		if (tagOnline)
		{
			tagOnline = FALSE;
			if (UseUART() && (debugLevel > 0))
				printf_P(PSTR("OFFLINE\n"));
			SetTask(OffLineEvent); //вызываем обработчик онлайн события
		}
	}

	SetTimerTask(ControlOnline, pauseIndicationOffline); //перезапуск задачи через 1 сек
}

void AcceptedOnlineLabel(void *data)
{
	countOffline = 0; //сброс счетчика отсутствия связи
	if (tagOnline == FALSE)
	{
		tagOnline = TRUE; //онлайн
		if (UseUART() && (debugLevel > 0))
			printf_P(PSTR("ONLINE\n"));
		SetTask(OnLineEvent); //вызываем обработчик онлайн события
	}
}

void StartControlOnline(const uint16_t freqRTOS)
{
	//вычисляем частоту миганий светодиода в зависимости от частоты RTOS
	pauseIndicationOnline = freqRTOS >> 3; //частота индикации при онлайне равна 1/8 сек
	pauseIndicationOffline = freqRTOS; //частота индикации при оффлайне равна 1 сек

	SetTask(ControlOnline); //запуск задачи контроля соединения с ПО верхнего уровня (Online)

	if (useLed) //если инициализирован выход светодиода
	 SetTask(IndicationOnLine); //запуск задачи индикации соединения (мигаем светодиодом)
	if (UseUART())
		printf_P(PSTR("Control Online\n"));
}

void OnlineLabelCanIdHandler(const uint8_t msgData[], uint8_t lenght) //при приеме онлайн метки в CAN посылке
{
	SetTask(AcceptedOnlineLabel);
}

void OnlineLabelHandler(void) //универсальный обработчик онлайн метки
{
	SetTask(AcceptedOnlineLabel);
}

uint8_t Online(void)
{
	return tagOnline;
}

