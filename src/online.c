/*
 * online.c
 *
 *  Created on: 26 ����. 2016 �.
 *      Author: ��������
 */

#include <stdio.h>
#include <avr/pgmspace.h>
#include "include/32m1_uart.h"
#include "include/online.h"

uint8_t tagOnline; //������� �������
uint8_t countOffline = 0; //������� ���������
volatile uint8_t* ledPort;
uint8_t ledMaskBit = 0;
uint8_t useLed = 0; //������������� ���������� ��� ���������
uint16_t pauseIndicationOnline = 100; //������� � �� ��� �������� ��������� ��� �������
uint16_t pauseIndicationOffline = 1000; //������� � �� ��� �������� ��������� ��� ��������

void InitOnlineLed(volatile uint8_t* port, uint8_t bit)
{
	ledPort = port;
	ledMaskBit =  _BV(bit);

	(*(port-1)) |= ledMaskBit; //�������������� ���� �� �����, PORTx-1 = DDRx
	useLed = 1; //���� ���������������
	if (UseUART())
		printf_P(PSTR("Online LED\n"));
}

void IndicationOnLine(void *data)
{
	(*ledPort) ^= ledMaskBit; //����������� ���������

	if (tagOnline)
		SetTimerTask(IndicationOnLine, pauseIndicationOnline); //���� ������ �� ������ ������
	else
		SetTimerTask(IndicationOnLine, pauseIndicationOffline); //���� ������� ������ ��� � �������
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
			SetTask(OffLineEvent); //�������� ���������� ������ �������
		}
	}

	SetTimerTask(ControlOnline, pauseIndicationOffline); //���������� ������ ����� 1 ���
}

void AcceptedOnlineLabel(void *data)
{
	countOffline = 0; //����� �������� ���������� �����
	if (tagOnline == FALSE)
	{
		tagOnline = TRUE; //������
		if (UseUART() && (debugLevel > 0))
			printf_P(PSTR("ONLINE\n"));
		SetTask(OnLineEvent); //�������� ���������� ������ �������
	}
}

void StartControlOnline(const uint16_t freqRTOS)
{
	//��������� ������� ������� ���������� � ����������� �� ������� RTOS
	pauseIndicationOnline = freqRTOS >> 3; //������� ��������� ��� ������� ����� 1/8 ���
	pauseIndicationOffline = freqRTOS; //������� ��������� ��� �������� ����� 1 ���

	SetTask(ControlOnline); //������ ������ �������� ���������� � �� �������� ������ (Online)

	if (useLed) //���� ��������������� ����� ����������
	 SetTask(IndicationOnLine); //������ ������ ��������� ���������� (������ �����������)
	if (UseUART())
		printf_P(PSTR("Control Online\n"));
}

void OnlineLabelCanIdHandler(const uint8_t msgData[], uint8_t lenght) //��� ������ ������ ����� � CAN �������
{
	SetTask(AcceptedOnlineLabel);
}

void OnlineLabelHandler(void) //������������� ���������� ������ �����
{
	SetTask(AcceptedOnlineLabel);
}

uint8_t Online(void)
{
	return tagOnline;
}

