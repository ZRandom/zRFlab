/**
  ******************************************************************************
  * @file    task_init.c
  * @author  AresZzz“∆÷≤
  * @version V1.0
  * @date    2018/04/17
  * @brief   
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
osThreadId initTaskHandle;
extern _ICDATA MyIC;
/* Function  -----------------------------------------------------------------*/
void Sys_Init(void)
{
	int i;
	
	zRC522_CS_Disable();
	RC522_Init();
	RC522_A_SET();
	for(i=0;i<6;i++)
	{
		MyIC.DafultKey[i]=0xFF;
	}
	
}

void InitTask(void const * argument)
{

  osDelay(100);
	Sys_Init();
	Led_Task_Init();
  testTaskThreadCreate(osPriorityNormal);
	
  osDelay(100);
  for(;;)
  {
    vTaskDelete(initTaskHandle);
  }
}


/**
  * @brief  Create the UnderpanTask threads
  * @param  None
  * @retval None
  */
void initTaskThreadCreate(osPriority taskPriority)
{
	osThreadDef(initTask, InitTask, taskPriority, 0, 64);
  initTaskHandle = osThreadCreate(osThread(initTask), NULL);
}

