/*
 * 32m1_can.h
 *
 *  Created on: 8 дек. 2015 г.
 *      Author: Алексеев
 */

#ifndef RTOS_32M1_CAN_H_
#define RTOS_32M1_CAN_H_

#include <avr/io.h>

#define CAN_A 0
#define CAN_B 1

//длина буффера очереди CAN пакетов
#define CAN_MsgQueueLenght 30

extern uint8_t debugLevel; //уровень выдачи отладочной информации

uint16_t receiveErrorCount; //счетчик ошибок CAN

uint8_t CanInit(uint16_t bitrate, uint8_t standart); //инициализация CAN
uint8_t SendCANmsg(const uint32_t msgID, const uint8_t msg[], const uint8_t msgLength); //поместить сообщение в очередь отправки
extern void ProcessingCANmsg(uint32_t msgID, const uint8_t msg[], uint8_t msgLength); //отработка принятой команды
uint8_t UseCAN(void); //использование CAN в системе, т.е. наличие инициализированного CAN

#endif /* RTOS_32M1_CAN_H_ */
