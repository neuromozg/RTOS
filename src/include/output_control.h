/*
 * output_control.h
 *
 *  Created on: 18.04.2016
 *      Author: max
 */

#ifndef OUTPUT_CONTROL_H_
#define OUTPUT_CONTROL_H_

#include <avr/io.h>
#include <avr/pgmspace.h>

//массив выходов
typedef struct {
	volatile uint8_t* port;	//порт
	uint8_t bit;	//номер бита в порту
	PGM_P outputName; //имя выхода
} outputListItem;

extern const uint8_t outputAmount; //количество выходов в системе
extern outputListItem outputList[]; //список выходов в системе

void OutputInit(void);
void SetOutputStatus(uint8_t outN, uint8_t status);
uint8_t GetOutputStatus(uint8_t outN);
uint8_t AddOutputList(uint8_t nOutput, volatile uint8_t* port, uint8_t bit, PGM_P outputName);

#endif /* OUTPUT_CONTROL_H_ */
