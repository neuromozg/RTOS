/*
 * about_device.h
 *
 *  Created on: 26 февр. 2016 г.
 *      Author: Алексеев
 */

#ifndef SRC_INCLUDE_ABOUT_DEVICE_H_
#define SRC_INCLUDE_ABOUT_DEVICE_H_

#define RequestDevice_ID 0x500   	//запос устройства на шине
#define AnswerDevice_ID 0x501   	//ответ ID устройства на шине

#include <avr/pgmspace.h>

typedef struct
{
	uint8_t deviceType; //тип устройства
	uint8_t versionSoftwareHi; //версия ПО
	uint8_t  versionSoftwareLo;
	uint16_t receiveCanID; //ID приема CAN пакетов
	uint16_t responseCanID; //ID отправки CAN пакетов
	PGM_P deviceName;
}DeviceInformation_t;

extern DeviceInformation_t deviceInformation;

void AnswerDeviceIdHandler(const uint8_t msgData[], uint8_t lenght);
void PrintDeviceInfo(void);

#endif /* SRC_INCLUDE_ABOUT_DEVICE_H_ */
