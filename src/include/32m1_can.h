/*
 * 32m1_can.h
 *
 *  Created on: 8 ���. 2015 �.
 *      Author: ��������
 */

#ifndef RTOS_32M1_CAN_H_
#define RTOS_32M1_CAN_H_

#include <avr/io.h>

#define CAN_A 0
#define CAN_B 1

//����� ������� ������� CAN �������
#define CAN_MsgQueueLenght 30

extern uint8_t debugLevel; //������� ������ ���������� ����������

uint16_t receiveErrorCount; //������� ������ CAN

uint8_t CanInit(uint16_t bitrate, uint8_t standart); //������������� CAN
uint8_t SendCANmsg(const uint32_t msgID, const uint8_t msg[], const uint8_t msgLength); //��������� ��������� � ������� ��������
extern void ProcessingCANmsg(uint32_t msgID, const uint8_t msg[], uint8_t msgLength); //��������� �������� �������
uint8_t UseCAN(void); //������������� CAN � �������, �.�. ������� ������������������� CAN

#endif /* RTOS_32M1_CAN_H_ */
