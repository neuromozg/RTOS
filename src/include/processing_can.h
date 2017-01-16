/*
 * processing_can.h
 *
 *  Created on: 14.04.2016
 *      Author: max
 */

#ifndef PROCESSING_CAN_H_
#define PROCESSING_CAN_H_

#include <avr/io.h>

typedef void (*pCanIdHandler_t)(const uint8_t msgData[], const uint8_t lenght); //прототип функции обработчика CAN пакета

//структура обработчик CAN пакета
typedef struct{
	uint32_t canID; //id пакета
	pCanIdHandler_t handler; //адрес обработчика события, если значение 0 то обработчика нет
} msgHanlerListItem;

extern const uint8_t msgAmount; //количество строк в массиве
extern msgHanlerListItem msgHanlerList[]; //массив обработчиков CAN сообщений

void DeviceCanIdHandler(const uint8_t msgData[], const uint8_t lenght);
void ProcessingCANmsg(const uint32_t msgID, const uint8_t msg[], const uint8_t msgLength);

#endif /* PROCESSING_CAN_H_ */
