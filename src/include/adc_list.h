/*
 * adc_list.h
 *
 *  Created on: 26 ����. 2016 �.
 *      Author: ��������
 */

#ifndef SRC_INCLUDE_ADC_LIST_H_
#define SRC_INCLUDE_ADC_LIST_H_

#include <avr/io.h>
#include "32m1_rtos.h"

typedef void (*TPTRadcEvent)(void); //�������� ������� ����������� �������� ���

//��������� ������ ���
typedef struct {
	PGM_P adcName; // �������� ������ ���
	uint8_t adcChannel; //����� ������ ���
	uint8_t average; //���������� (������� ������, �������� 8)
	uint16_t *pData; //����� ������������ ������, ���� ����� ���������� ���������� ��������
	TPTRadcEvent pEvent; //����� ����������� �������, ���� �������� 0 �� ����������� ���
} adcValueListItem;

extern adcValueListItem adcValueList[]; //������ �������� ���
extern const uint8_t adcAmount; //���������� ��������������� ���

void InitADC(void);
void StartAdcConversion(void);

#endif /* SRC_INCLUDE_ADC_LIST_H_ */
