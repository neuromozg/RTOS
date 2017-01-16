/*
 * 32m1_uart.h
 *
 *  Created on: 20 февр. 2016 г.
 *      Author: Алексеев
 */

#ifndef SRC_INCLUDE_32M1_UART_H_
#define SRC_INCLUDE_32M1_UART_H_

#define SAMPLES_PER_BIT 8

#include <avr/io.h>
#include <stdio.h>


int UartPutChar(char c, FILE *stream);

#if defined (__AVR_AT90CAN128__)
int SecondUartPutChar(char c);
#endif

static FILE mystdout = FDEV_SETUP_STREAM(UartPutChar, NULL, _FDEV_SETUP_WRITE);
//FILE mystdout = FDEV_SETUP_STREAM(UartPutChar, NULL, _FDEV_SETUP_WRITE);

void UartInit(uint32_t baudrate);
uint8_t UseUART(void);

#if defined (__AVR_AT90CAN128__)
void UartSecondaryInit(uint32_t baudrate);
uint8_t UseSecondUART(void);
void UartSecondaryRxEnable(void);
extern void Usart0RxHandler(uint8_t data);
#endif

void UartWriteByte(uint8_t data);
void UartWriteData(uint8_t data[], uint8_t lenght);

#endif /* SRC_INCLUDE_32M1_UART_H_ */
