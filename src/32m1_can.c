/*
 * 32m1_can.c

 *
 *  Created on: 8 ���. 2015 �.
 *      Author: ��������
 */
#include <stdio.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <string.h>
#include "include/32m1_rtos.h"
#include "include/32m1_can.h"
#include "include/32m1_uart.h"

//������������� ���������� CAN � �������
uint8_t useCAN = 0;

uint8_t queueEndIndex = 0; //����� ������� ���������
uint8_t queueStartIndex = 0; //������ ������� ���������
uint8_t tagQueueOverlap = 0; //������� �������� ������� ������� ����� 255
uint8_t tagCANSending = 0; //������� �������� ������ �� �������

//������� ��� ������������ CAN ���������
struct {
	uint32_t msgID; //�������������
	uint8_t msg[8]; //������ CAN ������
	uint8_t msgLength; //����� ������
} canMessageQueue[CAN_MsgQueueLenght];

uint8_t useStdB; //����� ���������� CAN ��������� 0-�������� A 11bit, 1-�������� B 29bit

void PushCANData(void *data); //��������� �������� ��������� �� �������

//���������� CAN Transfer Complete or Error
#if defined (__AVR_ATmega32M1__)
ISR(CAN_INT_vect)
#endif
#if defined (__AVR_AT90CAN128__)
ISR(CANIT_vect)
#endif
{  				// use interrupts
  	uint8_t dataLength;
  	uint32_t rxCanID;
  	uint8_t msg[8]; //������ ������ ��������� ���������

  	uint8_t canPageBackup = CANPAGE; // Save current MOB

	CANPAGE = CANHPMOB & 0xF0;		// Selects MOB with highest priority interrupt

	if (CANSTMOB & (1 << RXOK)) //������ �����
	{  	// Interrupt caused by receive finished

		CANSTMOB &= ~(1 << RXOK); // ����� ����� ����������

		//������� ������������� CAN ������
		if (useStdB)
			//�������� ID 29bit
			rxCanID = (((uint32_t)CANIDT1 << 21) | ((uint32_t)CANIDT2 << 13) | ((uint32_t)CANIDT3 << 5) | (CANIDT4 >> 3));
		else
			//�������� ID 11bit
			rxCanID = ((uint32_t) CANIDT1 << 3) | ((uint32_t) CANIDT2 >> 5);

		dataLength = ( CANCDMOB & 0x0F );	// ������� ����� ������

		for(uint8_t i=0; i<dataLength; i++) //������ �������� ������ � ������
			msg[i] = CANMSG;

		//��������� ��������� CAN ������
		ProcessingCANmsg(rxCanID, msg, dataLength);

		CANCDMOB |= ( 1 << CONMOB1 ); // ������� �����
	}


	if (CANSTMOB & (1 << TXOK)) //�������� ������ ���������
	{
		CANSTMOB &= ~(1 << TXOK); 	// ����� ����� ����������

		//�������� �������� ������ ��������� ����������� �� ��������
		if ((queueStartIndex == queueEndIndex) && (tagQueueOverlap == 0)) //���� ��������� ������ ������ ��������� � ��� ���� ��� ���������� �������
		{
			//�������� ��������
			tagCANSending = 0;
		}
		else
		{
			SetTask(PushCANData); //���������� ��������� CAN ����� �� �������
		}
	}

	// �������� �� ������ (general errors) ������ CAN
	if (CANGIT & (1 << BOFFIT))
	{
		// Oh dear, we are in bus off mode
		// Clear flag
		CANGIT |= (1 << BOFFIT);
	}

	// Clear error flags
	if (CANGIT & (_BV(SERG)|_BV(CERG)|_BV(FERG)|_BV(AERG)))
	{
		CANGIT |= (_BV(SERG)|_BV(CERG)|_BV(FERG)|_BV(AERG));
		receiveErrorCount++; //����������� ������� ������
	}

	CANPAGE = canPageBackup;		// Restore original MOB
}

//***** CAN ialization *****************************************************
uint8_t CanInit(uint16_t bitrate, uint8_t standart)  //��������� ������������� CAN ������� � �������� CAN ����������: 0 �������� � 11bit, 1 �������� B 29bit
{
	useStdB = standart;

	CANGCON =  1 << SWRES;   // Software reset
	while (CANGCON & (1 << SWRES));

	CANTCON = 0x00;         // CAN timing prescaler set to 0;

	//while( CANGSTA & ( 1<<ENFG )); /* AUF DISABLE WARTEN */

	if (bitrate <= 1000)
	{
		// CANopen 10..1000 kbit with 16 tq, sample point is at 14 tq
	    // all values are added to 1 by hardware
	    // Resynchronisation jump width (SJW)   = 1 tq
	    // Propagation Time Segment (PRS)       = 5 tq
	    // Phase Segment 1 (PHS1)           = 8 tq
	    // Phase Segment 2 (PHS2)           = 2 tq
	    // Total                    = 16 tq
	    CANBT1 = ((F_CPU/16/1000/bitrate-1) << BRP0);  // set bitrate
	    CANBT2 = ((1-1) << SJW0) |((5-1) << PRS0);    // set SJW, PRS
	    CANBT3 = (((2-1) << PHS20) | ((8-1) << PHS10) | (1<<SMP)); // set PHS1, PHS2, 3 sample points
	}
	else
	{
		if (UseUART())
			printf_P(PSTR("ERR: Wrong CAN bitrate %u\n"), bitrate);
		return 0;
	}
	//����� ���� MailBox
	for (uint8_t mob=0; mob<6; mob++ )
	{

		CANPAGE = ( mob << 4 );     	// Selects Message Object 0-5

		CANCDMOB = 0x00;       		// Disable mob

		CANSTMOB = 0x00;     		// Clear mob status register;
	}

	CANGIE = (1 << ENIT) | (1 << ENRX) | (1 << ENTX) | (1 << ENERG) | (1<<ENBOFF);   // ������� CAN ���������� �� ����� � �������� + ���������� �� ������
	CANEN1 = 0;
    CANEN2 = ( 1 << ENMOB1 ) | ( 1 << ENMOB0 ); // ������� MOB0, MOB1
    CANIE1 = 0;
	CANIE2 = ( 1 << IEMOB1 ) | ( 1 << IEMOB0 ); // Enable interrupts on mob1 for reception and transmission

	//��������� MOB0 �� ����� �������� �������
	CANPAGE = 0;		// Selecto MOB0

	CANIDM1 = 0;   	// Clear Mask, let all IDs pass
	CANIDM2 = 0; 	// ""
	CANIDM3 = 0; 	// ""
	CANIDM4 = 0; 	// ""

	if (useStdB)
		CANCDMOB = (1 << IDE); //CAN ��������� 29bit, �������� B
	CANCDMOB |= (1 << CONMOB1) ;  // ������� �����

	//��������� MOB1 �� �������� ������� � ID Response_CAN_ID
	CANPAGE = (1 << MOBNB0);		// Selecto MOB1

	// set compatibility registers, RTR bit, and reserved bit to 0
	CANIDT4 = 0;
	CANIDT3 = 0;
	CANIDT2 = 0;
	CANIDT1 = 0;
	// set ID tag registers
	//CANIDT2 = (((Response_CAN_ID) & 0x07) << 5); // bits 0-2 of idtag (0b0111)
	//CANIDT1 = (((Response_CAN_ID) & 0x7F8) >> 3); // bits 3-10 of idtag (0b11111111000)

	CANCDMOB = 0;
	if (useStdB)
		CANCDMOB = (1 << IDE); //��������� ��������� ���������� 29bit

	CANGCON =  (1 << ENASTB);			// ������� CAN
	while (!(CANGSTA & (1<<ENFG))); //�������� ����� ��������� CAN

	if (UseUART())
	{
		printf_P(PSTR("CAN %ubit/s"), bitrate);
		if (useStdB)
			printf_P(PSTR(" 29bit\n"));
		else
			printf_P(PSTR(" 11bit\n"));
	}

	useCAN = 1;

	return 1;
}

uint8_t UseCAN(void)
{
	return useCAN;
}
/*
// Sample call: sendCANmsg(NODE_watchdog,MSG_critical,data,dataLen);
//uint8_t sendCANmsg(uint16_t msgID, uint8_t msg[], uint8_t msgLength) {
void SendCANmsg(uint16_t msgID, const uint8_t msg[], const uint8_t msgLength) {
		// use MOb 0 for sending and auto-increment bits in CAN page MOb register
		CANPAGE = 0;

		//Wait for MOb1 to be free
		// TODO: This is not good practice; take another look later
		while(CANEN2 & (1 << ENMOB0)); // Stuck in infinite loop?
		CANEN2 |= (1 << ENMOB0); //Claim MOb1

		//Clear MOb status register
		CANSTMOB = 0x00;

		for (uint8_t i = 0; i < msgLength; ++i) {
			// while data remains, write it into the data page register
			CANMSG = msg[i];
		}

		// set compatibility registers, RTR bit, and reserved bit to 0
		CANIDT4 = 0;
		CANIDT3 = 0;

		// set ID tag registers
		CANIDT2 = ((msgID & 0x07) << 5); // bits 0-2 of idtag (0b0111)
		CANIDT1 = ((msgID & 0x7F8) >> 3); // bits 3-10 of idtag (0b11111111000)

		CANCDMOB = (_BV(CONMOB0) | msgLength); // set transmit bit and data length bits of MOb control register

		//TODO: use interrupts for this instead of while loop
		//wait for TXOK
		while((CANSTMOB & (1 << TXOK)) != (1 << TXOK));// & timeout--);

		//Disable Transmission
		CANCDMOB = 0x00;

		//Clear TXOK flag (and all others)
		CANSTMOB = 0x00;
}
*/

void PushCANData(void *data)
{
	cli(); //��������� ����������

    const uint32_t msgID = canMessageQueue[queueStartIndex].msgID;
    const uint8_t msgLength = (canMessageQueue[queueStartIndex].msgLength & 0b00001111);

	// �������� �� MOB1
	CANPAGE = ( 1 << MOBNB0 );

	CANCDMOB &= ~((1 << CONMOB0) | (1 << CONMOB1)); //��������� ����� ��������

	//Clear MOb status register
    //CANSTMOB = 0x00;

    //�������� ������ � ������� ������
    for (uint8_t i = 0; i < msgLength; i++)
    {
        // while data remains, write it into the data page register
        CANMSG = canMessageQueue[queueStartIndex].msg[i];
    }

    //��������� ������������� ������
    if (useStdB)
    {
    	//������������� 29bit
    	CANIDT4 = (msgID & 0xFF) << 3;
    	CANIDT3 = (msgID >> 5) & 0xFF;
    	CANIDT2 = (msgID >> 13) & 0xFF;
    	CANIDT1 = (msgID >> 21) & 0xFF;
    }
    else
    {
    	//������������� 11bit
		// set compatibility registers, RTR bit, and reserved bit to 0
		CANIDT4 = 0;
		CANIDT3 = 0;

		// set ID tag registers
		CANIDT2 = ((msgID & 0x07) << 5); // bits 0-2 of idtag (0b0111)
		CANIDT1 = ((msgID & 0x7F8) >> 3); // bits 3-10 of idtag (0b11111111000)
    }

    CANCDMOB |= ((1<<CONMOB0) | msgLength); // �������� �������� ������, ������ msgLength


    queueStartIndex ++; //����������� ������ �������
	if ((queueStartIndex == CAN_MsgQueueLenght) && tagQueueOverlap) //���� ��������� ��������� ������� � ���������� ������� �������� �����
	{
		tagQueueOverlap = 0; //����� �������� ��������
		queueStartIndex = 0;
	}

	sei(); //��������� ����������
}


//��������� ��������� � ������� ��������
uint8_t SendCANmsg(const uint32_t msgID, const uint8_t msg[], const uint8_t msgLength)
{
	//�������� �� ������������ �������
	if ((queueEndIndex == queueStartIndex) && tagQueueOverlap)
	{
		//������� �����������, �������� ����� ����������
		if (UseUART() && (debugLevel > 0))
			printf_P(PSTR("ERR: CAN queue full\n"));
		return 1;
	}
	else
	{
		if (msgLength < 9) //�������� ������������ ����� ������
		{
			//���������� ������ � �������
			canMessageQueue[queueEndIndex].msgID = msgID;
			canMessageQueue[queueEndIndex].msgLength = msgLength;
			memcpy(canMessageQueue[queueEndIndex].msg, msg, msgLength); // �������� ������ CAN ������
			//for (uint8_t i = 0; i < msgLength; i++)
				//canMessageQueue[queueEndIndex].msg[i] = msg[i];

			queueEndIndex++;

			if (queueEndIndex == CAN_MsgQueueLenght)
			{
				tagQueueOverlap = 1; //������� ������� ����� ������� ����� CAN_MsgQueueLenght
				queueEndIndex = 0;
			}


			if (tagCANSending == 0) //���� � ������ ������ �������� CAN ������� �� �������, ����� ������������� ��������� �����
			{
				tagCANSending = 1; //������� ��������
				SetTask(PushCANData); //���������� CAN �����
			}

			//if (UseUART() && (debugLevel > 0))
				//printf_P(PSTR("Add CAN queue ID:0x%X [%u]\n"), msgID, msgLength);

			return 0;
		}
		else
		{
			if (UseUART() && (debugLevel > 0))
				printf_P(PSTR("ERR: CAN:0x%X wrong lenght\n"), msgID);
			return 1;
		}
	}
}
