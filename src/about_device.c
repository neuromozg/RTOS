/*
 * about_device.c
 *
 *  Created on: 26 февр. 2016 г.
 *      Author: јлексеев
 */
#include <string.h>
#include <stdio.h>
#include "include/about_device.h"
#include "include/32m1_rtos.h"
#include "include/32m1_can.h"

//отправка ответа устройства, используетс€ дл€ идентификации устройств на шине CAN
void SendAnsverDeviceID(void *data)
{
	uint8_t canBuff[8];

	memcpy(&canBuff[0], &deviceInformation.receiveCanID, sizeof(uint16_t)); //записываем значение идентификатора принимаемых CAN ID сообщений устройства
	memcpy(&canBuff[2], &deviceInformation.responseCanID, sizeof(uint16_t)); //записываем значение идентификатора отправл€емых CAN ID сообщений устройства
	canBuff[4] = deviceInformation.deviceType; //тип устройства
	canBuff[5] = deviceInformation.versionSoftwareHi; //верси€ ѕќ
	canBuff[6] = deviceInformation.versionSoftwareLo;
	SendCANmsg(AnswerDevice_ID, canBuff, 7);
}

void AnswerDeviceIdHandler(const uint8_t msgData[], uint8_t lenght)
{
	SetTask(SendAnsverDeviceID); //ответ устройства
}

void PrintDeviceInfo(void)
{
	printf_P(PSTR("Dev name: %S\n"), deviceInformation.deviceName);
	printf_P(PSTR("Dev type: %u\n"), deviceInformation.deviceType);
	printf_P(PSTR("Soft ver: %u.%u\n"), deviceInformation.versionSoftwareHi, deviceInformation.versionSoftwareLo);
 	printf_P(PSTR("CAN ID 0x%X:0x%X\n"), deviceInformation.receiveCanID, deviceInformation.responseCanID);
}
