/*
 * adc_list.h
 *
 *  Created on: 26 февр. 2016 г.
 *      Author: Алексеев
 */

#ifndef SRC_INCLUDE_ADC_LIST_H_
#define SRC_INCLUDE_ADC_LIST_H_

#include <avr/io.h>
#include "32m1_rtos.h"

typedef void (*TPTRadcEvent)(void); //прототип функции обработчика значения АЦП

//структура данных АЦП
typedef struct {
	PGM_P adcName; // название канала АЦП
	uint8_t adcChannel; //номер канала АЦП
	uint8_t average; //усреднение (степень двойки, максимум 8)
	uint16_t *pData; //адрес расположения данных, куда будем записывать полученные значения
	TPTRadcEvent pEvent; //адрес обработчика события, если значение 0 то обработчика нет
} adcValueListItem;

extern adcValueListItem adcValueList[]; //массив значений АЦП
extern const uint8_t adcAmount; //количество задействованных АЦП

void InitADC(void);
void StartAdcConversion(void);

#endif /* SRC_INCLUDE_ADC_LIST_H_ */
