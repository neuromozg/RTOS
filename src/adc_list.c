/*
 * adc_list.c
 *
 *  Created on: 26 ����. 2016 �.
 *      Author: ��������
 */

#include <stdio.h>
#include <string.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "include/32m1_uart.h"
#include "include/adc_list.h"

uint8_t adcCounter = 0; //������� ��� �� ������� adcValueList
uint16_t tmpAdcSumm = 0; //����� �������� ��� ��� ����������
uint8_t averageCounter = 0; //������� ����������� ��������

ISR(ADC_vect)
{
	uint16_t tmpAdc;
	//uint8_t tmp_admux;

	cli();
	tmpAdc = ADCW; //������� ��������������
	sei();

	tmpAdcSumm += tmpAdc; //��������� ��������
	averageCounter++; //���������� �������� ����������

	if (averageCounter == (1 << adcValueList[adcCounter].average)) //���� ������� ����� �������� ������ �������� ����������
	{
		tmpAdc = tmpAdcSumm >> adcValueList[adcCounter].average; //���������� ������������ ��������
		tmpAdcSumm = 0; //��������� �����
		averageCounter = 0; //��������� �������� ����������

		memcpy(adcValueList[adcCounter].pData, &tmpAdc, 2); //���������� �������� �� ��������� � ������� adcValueList

		if (adcValueList[adcCounter].pEvent > 0) //���� ���� ����������
			adcValueList[adcCounter].pEvent();

		adcCounter++; //����������� �������
		if (adcCounter == adcAmount) //���� �������� ����� ������� adcValueList
			adcCounter = 0;

		//�������� ����� ���, �������� ������ � �������
		//tmp_admux = ADMUX;
		ADMUX &= ~(0b00011111);
		//tmp_admux |= (adc_valueList[adc_counter].adc_n << MUX0);
		ADMUX |= (adcValueList[adcCounter].adcChannel << MUX0);
	}

	ADCSRA |= (1 << ADSC); //��������� ��������� ��������� ��������������
}

void InitADC(void)
{
	//------------������������� ��� ------------------------------------------------
#if defined (__AVR_ATmega32M1__)
	ADCSRA = (1 << ADEN) | (1 << ADIE) | (5 << ADPS0); //��� ���, ���������� ��� ���, �������� = 32
	ADCSRB = (1 << AREFEN); //AVcc with external capacitor connected on the AREF pin
#endif
	ADMUX = (1 << REFS0);
	ADMUX |= (adcValueList[adcCounter].adcChannel << MUX0); //����� ��� �� ������� adc_valueList[adc_channel].adc_n

	if (UseUART())
		printf_P(PSTR("ADC: %u\n"), adcAmount);
}


void StartAdcConversion(void)
{
	ADCSRA |= (1 << ADSC); //��������� ��������������, ����� � �������� ��. ���������� ADC_vect
	if (UseUART())
		printf_P(PSTR("ADC start conversion\n"));
}

//uint8_t AddAdcList (const uint8_t nCmd, pHandler_t cmdHandler, const uint8_t prmAmount, PGM_P adcName);
