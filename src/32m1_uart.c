/*
 * 32m1_uart.c

 *
 *  Created on: 15 дек. 2015 г.
 *      Author: Алексеев
 */

#include <stdio.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include "include/32m1_uart.h"


char buffer[UINT8_MAX+1]; //буффер отправки (максимальное значение 256), как правило используется для отправки текстовой(отладочной) информации в терминал

uint8_t buffStartIndex = 0;
uint8_t buffEndIndex = 0;
uint8_t tagSending = 0;
uint8_t tagOverlap = 0; //признак перехода индекса буффера через максимальное знначение

//использование интерфейса UART в системе
uint8_t useUART = 0;

#if defined (__AVR_AT90CAN128__)
char bufferSecond[UINT8_MAX+1]; //вторичный буффер отправки (максимальное значение 256), для бинарный протоколов передачи данных

uint8_t buffSecondStartIndex = 0;
uint8_t buffSecondEndIndex = 0;
uint8_t tagSecondSending = 0;
uint8_t tagSecondOverlap = 0; //признак перехода индекса вторичного буффера через максимальное знначение

//использование второго интерфейса UART в системе
uint8_t useSecondUART = 0;

ISR (USART0_UDRE_vect) //обработка прерывания по опустошению буффера
{
	if ((buffSecondStartIndex == buffSecondEndIndex) && (tagSecondOverlap == 0)) //если начальный индекс достиг конечного и при этом нету перехлеста
	{
		//передача окончена
		tagSecondSending = 0;
		UCSR0B &= ~(1<<UDRIE0); 	//запрет прерывания по опустошению
	}
	else
	{
		UDR0 = bufferSecond[buffSecondStartIndex]; // отсылаем данные из буфера
		buffSecondStartIndex++;
		if ((buffSecondStartIndex == 0) && tagSecondOverlap) //если произошел перехлест индекса и установлен признак перехода тогда
			tagSecondOverlap = 0; //сброс признака перехода
	}
}
#endif


#if defined (__AVR_ATmega32M1__)
void PushCharBufferLINDAT(void);

ISR (LIN_TC_vect)
{
	if(LINSIR & (1<<LTXOK)) //сработало прерыванеие TX
	{
		if ((buffStartIndex == buffEndIndex) && (tagOverlap == 0)) //если начальный индекс досктиг конечного и при этом нету перехлеста
		{
			//передача окончена
			tagSending = 0;
			LINSIR |= (1<<LTXOK); //принудительный сброс флага LTXOK
		}
		else
		{
			PushCharBufferLINDAT(); //помещаем байт из буффера в регистр LINDAT
		}
	}
}

void PushCharBufferLINDAT(void)
{
	LINDAT = buffer[buffStartIndex];
	buffStartIndex++;
	if ((buffStartIndex == 0) && tagOverlap) //если произошел перехлест индекса и установлен признак перехода тогда
		tagOverlap = 0; //сброс признака перехода
}
#endif

#if defined (__AVR_AT90CAN128__)
ISR (USART1_UDRE_vect) //обработка прерывания
{
	if ((buffStartIndex == buffEndIndex) && (tagOverlap == 0)) //если начальный индекс достиг конечного и при этом нету перехлеста
	{
		//передача окончена
		tagSending = 0;
		UCSR1B &= ~(1<<UDRIE1); 	//запрет прерывания по опустошению
	}
	else
	{
		UDR1 = buffer[buffStartIndex]; // отсылаем данные из буфера
		buffStartIndex++;
		if ((buffStartIndex == 0) && tagOverlap) //если произошел перехлест индекса и установлен признак перехода тогда
			tagOverlap = 0; //сброс признака перехода
	}
}
#endif

int UartPutChar(char c, FILE *stream)
{
	if (c == '\n')
		UartPutChar('\r', stream);

	//проверка на переполнение буффера
	if ((buffEndIndex == buffStartIndex) && tagOverlap)
	{
		//буффер переполнен, отправка байта невозможна
		return 1;
	}
	else
	{
		buffer[buffEndIndex] = c;
		buffEndIndex++;
		if (buffEndIndex == 0)
			tagOverlap = 1; //переход индекса конца буффера через 255,

		if (tagSending == 0) //если в данный момент отправка данных не ведется, тогда принудительно отправляю 1-й байт
		{
			tagSending = 1; //признак передачи
#if defined (__AVR_ATmega32M1__)
			PushCharBufferLINDAT(); //помещаем байт из буффера в регистр LINDAT
#endif
#if defined (__AVR_AT90CAN128__)
			UCSR1B |= 1<<UDRIE1;	// разрешаем отправку если все в порядке
#endif
		}
		return 0;
	}
}

#if defined (__AVR_AT90CAN128__)
int SecondUartPutChar(char c)
{
	//проверка на переполнение буффера
	if ((buffSecondEndIndex == buffSecondStartIndex) && tagSecondOverlap)
	{
		//буффер переполнен, отправка байта невозможна
		return 1;
	}
	else
	{
		bufferSecond[buffSecondEndIndex] = c;
		buffSecondEndIndex++;
		if (buffSecondEndIndex == 0)
			tagSecondOverlap = 1; //переход индекса конца буффера через 255,

		if (tagSecondSending == 0) //если в данный момент отправка данных не ведется, тогда принудительно отправляю 1-й байт
		{
			tagSecondSending = 1; //признак передачи

			UCSR0B |= 1<<UDRIE0;	// разрешаем отправку если все в порядке

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
	UCSR1A = 0; 						// флаги завершения приема/передачи и ошибок
	UCSR1B = (1<<TXEN1); 				// разрешение передачи
	UCSR1C = (1<<UCSZ11) | (1<<UCSZ10);	// стандартная 8-битная посылка
#endif

	useUART = 1;

	printf_P(PSTR("UART %lu 8-N-1\n"), baudrate);
}

#if defined (__AVR_AT90CAN128__)
void UartSecondaryInit(uint32_t baudrate)	// инициализация UART 1-го канала
{
	UBRR0 = F_CPU/(16*baudrate)-1; 		// записываем значение скорости передачи
	UCSR0A = 0; 						// флаги завершения приема/передачи и ошибок
	UCSR0B = (1<<TXEN0); 				// разрешение передачи
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00); // стандартная 8-data bit, Parity: none, stop bit 1


	useSecondUART = 1;

	printf_P(PSTR("SecondUART %lu 8-N-1\n"), baudrate);
}

uint8_t UseSecondUART(void)
{
	return useSecondUART;
}

void UartSecondaryRxEnable(void) //подключаем прием байт по USART0
{
	if (useSecondUART) //если UART инициализирован, тогда подключаем прием байт
		UCSR0B |= (1<<RXCIE0) | (1<<RXEN0); //разрешаем прерывание на прием и разрешаем прием

	printf_P(PSTR("SecondUART Rx enable\n"));
}

ISR (USART0_RX_vect) //обработчик прерывания по приему байта USART0
{
	Usart0RxHandler(UDR0); //передаем принятый байт на обработку
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
