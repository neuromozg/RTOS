/*
 * 32m1_uart.c

 *
 *  Created on: 15 ���. 2015 �.
 *      Author: ��������
 */

#include <stdio.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include "include/32m1_uart.h"


char buffer[UINT8_MAX+1]; //������ �������� (������������ �������� 256), ��� ������� ������������ ��� �������� ���������(����������) ���������� � ��������

uint8_t buffStartIndex = 0;
uint8_t buffEndIndex = 0;
uint8_t tagSending = 0;
uint8_t tagOverlap = 0; //������� �������� ������� ������� ����� ������������ ���������

//������������� ���������� UART � �������
uint8_t useUART = 0;

#if defined (__AVR_AT90CAN128__)
char bufferSecond[UINT8_MAX+1]; //��������� ������ �������� (������������ �������� 256), ��� �������� ���������� �������� ������

uint8_t buffSecondStartIndex = 0;
uint8_t buffSecondEndIndex = 0;
uint8_t tagSecondSending = 0;
uint8_t tagSecondOverlap = 0; //������� �������� ������� ���������� ������� ����� ������������ ���������

//������������� ������� ���������� UART � �������
uint8_t useSecondUART = 0;

ISR (USART0_UDRE_vect) //��������� ���������� �� ����������� �������
{
	if ((buffSecondStartIndex == buffSecondEndIndex) && (tagSecondOverlap == 0)) //���� ��������� ������ ������ ��������� � ��� ���� ���� ����������
	{
		//�������� ��������
		tagSecondSending = 0;
		UCSR0B &= ~(1<<UDRIE0); 	//������ ���������� �� �����������
	}
	else
	{
		UDR0 = bufferSecond[buffSecondStartIndex]; // �������� ������ �� ������
		buffSecondStartIndex++;
		if ((buffSecondStartIndex == 0) && tagSecondOverlap) //���� ��������� ��������� ������� � ���������� ������� �������� �����
			tagSecondOverlap = 0; //����� �������� ��������
	}
}
#endif


#if defined (__AVR_ATmega32M1__)
void PushCharBufferLINDAT(void);

ISR (LIN_TC_vect)
{
	if(LINSIR & (1<<LTXOK)) //��������� ����������� TX
	{
		if ((buffStartIndex == buffEndIndex) && (tagOverlap == 0)) //���� ��������� ������ ������� ��������� � ��� ���� ���� ����������
		{
			//�������� ��������
			tagSending = 0;
			LINSIR |= (1<<LTXOK); //�������������� ����� ����� LTXOK
		}
		else
		{
			PushCharBufferLINDAT(); //�������� ���� �� ������� � ������� LINDAT
		}
	}
}

void PushCharBufferLINDAT(void)
{
	LINDAT = buffer[buffStartIndex];
	buffStartIndex++;
	if ((buffStartIndex == 0) && tagOverlap) //���� ��������� ��������� ������� � ���������� ������� �������� �����
		tagOverlap = 0; //����� �������� ��������
}
#endif

#if defined (__AVR_AT90CAN128__)
ISR (USART1_UDRE_vect) //��������� ����������
{
	if ((buffStartIndex == buffEndIndex) && (tagOverlap == 0)) //���� ��������� ������ ������ ��������� � ��� ���� ���� ����������
	{
		//�������� ��������
		tagSending = 0;
		UCSR1B &= ~(1<<UDRIE1); 	//������ ���������� �� �����������
	}
	else
	{
		UDR1 = buffer[buffStartIndex]; // �������� ������ �� ������
		buffStartIndex++;
		if ((buffStartIndex == 0) && tagOverlap) //���� ��������� ��������� ������� � ���������� ������� �������� �����
			tagOverlap = 0; //����� �������� ��������
	}
}
#endif

int UartPutChar(char c, FILE *stream)
{
	if (c == '\n')
		UartPutChar('\r', stream);

	//�������� �� ������������ �������
	if ((buffEndIndex == buffStartIndex) && tagOverlap)
	{
		//������ ����������, �������� ����� ����������
		return 1;
	}
	else
	{
		buffer[buffEndIndex] = c;
		buffEndIndex++;
		if (buffEndIndex == 0)
			tagOverlap = 1; //������� ������� ����� ������� ����� 255,

		if (tagSending == 0) //���� � ������ ������ �������� ������ �� �������, ����� ������������� ��������� 1-� ����
		{
			tagSending = 1; //������� ��������
#if defined (__AVR_ATmega32M1__)
			PushCharBufferLINDAT(); //�������� ���� �� ������� � ������� LINDAT
#endif
#if defined (__AVR_AT90CAN128__)
			UCSR1B |= 1<<UDRIE1;	// ��������� �������� ���� ��� � �������
#endif
		}
		return 0;
	}
}

#if defined (__AVR_AT90CAN128__)
int SecondUartPutChar(char c)
{
	//�������� �� ������������ �������
	if ((buffSecondEndIndex == buffSecondStartIndex) && tagSecondOverlap)
	{
		//������ ����������, �������� ����� ����������
		return 1;
	}
	else
	{
		bufferSecond[buffSecondEndIndex] = c;
		buffSecondEndIndex++;
		if (buffSecondEndIndex == 0)
			tagSecondOverlap = 1; //������� ������� ����� ������� ����� 255,

		if (tagSecondSending == 0) //���� � ������ ������ �������� ������ �� �������, ����� ������������� ��������� 1-� ����
		{
			tagSecondSending = 1; //������� ��������

			UCSR0B |= 1<<UDRIE0;	// ��������� �������� ���� ��� � �������

		}
		return 0;
	}
}
#endif

void UartInit(uint32_t baudrate)
{
#if defined (__AVR_ATmega32M1__)
	LINCR = (1 << LSWRES); //Reset
	LINCR = (1<<LCMD2)|(0<<LCMD1)|(1<<LCMD0); // enable UART TX(1x1), Full Duplex mode(111)
	LINBTR = (1 << LDISR) | SAMPLES_PER_BIT; // set samples per bit, disable re-syncronization
	LINBRR = ((F_CPU / SAMPLES_PER_BIT) / baudrate) - 1; // set baudrate
	LINENIR = (1<<LENTXOK); // Transmit interrupt is enable
	LINCR |= (1<<LENA); // UART is enable, Tx and Rx are enabled too
#endif
#if defined (__AVR_AT90CAN128__)

	UBRR1 = F_CPU/(16*baudrate)-1;
	UCSR1A = 0; 						// ����� ���������� ������/�������� � ������
	UCSR1B = (1<<TXEN1); 				// ���������� ��������
	UCSR1C = (1<<UCSZ11) | (1<<UCSZ10);	// ����������� 8-������ �������
#endif

	useUART = 1;

	printf_P(PSTR("UART %lu 8-N-1\n"), baudrate);
}

#if defined (__AVR_AT90CAN128__)
void UartSecondaryInit(uint32_t baudrate)	// ������������� UART 1-�� ������
{
	UBRR0 = F_CPU/(16*baudrate)-1; 		// ���������� �������� �������� ��������
	UCSR0A = 0; 						// ����� ���������� ������/�������� � ������
	UCSR0B = (1<<TXEN0); 				// ���������� ��������
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00); // ����������� 8-data bit, Parity: none, stop bit 1


	useSecondUART = 1;

	printf_P(PSTR("SecondUART %lu 8-N-1\n"), baudrate);
}

uint8_t UseSecondUART(void)
{
	return useSecondUART;
}

void UartSecondaryRxEnable(void) //���������� ����� ���� �� USART0
{
	if (useSecondUART) //���� UART ���������������, ����� ���������� ����� ����
		UCSR0B |= (1<<RXCIE0) | (1<<RXEN0); //��������� ���������� �� ����� � ��������� �����

	printf_P(PSTR("SecondUART Rx enable\n"));
}

ISR (USART0_RX_vect) //���������� ���������� �� ������ ����� USART0
{
	Usart0RxHandler(UDR0); //�������� �������� ���� �� ���������
}

#endif

uint8_t UseUART(void)
{
	return useUART;
}

void UartWriteByte(uint8_t data)
{
#if defined (__AVR_ATmega32M1__)
	LINDAT = data;
	while (!(LINSIR & (1 << LTXOK)));
#endif

#if defined (__AVR_AT90CAN128__)
	while (!(UCSR0A & (1<<UDRE0)));
	UDR0 = data;
#endif
}

void UartWriteData(uint8_t data[], uint8_t lenght)
{
	for(uint8_t i=0; i<lenght; i++)
		UartWriteByte(data[i]);
}
