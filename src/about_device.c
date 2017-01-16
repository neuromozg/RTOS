/*
 * about_device.c
 *
 *  Created on: 26 ����. 2016 �.
 *      Author: ��������
 */
#include <string.h>
#include <stdio.h>
#include "include/about_device.h"
#include "include/32m1_rtos.h"
#include "include/32m1_can.h"

//�������� ������ ����������, ������������ ��� ������������� ��������� �� ���� CAN
void SendAnsverDeviceID(void *data)
{
	uint8_t canBuff[8];

	memcpy(&canBuff[0], &deviceInformation.receiveCanID, sizeof(uint16_t)); //���������� �������� �������������� ����������� CAN ID ��������� ����������
	memcpy(&canBuff[2], &deviceInformation.responseCanID, sizeof(uint16_t)); //���������� �������� �������������� ������������ CAN ID ��������� ����������
	canBuff[4] = deviceInformation.deviceType; //��� ����������
	canBuff[5] = deviceInformation.versionSoftwareHi; //������ ��
	canBuff[6] = deviceInformation.versionSoftwareLo;
	SendCANmsg(AnswerDevice_ID, canBuff, 7);
}

void AnswerDeviceIdHandler(const uint8_t msgData[], uint8_t lenght)
{
	SetTask(SendAnsverDeviceID); //����� ����������
}

void PrintDeviceInfo(void)
{
	printf_P(PSTR("Dev name: %S\n"), deviceInformation.deviceName);
	printf_P(PSTR("Dev type: %u\n"), deviceInformation.deviceType);
	printf_P(PSTR("Soft ver: %u.%u\n"), deviceInformation.versionSoftwareHi, deviceInformation.versionSoftwareLo);
 	printf_P(PSTR("CAN ID 0x%X:0x%X\n"), deviceInformation.receiveCanID, deviceInformation.responseCanID);
}
