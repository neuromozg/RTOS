/*
 * processing_can.c
 *
 *  Created on: 14.04.2016
 *      Author: max
 */
#include <string.h>
#include "include/processing_can.h"
#include "include/cmd_list.h"
#include "include/param_list.h"

//отработка принятого CAN пакета
void ProcessingCANmsg(const uint32_t msgID, const uint8_t msg[], const uint8_t msgLength)
{
	for (uint8_t i = 0; i < msgAmount; i++) //перебор всего массива msgHanlerList
	{
		if ((msgHanlerList[i].canID == msgID) && (msgHanlerList[i].handler != NULL)) //если совпал canID и есть обработчик
		{
			msgHanlerList[i].handler(msg, msgLength); //вызываем обработчик
			break;
		}
	}
}

//стандартный обработчик CAN пакетов, относящихся к данному устройству в рамках собственного протокола
void DeviceCanIdHandler(const uint8_t msgData[], const uint8_t lenght)
{
	//проверка на принадлежность байта msg[0] к параметрам системы
	if (msgData[0] < CmdStartNumber)
	{
		//изменяем значение параметра системы
		UpdateParam(msgData[0], &msgData[2], msgData[1]); //номер параметра, указатель на данные параметра, длина данных
	}
	else
	{
		//если msg[0] не относится к параметрам, значит считаем, что это команда
		RunCommand(msgData[0], &msgData[1], lenght-1); //номер команды, указатель на данные команды, количество параметров команды
	} //else
}
