/*
 * online.h
 *
 *  Created on: 26 февр. 2016 г.
 *      Author: Алексеев
 */

#ifndef SRC_INCLUDE_ONLINE_H_
#define SRC_INCLUDE_ONLINE_H_

#include "32m1_rtos.h"

#define WaitOnLineSec 3 		//количество секунд за время которых должен прибыть сигнал онлайн
#define GetOnLine_ID 0x600		//онлайн метка (должна прилетать не менее раза в 3 секунды)

extern uint8_t debugLevel;

void InitOnlineLed(volatile uint8_t *port, uint8_t bit); //инициализация светодиода
void StartControlOnline(const uint16_t freqRTOS); //запуск задач RTOS для контроля связи
void OnlineLabelCanIdHandler(const uint8_t msgData[], uint8_t lenght); //обработчик сообщения OnLine для CAN
void OnlineLabelHandler(void); //универсальный обработчик онлайн метки
uint8_t Online(void);

//функции вызываемые из RTOS
extern void OffLineEvent(void *data);
extern void OnLineEvent(void *data);

#endif /* SRC_INCLUDE_ONLINE_H_ */
