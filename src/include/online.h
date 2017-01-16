/*
 * online.h
 *
 *  Created on: 26 ����. 2016 �.
 *      Author: ��������
 */

#ifndef SRC_INCLUDE_ONLINE_H_
#define SRC_INCLUDE_ONLINE_H_

#include "32m1_rtos.h"

#define WaitOnLineSec 3 		//���������� ������ �� ����� ������� ������ ������� ������ ������
#define GetOnLine_ID 0x600		//������ ����� (������ ��������� �� ����� ���� � 3 �������)

extern uint8_t debugLevel;

void InitOnlineLed(volatile uint8_t *port, uint8_t bit); //������������� ����������
void StartControlOnline(const uint16_t freqRTOS); //������ ����� RTOS ��� �������� �����
void OnlineLabelCanIdHandler(const uint8_t msgData[], uint8_t lenght); //���������� ��������� OnLine ��� CAN
void OnlineLabelHandler(void); //������������� ���������� ������ �����
uint8_t Online(void);

//������� ���������� �� RTOS
extern void OffLineEvent(void *data);
extern void OnLineEvent(void *data);

#endif /* SRC_INCLUDE_ONLINE_H_ */
