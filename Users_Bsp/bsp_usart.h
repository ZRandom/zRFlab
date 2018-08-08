#ifndef _BSP_USART_
#define _BSP_USART_

#include <stm32f4xx.h>
#include "task_init.h"
#include "usart.h"

#define PC_HUART      huart1
#define PC_BUF_LEN    50

#define HEAD_LEN    4
#define ON  1
#define OFF 0

#define PTZ_HUART      huart1

extern uint8_t PC_Buf[PC_BUF_LEN];



HAL_StatusTypeDef Bsp_UART_Receive_DMA(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef Bsp_UART_Receive_IT(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);
void Bsp_UsartIdleHanlder(UART_HandleTypeDef *huart,uint16_t Size);
void ISO14443AAppendCRCA(void* Buffer, uint16_t ByteCount);
uint8_t ISO14443ACheckCRCA(void* Buffer, uint16_t ByteCount);
uint8_t ISO14443ACheckLen(uint8_t* Buffer);
void UART_Printf(UART_HandleTypeDef *huart, const char* fmt, ...);
void Send_Check(int *s); 
void UART_IdleRxCallback(UART_HandleTypeDef *huart);

#endif
