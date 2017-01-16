/*
 * eertos.c
 *
 *  Created on: 29 июня 2015 г.
 *      Author: воробьев
 */
#include <string.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "include/32m1_rtos.h"
#include "include/32m1_uart.h"


//static uint64_t tickCount = 0;
//очереди задач, таймеров
typedef struct {
	TPTR gotoTask; //указатель перехода на задачу
	void *data;  //указатель параметр задачи
} TaskData;

//очередь задач
static TaskData taskQueue[TaskQueueSize+1]; //очередь задач

typedef struct {
	TPTR gotoTask; //указатель перехода на задачу
	uint16_t time; //выдержка в мс
	void *data;  //указатель параметр задачи
} TimerData;

//таймеры RTOS
static TimerData mainTimer[MainTimerQueueSize];

#if defined (__AVR_ATmega32M1__)
ISR(TIMER0_COMPA_vect) //обработчик прерывания таймера RTOS //служба таймеров ядра
{
	TimerService();
	//tickCount++;
}
#endif

#if defined (__AVR_AT90CAN128__)
ISR(TIMER0_COMP_vect) //обработчик прерывания таймера RTOS
{
	TimerService();
	//tickCount++;
}
#endif

//rtos запуск системного таймера
inline void RunRTOS(uint16_t freq)
{
	uint16_t TimerDivider = F_CPU/Prescaler/freq;

#if defined (__AVR_ATmega32M1__)
	TCCR0A = 1 << WGM01; 			//режим ctc, регистр сбрасывается при достижении регистра сравнения
	TCCR0B = 1 << CS00 | 1 << CS01;	//предделитель 64
#endif
#if defined (__AVR_AT90CAN128__)
	TCCR0A = 1 << WGM01 | 1 << CS00 | 1 << CS01;	//предделитель 64
#endif


	TCNT0 = 0; //начальное значение счетчика
	OCR0A = TimerDivider; //устанавливаем значение в регистр сравнения
	TIMSK0 = 1 << OCIE0A; //разрешаем прерывания по совпадению

	//sei(); запуск RTOS
	if (UseUART())
		printf_P(PSTR("RTOS %uHz\n"), freq);
}

//RTOS подготовка. Очистка очередей
inline void InitRTOS(void)
{
	for(uint8_t i = 0; i<(TaskQueueSize+1); i++)//во все позиции записываем Idle
	{
		taskQueue[i].gotoTask = Idle;
		taskQueue[i].data = NULL;
	}

	for(uint8_t i = 0; i < MainTimerQueueSize; i++)//обнуляем все таймеры
	{
		mainTimer[i].gotoTask = Idle;
		mainTimer[i].time = 0;
		mainTimer[i].data = NULL;
	}
	if (UseUART())
		printf_P(PSTR("RTOS init.\n"));
}

//пустая процедура - простой ядра
inline void Idle(void *data)
{

}

//функция установки задачи в очередь. Передаваемый параметр - указатель на функцию
//для совметимости со старыми версиями RTOS, где не было передачи параметра
uint8_t SetTask(TPTR TS){
	return SetTaskParam(TS, NULL);
}

//функция установки задачи в очередь. Передаваемый параметр - указатель на функцию и двухбайтовый параметр функции
uint8_t SetTaskParam(TPTR TS, void *data)
{
	uint8_t index = 0;
#ifndef TIMERSERVICE_DISABLE_GLOBAL_INTERRUPT
	uint8_t nointerrupted = 0;
#endif
	uint8_t res = 1;

#ifndef TIMERSERVICE_DISABLE_GLOBAL_INTERRUPT
	if(SREG & (1<<SREG_I)) //если прерывания разрешены, запрещаем
	{
		cli();
		nointerrupted = 1; //ставим флаг, что мы не в прерывании
	}
#endif

	while(taskQueue[index].gotoTask != Idle) //просматриваем очередь задач на предмет свободной ячейки
	{
		index++;
		if(index == TaskQueueSize) //если очередь переполнена, выходим
		{
			//if(nointerrupted) sei(); //если мы не в прерывании, разрешаем прерывания
			res = 0;
			break; //выходим
		}
	}

	if (res)
	{
		taskQueue[index].gotoTask = TS; //записываем задачу в очередь
		taskQueue[index].data = data;

		taskQueue[index+1].gotoTask = Idle; //записываем следующей задачей заглушку
		taskQueue[index+1].data = NULL;
	}

#ifndef TIMERSERVICE_DISABLE_GLOBAL_INTERRUPT
	if(nointerrupted) sei(); //включаем прерывания, если не в обработчике прерываний
#endif

	return res;
}

uint8_t SetTimerTask(TPTR TS, const uint16_t time)
{
	return SetTimerTaskParam(TS, time, NULL);
}

//функция установки задачи по таймеру.передаваемые параметры - указатель на функцию,
//время выдержки в тиках системного таймера, двухбайтовый параметр
uint8_t SetTimerTaskParam(TPTR TS, const uint16_t time, void *data)
{
#ifndef TIMERSERVICE_DISABLE_GLOBAL_INTERRUPT
	uint8_t nointerrupted = 0;
#endif
	uint8_t tmpTimerIndex = UINT8_MAX;
	uint8_t res = 1;

#ifndef TIMERSERVICE_DISABLE_GLOBAL_INTERRUPT
	if(SREG & (1<<SREG_I)) //проверка запрета прерываний, см функцию выше
	{
		cli();
		nointerrupted = 1;
	}
#endif

	if(time == 0) //если время 0, тогда сразу ставим задачу в очередь
	{
		res = SetTaskParam(TS, data);
		//if(nointerrupted) sei(); //разрешаем прерывания
	}
	else
	{
		for(uint8_t i = 0; i < MainTimerQueueSize; i++) //прочесываем очередь таймеров
		{
			if ((mainTimer[i].gotoTask == TS) && (mainTimer[i].data == data)) //если уже есть запись с таким адресом и параметром
			{
				tmpTimerIndex = i;
				break; //выходим
			}
			else
			{
				//на случай, если в списке таймеров нет задачи TS, определяем индекс первого не занятого таймера
				if((mainTimer[i].gotoTask == Idle) && (tmpTimerIndex == UINT8_MAX))
					tmpTimerIndex = i;
			}
		}

		if (tmpTimerIndex != UINT8_MAX) //если в системе есть не работающий таймер
		{
			//задаем новый таймер
			mainTimer[tmpTimerIndex].gotoTask = TS; //заполяем поле перехода задачи
			mainTimer[tmpTimerIndex].time = (time - 1); //поле выдержки времени
			mainTimer[tmpTimerIndex].data = data;
			//if(nointerrupted) sei(); //если мы не в прерывании, разрешаем прерывания
		}
		else
			res = 0; //все таймеры задействованы, установить таймер не удалось
	}

#ifndef TIMERSERVICE_DISABLE_GLOBAL_INTERRUPT
	if(nointerrupted) sei(); //разрешаем прерывания
#endif

	return res;
}

// Диспетчер задач ОС. Выбирает из очереди задачи и отправляет на выполнение.
inline void TaskManager(void)
{
	uint8_t index = 0;

	cli(); //запрещаем прерывания
	TPTR gotoTask = taskQueue[0].gotoTask; //берем первое значение из очереди
	void *data = taskQueue[0].data; //указатель на параметры
	sei(); //разрешаем прерывания


	if (gotoTask == Idle) //если там ничего нет
	{
		(Idle)(NULL); //переходим на обработку пустого цикла
	}
	else
	{
		cli();
		while(taskQueue[index].gotoTask != Idle) //если что то есть, двигаем всю очередь
		{
			//taskQueue[index] = taskQueue[index+1];
			memcpy(&taskQueue[index], &taskQueue[index+1], sizeof(TaskData)); //перемещаем значение соседней ячейки
			index++;
		}
		sei();
//		for(uint8_t i = 0; i < TaskQueueSize; i++) //если что то есть, двигаем всю очередь
//		{
//			//cli(); //запрещаем прерывания
//			//taskQueue[i] = taskQueue[i+1];
//			memcpy(&taskQueue[i], &taskQueue[i+1], sizeof(TaskData)); //перемещаем значение соседней ячейки
//			//sei();
//
//			if (taskQueue[i+1].gotoTask == Idle)
//				break;
//		}
		//taskQueue[TaskQueueSize].gotoTask = Idle; //в последнюю запись пишем простой ядра
		//taskQueue[TaskQueueSize].prm = 0;

		//sei(); //разрешаем прерывания

		(gotoTask)(data); //переходим к задаче и передаем ей параметр
	}
}

//служба таймеров ядра. вызывается из прерывания раз в 1 мс.
inline void TimerService(void)
{
	for(uint8_t i = 0; i < MainTimerQueueSize; i++) //проходим все таймеры
	{
		if(mainTimer[i].gotoTask == Idle) continue; //если нашли пустой - пропускаем

		if(mainTimer[i].time > 0) //если таймер не закончил считать, делаем еще итерацию
		{
			mainTimer[i].time--; //уменьшаем число в ячейке, если еще не конец
		}
		else
		{
			SetTaskParam(mainTimer[i].gotoTask, mainTimer[i].data); //если таймер дошел до нуля, отправляем в очередь задач
			mainTimer[i].gotoTask = Idle; //в освободившуюся ячейку пишем простой ядра
		}
	}
}

/*
uint64_t GetTickCount(void)
{
	return tickCount;
}
*/
