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

//��������� ��������� CAN ������
void ProcessingCANmsg(const uint32_t msgID, const uint8_t msg[], const uint8_t msgLength)
{
	for (uint8_t i = 0; i < msgAmount; i++) //������� ����� ������� msgHanlerList
	{
		if ((msgHanlerList[i].canID == msgID) && (msgHanlerList[i].handler != NULL)) //���� ������ canID � ���� ����������
		{
			msgHanlerList[i].handler(msg, msgLength); //�������� ����������
			break;
		}
	}
}

//����������� ���������� CAN �������, ����������� � ������� ���������� � ������ ������������ ���������
void DeviceCanIdHandler(const uint8_t msgData[], const uint8_t lenght)
{
	//�������� �� �������������� ����� msg[0] � ���������� �������
	if (msgData[0] < CmdStartNumber)
	{
		//�������� �������� ��������� �������
		UpdateParam(msgData[0], &msgData[2], msgData[1]); //����� ���������, ��������� �� ������ ���������, ����� ������
	}
	else
	{
		//���� msg[0] �� ��������� � ����������, ������ �������, ��� ��� �������
		RunCommand(msgData[0], &msgData[1], lenght-1); //����� �������, ��������� �� ������ �������, ���������� ���������� �������
	} //else
}
