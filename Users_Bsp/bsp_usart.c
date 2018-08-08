/**
  ******************************************************************************
  * @file    task_frictiongear.c
  * @author  AresZzz移植
  * @version V1.0
  * @date    2018/04/18
  * @brief   串口定义
  * 
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "task_init.h"
#include "stdarg.h"
#include "string.h"
/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
uint8_t PC_Buf[PC_BUF_LEN];

/* Function  -----------------------------------------------------------------*/

/**
  * @brief  为串口开启没有中断的DMA传输，为了减少中断次数为其他中断空出资源。
  *         代替HAL库的函数(此处在main函数中调用)
  * @param  hdma: 指向DMA_HandleTypeDef结构体的指针，这个结构体包含了DMA流的配置信息.  
  * @retval HAL status
  */
HAL_StatusTypeDef Bsp_UART_Receive_DMA(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
{
  uint32_t *tmp;
  uint32_t tmp1 = 0;
  
  tmp1 = huart->gState;
  if((tmp1 == HAL_UART_STATE_READY) || (tmp1 == HAL_UART_STATE_BUSY_TX))
  {
    if((pData == NULL ) || (Size == 0)) 
    {
      return HAL_ERROR;
    }
    
    /* Process Locked */
    __HAL_LOCK(huart);
    
    huart->pRxBuffPtr = pData;
    huart->RxXferSize = Size;
    
    huart->ErrorCode = HAL_UART_ERROR_NONE;
    /* Check if a transmit process is ongoing or not */
    if(huart->gState == HAL_UART_STATE_BUSY_TX)
    {
      huart->gState = HAL_UART_STATE_BUSY_TX_RX;
    }
    else
    {
      huart->gState = HAL_UART_STATE_BUSY_RX;
    }
    
    /* Enable the DMA Stream */
    tmp = (uint32_t*)&pData;
    HAL_DMA_Start(huart->hdmarx, (uint32_t)&huart->Instance->DR, *(uint32_t*)tmp, Size);
    
    /* Enable the DMA transfer for the receiver request by setting the DMAR bit 
    in the UART CR3 register */
    huart->Instance->CR3 |= USART_CR3_DMAR;
    
    /* Process Unlocked */
    __HAL_UNLOCK(huart);
    
    return HAL_OK;
  }
  else
  {
    return HAL_BUSY; 
  }
}


/**
  * @brief  通过非阻塞方式接收数据，数据长度有最大限度，但是在最大限度之内可以接受任意长度的数据 DMADMA
  * @param  huart: 指向UART_HandleTypeDef结构体的指针，该指针包含了UART的配置信息
  * @param  pData: 指向接受数据缓冲区的指针
  * @param  Size: 可接收数据的最大长度
  * @retval HAL status
  */
HAL_StatusTypeDef Bsp_UART_Receive_IT(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
{
	__HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);//开启串口空闲中断
	
	Bsp_UART_Receive_DMA(huart,pData,Size);//利用DMA接受数据

	return HAL_OK;
}


/**
  * @brief  通过非阻塞方式接收数据，数据长度有最大限度，但是在最大限度之内可以接受任意长度的数据 
  * @param  huart: 指向UART_HandleTypeDef结构体的指针，该指针包含了UART的配置信息
  * @param  Size: 可接收数据的最大长度
  * @retval None
  */
void Bsp_UsartIdleHanlder(UART_HandleTypeDef *huart,uint16_t Size)
{
	uint32_t DMA_FLAGS;//根据串口的不同来选择清除不同的DMA标志位
//  uint32_t tmp;
	
	if(__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE))
  {
				//关闭dma
	  __HAL_DMA_DISABLE(huart->hdmarx);
		
		//及时更新
		UART_IdleRxCallback(huart);
		


		
		DMA_FLAGS = __HAL_DMA_GET_TC_FLAG_INDEX(huart->hdmarx);	
		__HAL_DMA_CLEAR_FLAG(huart->hdmarx,DMA_FLAGS);
		

		
		//while(DMA_GetCmdStatus(DMA2_Stream1) != DISABLE);
	 
		__HAL_DMA_SET_COUNTER(huart->hdmarx,Size);
		
		__HAL_DMA_ENABLE(huart->hdmarx);

		
		/*清除IDLE标志位*/
    __HAL_UART_CLEAR_IDLEFLAG(huart);
		
	}
}

//CRC16打包
void ISO14443AAppendCRCA(void* Buffer, uint16_t ByteCount) {
    uint16_t Checksum = 0x6363;
    uint8_t* DataPtr = (uint8_t*) Buffer;

    while(ByteCount--) {
        uint8_t Byte = *DataPtr++;

        Byte ^= (uint8_t) (Checksum & 0x00FF);
        Byte ^= Byte << 4;

        Checksum = (Checksum >> 8) ^ ( (uint16_t) Byte << 8 ) ^
                ( (uint16_t) Byte << 3 ) ^ ( (uint16_t) Byte >> 4 );
    }

    *DataPtr++ = (Checksum >> 0) & 0x00FF;
    *DataPtr = (Checksum >> 8) & 0x00FF;
}
//CRC16解包
uint8_t ISO14443ACheckCRCA(void* Buffer, uint16_t ByteCount)
{
    uint16_t Checksum = 0x6363;
    uint8_t* DataPtr = (uint8_t*) Buffer;

    while(ByteCount--) {
        uint8_t Byte = *DataPtr++;

        Byte ^= (uint8_t) (Checksum & 0x00FF);
        Byte ^= Byte << 4;

        Checksum = (Checksum >> 8) ^ ( (uint16_t) Byte << 8 ) ^
                ( (uint16_t) Byte << 3 ) ^ ( (uint16_t) Byte >> 4 );
    }

    return (DataPtr[0] == ((Checksum >> 0) & 0xFF)) && (DataPtr[1] == ((Checksum >> 8) & 0xFF));
}

uint8_t ISO14443ACheckLen(uint8_t* Buffer)
{
  if((Buffer[0]+Buffer[1])==0xff&&Buffer[0]<PC_BUF_LEN-2)
		return 1;
	else
		return 0;
}
/**
  * @brief  串口打印发送
  * @param  UART_HandleTypeDef *huart
  * @param   void* fmt, ...
  * @retval None
  */
void UART_Printf(UART_HandleTypeDef *huart, const char* fmt, ...)
{
  uint8_t buff[128] = {0};
	uint8_t *p = buff;
	va_list ap;

	va_start(ap, fmt);
	vsprintf((char *)buff, fmt, ap);
	
	uint8_t size=0;
	while(*p++)
  {
		size++;
	}
	
	HAL_UART_Transmit(huart, buff, size, 0xff);
	va_end(ap);
}
////////////////////


///////////////////
void UART_IdleRxCallback(UART_HandleTypeDef *huart)	
{
	
  if(huart == &PTZ_HUART)  
	{
		
		 if(PC_Buf[0]==0x55)//帧头验证
	 {
		 if(ISO14443ACheckLen(PC_Buf+HEAD_LEN-2))//长度验证
		 {
		 if(ISO14443ACheckCRCA(PC_Buf,PC_Buf[2]+HEAD_LEN))//CRC16验证
		 {
			 
	   }
		 
	 }	 
		
		
		
	}
}
}

