#ifndef __TASK_INIT_H
#define __TASK_INIT_H

#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "spi.h"
#include "gpio.h"
#include "usart.h"

#include "FreeRTOS.h"


#include "task_led.h"
#include "task_test.h"

#include "Bsp_RC522.h"
#include "Bsp_usart.h"

void Sys_Init(void);
void initTaskThreadCreate(osPriority taskPriority);

#endif
