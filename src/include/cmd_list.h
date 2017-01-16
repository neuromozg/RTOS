/*
 * cmd_list.h
 *
 *  Created on: 25 февр. 2016 г.
 *      Author: Алексеев
 */

#ifndef SRC_INCLUDE_CMD_LIST_H_
#define SRC_INCLUDE_CMD_LIST_H_

#define MaxCmdNameLen 20 //максимальная длина имени команды
#define CmdStartNumber 200 //номер с которого начинаются номера команд

#include <avr/io.h>
#include <avr/pgmspace.h>

typedef void (*pHandler_t)(const uint8_t cmdData[]); //прототип функции обработчика команды

//массив комманд
typedef struct {
	pHandler_t cmdHandler; // обработчик команды
	PGM_P cmdName; // имя команды
	uint8_t amountPrm; //количество байт параметров
} cmdListItem;

extern uint8_t debugLevel; //уровень выдачи отладочной информации
extern cmdListItem cmdList[];
extern const uint8_t cmdAmount;
//extern const uint16_t responseCanID;

void ClearCmdList();
uint8_t AddCmdList (const uint8_t nCmd, pHandler_t cmdHandler, const uint8_t prmAmount, PGM_P cmdName);
uint8_t RunCommand(const uint8_t nCmd, const uint8_t cmdPrm[], const uint8_t prmAmount);

//функции вызываемые из RTOS
void PrintExecCmd(void *data);

#endif /* SRC_INCLUDE_CMD_LIST_H_ */
