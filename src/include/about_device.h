/*
 * about_device.h
 *
 *  Created on: 26 ����. 2016 �.
 *      Author: ��������
 */

#ifndef SRC_INCLUDE_ABOUT_DEVICE_H_
#define SRC_INCLUDE_ABOUT_DEVICE_H_

#define RequestDevice_ID 0x500   	//����� ���������� �� ����
#define AnswerDevice_ID 0x501   	//����� ID ���������� �� ����

#include <avr/pgmspace.h>

typedef struct
{
	uint8_t deviceType; //��� ����������
	uint8_t versionSoftwareHi; //������ ��
	uint8_t  versionSoftwareLo;
	uint16_t receiveCanID; //ID ������ CAN �������
	uint16_t responseCanID; //ID �������� CAN �������
	PGM_P deviceName;
}DeviceInformation_t;

extern DeviceInformation_t deviceInformation;

void AnswerDeviceIdHandler(const uint8_t msgData[], uint8_t lenght);
void PrintDeviceInfo(void);

#endif /* SRC_INCLUDE_ABOUT_DEVICE_H_ */
