/**
  ******************************************************************************
  * @file    task_test.c
  * @author  AresZzz
  * @version V1.0
  * @date    2018/08/6
  * @brief   一些测试工作
  * 
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "task_init.h"
/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/

_ICDATA MyIC;

osThreadId testTaskHandle;
osThreadId readTaskHandle;


uint8_t ucStatusReturn; 
uint8_t ucArray_ID[4];
uint8_t UID[4]={0};
uint8_t ReadComplete=0;

uint8_t Read_Data[16]={0};
uint8_t Write_Data[16]={0x00,0x01,0x02,0x03,
												0x04,0x05,0x06,0x07,
												0x08,0x09,0x0A,0x0B,
												0x0C,0x0D,0x0E,0x0F};
uint8_t flag=0;
/* Function  -----------------------------------------------------------------*/

												
uint8_t ReadICData(void)	
{
	uint8_t Status_Select;
	uint8_t Status_Anticoll;
	uint8_t Status_AuthState;
	uint8_t Status_Read;
	uint8_t SectionNum,BlockID,BlockNum=0,i;
	
	Status_Anticoll=RC522_Anticoll(MyIC.CardUid);
	if(Status_Anticoll!=MI_OK)
	{
		UART_Printf(&huart1,"Anticoll Fail！\n");
		return MI_ERR;
	}
	else
		UART_Printf(&huart1,"Anticoll Success！\n");
	Status_Select=RC522_Select(MyIC.CardUid);
	if(Status_Select!=MI_OK)
	{
		UART_Printf(&huart1,"Select Card Fail！\n");
		return MI_ERR;
	}
	else
		UART_Printf(&huart1,"Select Card Success！\n");
	for(SectionNum=0;SectionNum<16;SectionNum++)
	{
		BlockNum=SectionNum*4;
		Status_AuthState=RC522_AuthState(PICC_AUTHENT1A,BlockNum,MyIC.DafultKey,MyIC.CardUid);
		if(Status_AuthState!=MI_OK)
		{
			UART_Printf(&huart1,"AuthState Fail！\n");
			return MI_ERR;
		}
		else
			UART_Printf(&huart1,"AuthState Success！\n");
		for(BlockID=0;BlockID<4;BlockID++)
		{
			BlockNum=SectionNum*4+BlockID;
				//RC522_AuthState(PICC_AUTHENT1A,BlockNum,MyIC.DafultKey,MyIC.CardUid);
			if(BlockID<3)
			{
				Status_Read=RC522_ReadBlock(BlockNum,MyIC.Section[SectionNum].DataBlock[BlockID].Data);
				if(Status_Read==MI_OK)
					UART_Printf(&huart1,"Read Block %2d Success！\n",BlockNum);
				else
					UART_Printf(&huart1,"Read Block %2d Fail！\n",BlockNum);
			}
			else
			{
				Status_Read=RC522_ReadBlock(BlockNum,MyIC.Section[SectionNum].ControlBlock.Data);
				if(Status_Read==MI_OK)
					UART_Printf(&huart1,"Read Block %2d Success！\n",BlockNum);
				else
					UART_Printf(&huart1,"Read Block %2d Fail！\n",BlockNum);
				for(i=0;i<4;i++)
				{
					MyIC.Section[SectionNum].ControlBlock.ControlBit[i]=MyIC.Section[SectionNum].ControlBlock.Data[i+6];
				}
				
				for(i=0;i<6;i++)
				{
					MyIC.Section[SectionNum].ControlBlock.KeyB[i]=MyIC.Section[SectionNum].ControlBlock.Data[i+10];
				}
			}
			
		}
	}
	return MI_OK;
	//RC522_Halt();
}	


uint8_t WriteICData(uint8_t SectionNum,uint8_t BlockNum,uint8_t *data)	
{
	uint8_t Status_Select;
	uint8_t Status_Anticoll;
	uint8_t Status_AuthState;
	uint8_t Status_Write;
	
	uint8_t BlockID;
	
	BlockID=SectionNum*4+BlockNum;
	
	Status_Anticoll=RC522_Anticoll(MyIC.CardUid);
	if(Status_Anticoll!=MI_OK)
		return MI_ERR;
	Status_Select=RC522_Select(MyIC.CardUid);
	if(Status_Select!=MI_OK)
		return MI_ERR;
	Status_AuthState=RC522_AuthState(PICC_AUTHENT1A,BlockID,MyIC.DafultKey,MyIC.CardUid);
	if(Status_AuthState!=MI_OK)
		return MI_ERR;
	
	Status_Write=RC522_WriteBlock(BlockID,data);
	
	
	return MI_OK;
}
void TestProg(void)
{	
	
	if(PC_Buf[0]==0x55)
	{
		if((ucStatusReturn=RC522_Request(PICC_REQALL, MyIC.CardType))!=MI_OK)
			ucStatusReturn = RC522_Request(PICC_REQALL, MyIC.CardType);	
		if(ucStatusReturn==MI_OK)
		{
		//	if(WriteICData(1,1,Write_Data)==MI_OK);
			//{
				if(ReadICData()==MI_OK);
				{
					PrintICData(MyIC);
					PC_Buf[0]=0;
				}
			//}
		}
		
	}
	
	
		
	
}

void TestTask(void const * argument)
{
	 osDelay(100);

  for(;;)
  {
    TestProg();
		osDelay(100);
  }
}
void testTaskThreadCreate(osPriority taskPriority)
{
	osThreadDef(initTask, TestTask, taskPriority, 0, 2000);
  testTaskHandle = osThreadCreate(osThread(initTask), NULL);
}


