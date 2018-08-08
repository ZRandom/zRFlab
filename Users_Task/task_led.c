/**
  ******************************************************************************
  * @file    task_led.c
  * @author  AresZzz ÒÆÖ²
  * @version V1.0
  * @date    2018/04/17
  * @brief   ledÁÁÃð
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
osThreadId ledRedTaskHandle;
osThreadId ledBlueTaskHandle;
/* Function  -----------------------------------------------------------------*/


void ERROR_LED(uint8_t led)
{
	int i=0;
	if(led>0)
	{
		for(i=0;i<led;i++)
		{
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
			osDelay(150);
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
			osDelay(150);
		}
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
		osDelay(1000);
	}
	else 
 {
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
    osDelay(500);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
    osDelay(500);   
 }
}


void BlinkLedBlue(void const *argument)
{
	for(;;){
        ERROR_LED(1); 
    }    
}

void Led_Task_Init(void)
{
	osThreadDef(ledBlueTask, BlinkLedBlue, osPriorityLow, 0, 56);
  ledBlueTaskHandle = osThreadCreate(osThread(ledBlueTask), NULL);
	//osThreadDef(ledRedTask, BlinkLedRed, osPriorityLow, 0, 56);
  //ledRedTaskHandle = osThreadCreate(osThread(ledRedTask), NULL);
}

