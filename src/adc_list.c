/*
 * adc_list.c
 *
 *  Created on: 26 февр. 2016 г.
 *      Author: Алексеев
 */

#include <stdio.h>
#include <string.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "include/32m1_uart.h"
#include "include/adc_list.h"

uint8_t adcCounter = 0; //текущий ацп из массива adcValueList
uint16_t tmpAdcSumm = 0; //сумма значений АЦП для усреднения
uint8_t averageCounter = 0; //счетчик усредняемых значений

ISR(ADC_vect)
{
	uint16_t tmpAdc;
	//uint8_t tmp_admux;

	cli();
	tmpAdc = ADCW; //получил преобразование
	sei();

	tmpAdcSumm += tmpAdc; //суммируем значение
	averageCounter++; //увеличение счетчика усреднения

	if (averageCounter == (1 << adcValueList[adcCounter].average)) //если счетчик суммы значений достиг значения усреднения
	{
		tmpAdc = tmpAdcSumm >> adcValueList[adcCounter].average; //вычисление усредненного значения
		tmpAdcSumm = 0; //обнуление суммы
		averageCounter = 0; //обнуление счетчика усреднения

		memcpy(adcValueList[adcCounter].pData, &tmpAdc, 2); //скопировал значение по указателю в массиве adcValueList

		if (adcValueList[adcCounter].pEvent > 0) //если есть обработчик
			adcValueList[adcCounter].pEvent();

		adcCounter++; //увеличиваем счетчик
		if (adcCounter == adcAmount) //если достигли конца массива adcValueList
			adcCounter = 0;

		//изменяем канал АЦП, согласно записи в массиве
		//tmp_admux = ADMUX;
		ADMUX &= ~(0b00011111);
		//tmp_admux |= (adc_valueList[adc_counter].adc_n << MUX0);
		ADMUX |= (adcValueList[adcCounter].adcChannel << MUX0);
	}

	ADCSRA |= (1 << ADSC); //запускаем очередное одиночное преобразование
}

void InitADC(void)
{
	//------------ИНИЦИАЛИЗАЦИЯ АЦП ------------------------------------------------
#if defined (__AVR_ATmega32M1__)
	ADCSRA = (1 << ADEN) | (1 << ADIE) | (5 << ADPS0); //АЦП вкл, прерывания ацп вкл, делитель = 32
	ADCSRB = (1 << AREFEN); //AVcc with external capacitor connected on the AREF pin
#endif
	ADMUX = (1 << REFS0);
	ADMUX |= (adcValueList[adcCounter].adcChannel << MUX0); //канал АЦП из массива adc_valueList[adc_channel].adc_n

	if (UseUART())
		printf_P(PSTR("ADC: %u\n"), adcAmount);
}


void StartAdcConversion(void)
{
	ADCSRA |= (1 << ADSC); //одиночное преобразование, далее в автомате см. прерывание ADC_vect
	if (UseUART())
		printf_P(PSTR("ADC start conversion\n"));
}

//uint8_t AddAdcList (const uint8_t nCmd, pHandler_t cmdHandler, const uint8_t prmAmount, PGM_P adcName);
