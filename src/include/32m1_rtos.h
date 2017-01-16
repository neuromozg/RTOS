/*
 * eertos.h
 *
 *  Created on: 29 ���� 2015 �.
 *      Author: ��������
 */

#ifndef LIB_32M1_RTOS_H_
#define LIB_32M1_RTOS_H_

#include <avr/io.h>

//#define TIMERSERVICE_DISABLE_GLOBAL_INTERRUPT  //������������� ������� ���������� ���������� � ��������� ������ RTOS
#define Prescaler 64 //�������� ������� ������� ������������� ��� ������� RTOS

//����� ������� �����
#ifndef TaskQueueSize
#define TaskQueueSize 30
#endif

//���������� �������� RTOS
#ifndef MainTimerQueueSize
#define MainTimerQueueSize 6
#endif

#ifndef F_CPU
#warning "F_CPU not defined for <32m1_rtos.h>"
#define F_CPU 16000000UL //�������� �������
#endif

#define HI(x) ((x)>>8)	//������� ������ 8-������� �����
#define LO(x) ((x)&0xFF) //������� ������ 8-������� �����?0
#define TRUE 1
#define FALSE 0

typedef void (*TPTR)(void *data); //�������� ������� ����, �������� ������� ��������� (2 �����)

extern void RunRTOS(uint16_t freq);

extern void InitRTOS(void);
extern void Idle(void *data);

extern uint8_t SetTask(TPTR TS);
extern uint8_t SetTaskParam(TPTR TS, void *data);
extern uint8_t SetTimerTask(TPTR TS, const uint16_t time);
extern uint8_t SetTimerTaskParam(TPTR TS, const uint16_t time, void *data);

extern void TaskManager(void);
extern void TimerService(void);
//extern uint64_t GetTickCount(void);

//RTOS Errors ���� �� ������������.
//#define TaskSetOk			 'A'
//#define TaskQueueOverflow	 'B'
//#define TimerUpdated		 'C'
//#define TimerSetOk		 'D'
//#define TimerOverflow		 'E'

#endif /* LIB_32M1_RTOS_H_ */
