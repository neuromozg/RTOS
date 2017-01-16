/*
 * eertos.c
 *
 *  Created on: 29 ���� 2015 �.
 *      Author: ��������
 */
#include <string.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "include/32m1_rtos.h"
#include "include/32m1_uart.h"


//static uint64_t tickCount = 0;
//������� �����, ��������
typedef struct {
	TPTR gotoTask; //��������� �������� �� ������
	void *data;  //��������� �������� ������
} TaskData;

//������� �����
static TaskData taskQueue[TaskQueueSize+1]; //������� �����

typedef struct {
	TPTR gotoTask; //��������� �������� �� ������
	uint16_t time; //�������� � ��
	void *data;  //��������� �������� ������
} TimerData;

//������� RTOS
static TimerData mainTimer[MainTimerQueueSize];

#if defined (__AVR_ATmega32M1__)
ISR(TIMER0_COMPA_vect) //���������� ���������� ������� RTOS //������ �������� ����
{
	TimerService();
	//tickCount++;
}
#endif

#if defined (__AVR_AT90CAN128__)
ISR(TIMER0_COMP_vect) //���������� ���������� ������� RTOS
{
	TimerService();
	//tickCount++;
}
#endif

//rtos ������ ���������� �������
inline void RunRTOS(uint16_t freq)
{
	uint16_t TimerDivider = F_CPU/Prescaler/freq;

#if defined (__AVR_ATmega32M1__)
	TCCR0A = 1 << WGM01; 			//����� ctc, ������� ������������ ��� ���������� �������� ���������
	TCCR0B = 1 << CS00 | 1 << CS01;	//������������ 64
#endif
#if defined (__AVR_AT90CAN128__)
	TCCR0A = 1 << WGM01 | 1 << CS00 | 1 << CS01;	//������������ 64
#endif


	TCNT0 = 0; //��������� �������� ��������
	OCR0A = TimerDivider; //������������� �������� � ������� ���������
	TIMSK0 = 1 << OCIE0A; //��������� ���������� �� ����������

	//sei(); ������ RTOS
	if (UseUART())
		printf_P(PSTR("RTOS %uHz\n"), freq);
}

//RTOS ����������. ������� ��������
inline void InitRTOS(void)
{
	for(uint8_t i = 0; i<(TaskQueueSize+1); i++)//�� ��� ������� ���������� Idle
	{
		taskQueue[i].gotoTask = Idle;
		taskQueue[i].data = NULL;
	}

	for(uint8_t i = 0; i < MainTimerQueueSize; i++)//�������� ��� �������
	{
		mainTimer[i].gotoTask = Idle;
		mainTimer[i].time = 0;
		mainTimer[i].data = NULL;
	}
	if (UseUART())
		printf_P(PSTR("RTOS init.\n"));
}

//������ ��������� - ������� ����
inline void Idle(void *data)
{

}

//������� ��������� ������ � �������. ������������ �������� - ��������� �� �������
//��� ������������ �� ������� �������� RTOS, ��� �� ���� �������� ���������
uint8_t SetTask(TPTR TS){
	return SetTaskParam(TS, NULL);
}

//������� ��������� ������ � �������. ������������ �������� - ��������� �� ������� � ������������ �������� �������
uint8_t SetTaskParam(TPTR TS, void *data)
{
	uint8_t index = 0;
#ifndef TIMERSERVICE_DISABLE_GLOBAL_INTERRUPT
	uint8_t nointerrupted = 0;
#endif
	uint8_t res = 1;

#ifndef TIMERSERVICE_DISABLE_GLOBAL_INTERRUPT
	if(SREG & (1<<SREG_I)) //���� ���������� ���������, ���������
	{
		cli();
		nointerrupted = 1; //������ ����, ��� �� �� � ����������
	}
#endif

	while(taskQueue[index].gotoTask != Idle) //������������� ������� ����� �� ������� ��������� ������
	{
		index++;
		if(index == TaskQueueSize) //���� ������� �����������, �������
		{
			//if(nointerrupted) sei(); //���� �� �� � ����������, ��������� ����������
			res = 0;
			break; //�������
		}
	}

	if (res)
	{
		taskQueue[index].gotoTask = TS; //���������� ������ � �������
		taskQueue[index].data = data;

		taskQueue[index+1].gotoTask = Idle; //���������� ��������� ������� ��������
		taskQueue[index+1].data = NULL;
	}

#ifndef TIMERSERVICE_DISABLE_GLOBAL_INTERRUPT
	if(nointerrupted) sei(); //�������� ����������, ���� �� � ����������� ����������
#endif

	return res;
}

uint8_t SetTimerTask(TPTR TS, const uint16_t time)
{
	return SetTimerTaskParam(TS, time, NULL);
}

//������� ��������� ������ �� �������.������������ ��������� - ��������� �� �������,
//����� �������� � ����� ���������� �������, ������������ ��������
uint8_t SetTimerTaskParam(TPTR TS, const uint16_t time, void *data)
{
#ifndef TIMERSERVICE_DISABLE_GLOBAL_INTERRUPT
	uint8_t nointerrupted = 0;
#endif
	uint8_t tmpTimerIndex = UINT8_MAX;
	uint8_t res = 1;

#ifndef TIMERSERVICE_DISABLE_GLOBAL_INTERRUPT
	if(SREG & (1<<SREG_I)) //�������� ������� ����������, �� ������� ����
	{
		cli();
		nointerrupted = 1;
	}
#endif

	if(time == 0) //���� ����� 0, ����� ����� ������ ������ � �������
	{
		res = SetTaskParam(TS, data);
		//if(nointerrupted) sei(); //��������� ����������
	}
	else
	{
		for(uint8_t i = 0; i < MainTimerQueueSize; i++) //����������� ������� ��������
		{
			if ((mainTimer[i].gotoTask == TS) && (mainTimer[i].data == data)) //���� ��� ���� ������ � ����� ������� � ����������
			{
				tmpTimerIndex = i;
				break; //�������
			}
			else
			{
				//�� ������, ���� � ������ �������� ��� ������ TS, ���������� ������ ������� �� �������� �������
				if((mainTimer[i].gotoTask == Idle) && (tmpTimerIndex == UINT8_MAX))
					tmpTimerIndex = i;
			}
		}

		if (tmpTimerIndex != UINT8_MAX) //���� � ������� ���� �� ���������� ������
		{
			//������ ����� ������
			mainTimer[tmpTimerIndex].gotoTask = TS; //�������� ���� �������� ������
			mainTimer[tmpTimerIndex].time = (time - 1); //���� �������� �������
			mainTimer[tmpTimerIndex].data = data;
			//if(nointerrupted) sei(); //���� �� �� � ����������, ��������� ����������
		}
		else
			res = 0; //��� ������� �������������, ���������� ������ �� �������
	}

#ifndef TIMERSERVICE_DISABLE_GLOBAL_INTERRUPT
	if(nointerrupted) sei(); //��������� ����������
#endif

	return res;
}

// ��������� ����� ��. �������� �� ������� ������ � ���������� �� ����������.
inline void TaskManager(void)
{
	uint8_t index = 0;

	cli(); //��������� ����������
	TPTR gotoTask = taskQueue[0].gotoTask; //����� ������ �������� �� �������
	void *data = taskQueue[0].data; //��������� �� ���������
	sei(); //��������� ����������


	if (gotoTask == Idle) //���� ��� ������ ���
	{
		(Idle)(NULL); //��������� �� ��������� ������� �����
	}
	else
	{
		cli();
		while(taskQueue[index].gotoTask != Idle) //���� ��� �� ����, ������� ��� �������
		{
			//taskQueue[index] = taskQueue[index+1];
			memcpy(&taskQueue[index], &taskQueue[index+1], sizeof(TaskData)); //���������� �������� �������� ������
			index++;
		}
		sei();
//		for(uint8_t i = 0; i < TaskQueueSize; i++) //���� ��� �� ����, ������� ��� �������
//		{
//			//cli(); //��������� ����������
//			//taskQueue[i] = taskQueue[i+1];
//			memcpy(&taskQueue[i], &taskQueue[i+1], sizeof(TaskData)); //���������� �������� �������� ������
//			//sei();
//
//			if (taskQueue[i+1].gotoTask == Idle)
//				break;
//		}
		//taskQueue[TaskQueueSize].gotoTask = Idle; //� ��������� ������ ����� ������� ����
		//taskQueue[TaskQueueSize].prm = 0;

		//sei(); //��������� ����������

		(gotoTask)(data); //��������� � ������ � �������� �� ��������
	}
}

//������ �������� ����. ���������� �� ���������� ��� � 1 ��.
inline void TimerService(void)
{
	for(uint8_t i = 0; i < MainTimerQueueSize; i++) //�������� ��� �������
	{
		if(mainTimer[i].gotoTask == Idle) continue; //���� ����� ������ - ����������

		if(mainTimer[i].time > 0) //���� ������ �� �������� �������, ������ ��� ��������
		{
			mainTimer[i].time--; //��������� ����� � ������, ���� ��� �� �����
		}
		else
		{
			SetTaskParam(mainTimer[i].gotoTask, mainTimer[i].data); //���� ������ ����� �� ����, ���������� � ������� �����
			mainTimer[i].gotoTask = Idle; //� �������������� ������ ����� ������� ����
		}
	}
}

/*
uint64_t GetTickCount(void)
{
	return tickCount;
}
*/
