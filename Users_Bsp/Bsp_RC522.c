/**
  ******************************************************************************
  * @file    Bsp_RC522.c
  * @author  AresZzz 
  * @version V1.0
  * @date    2018/04/17
  * @brief   RC522控制程序
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

/* Function  -----------------------------------------------------------------*/

void Write_Reg(uint8_t ucRegAddr,uint8_t ucCommand)
{
	uint8_t addr;
	addr= (ucRegAddr<<1)& 0x7E;
	zRC522_CS_Enable();
	HAL_SPI_Transmit(&hspi2,&addr, 1, 1000);
	HAL_SPI_Transmit(&hspi2,&ucCommand, 1, 1000);
	zRC522_CS_Disable();
}

uint8_t Read_Reg(uint8_t ucRegAddr)
{
	uint8_t addr,data;
	addr= ((ucRegAddr<<1)& 0x7E )| 0x80;
	zRC522_CS_Enable();
	HAL_SPI_Transmit(&hspi2,&addr, 1, 1000);
	HAL_SPI_Receive(&hspi2,&data, 1, 0xFFFF);
	zRC522_CS_Disable();
	return data;
}

void ClearBitMask(uint8_t ucRegAddr,uint8_t ucMask)  
{
    uint8_t ucTemp;
    ucTemp=Read_Reg(ucRegAddr);
    Write_Reg(ucRegAddr, ucTemp & ( ~ucMask));  // clear bit mask
}

void SetBitMask(uint8_t ucRegAddr,uint8_t ucMask)  
{
    uint8_t ucTemp;
    ucTemp = Read_Reg ( ucRegAddr );
    Write_Reg ( ucRegAddr, ucTemp | ucMask );         // set bit mask
}

void AntennaOpen(void)
{
    uint8_t uc;
    uc = Read_Reg(TxControlReg);
    if(!( uc & 0x03 ))
			SetBitMask(TxControlReg, 0x03);
}

void AntennaClose(void)
{
  ClearBitMask(TxControlReg, 0x03);
}


void RC522_Init(void)
{
	zRC522_RST_Disable();
	HAL_Delay(1);
	zRC522_RST_Enable();
	HAL_Delay(1);
	zRC522_RST_Disable() ;
	HAL_Delay(1);
	
	Write_Reg(CommandReg,0x0f);
  Read_Reg(CommandReg);
	while(Read_Reg(CommandReg)&0x10)
	{
	}
	HAL_Delay(1);
	
  Write_Reg(ModeReg,0x3D);   //定义发送和接收常用模式 和Mifare卡通讯，CRC初始值0x6363
  Write_Reg(TReloadRegL, 30 );          //16位定时器低位    
	Write_Reg(TReloadRegH, 0 );			     //16位定时器高位
  Write_Reg(TModeReg, 0x8D );				   //定义内部定时器的设置
  Write_Reg(TPrescalerReg, 0x3E );			 //设置定时器分频系数
	Write_Reg(TxAutoReg, 0x40 );				   //调制发送信号为100%ASK	
}

void RC522_A_SET(void)
{
	ClearBitMask(Status2Reg,0x08);
	Write_Reg(ModeReg,0x3D);//3F	
	Write_Reg(RxSelReg,0x86);//84
	Write_Reg(RFCfgReg,0x7F);   //4F
	Write_Reg(TReloadRegL,30);//tmoLength);// TReloadVal = 'h6a =tmoLength(dec) 
	Write_Reg(TReloadRegH,0);
	Write_Reg(TModeReg,0x8D);
	Write_Reg(TPrescalerReg,0x3E);
	HAL_Delay(1);
	AntennaOpen();//开天线
}


/*
 * 函数名：RC522_Command
 * 描述  ：通过RC522和ISO14443卡通讯
 * 输入  ：ucCommand，RC522命令字
 *         pInData，通过RC522发送到卡片的数据
 *         ucInLenByte，发送数据的字节长度
 *         pOutData，接收到的卡片返回数据
 *         pOutLenBit，返回数据的位长度
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：内部调用
 */
char RC522_Command(uint8_t ucCommand, uint8_t * pInData, uint8_t ucInLenByte, uint8_t * pOutData, uint32_t * pOutLenBit)		
{
    char cStatus = MI_ERR;
    uint8_t ucIrqEn   = 0x00;
    uint8_t ucWaitFor = 0x00;
    uint8_t ucLastBits;
    uint8_t ucN;
    uint32_t ul;
	
    switch (ucCommand)
    {
       case PCD_AUTHENT:		//Mifare认证
          ucIrqEn   = 0x12;		//允许错误中断请求ErrIEn  允许空闲中断IdleIEn
          ucWaitFor = 0x10;		//认证寻卡等待时候 查询空闲中断标志位
          break;
       case PCD_TRANSCEIVE:		//接收发送 发送接收
          ucIrqEn   = 0x77;		//允许TxIEn RxIEn IdleIEn LoAlertIEn ErrIEn TimerIEn
          ucWaitFor = 0x30;		//寻卡等待时候 查询接收中断标志位与 空闲中断标志位
          break;
       default:
         break;		 
    }
 
    Write_Reg(ComIEnReg, ucIrqEn | 0x80);		//IRqInv置位管脚IRQ与Status1Reg的IRq位的值相反 
    ClearBitMask(ComIrqReg,0x80);			//Set1该位清零时，CommIRqReg的屏蔽位清零
    Write_Reg(CommandReg, PCD_IDLE);		//写空闲命令
    SetBitMask(FIFOLevelReg, 0x80);			//置位FlushBuffer清除内部FIFO的读和写指针以及ErrReg的BufferOvfl标志位被清除
    
    for(ul = 0;ul < ucInLenByte; ul++)
		  Write_Reg(FIFODataReg, pInData[ul]);    		//写数据进FIFOdata
		
    Write_Reg(CommandReg, ucCommand);					//写命令
    if ( ucCommand == PCD_TRANSCEIVE )
			SetBitMask(BitFramingReg,0x80);  				//StartSend置位启动数据发送 该位与收发命令使用时才有效
    
    ul = 100000;//根据时钟频率调整，操作M1卡最大等待时间25ms
    do 														//认证 与寻卡等待时间	
    {
         ucN = Read_Reg(ComIrqReg);							//查询事件中断
         ul --;
    } while ((ul!= 0) && (!(ucN & 0x01)) && (!(ucN & ucWaitFor)));		//退出条件i=0,定时器中断，与写空闲命令
		//} while ((!(ucN & 0x01)) && (!(ucN & ucWaitFor)));		//退出条件i=0,定时器中断，与写空闲命令
    ClearBitMask(BitFramingReg, 0x80 );					//清理允许StartSend位
    if(ul!=0)
    {
			if (!(Read_Reg(ErrorReg) & 0x1B))			//读错误标志寄存器BufferOfI CollErr ParityErr ProtocolErr
			{
				cStatus = MI_OK;
				
				if(ucN & ucIrqEn & 0x01 )					//是否发生定时器中断
				  cStatus = MI_NOTAGERR;   
					
				if(ucCommand == PCD_TRANSCEIVE)
				{
					ucN = Read_Reg(FIFOLevelReg);			//读FIFO中保存的字节数
					ucLastBits = Read_Reg(ControlReg) & 0x07;	//最后接收到得字节的有效位数
					
					if (ucLastBits)
						* pOutLenBit = (ucN-1) * 8 + ucLastBits;   	//N个字节数减去1（最后一个字节）+最后一位的位数 读取到的数据总位数
					else
						* pOutLenBit = ucN * 8;   					//最后接收到的字节整个字节有效
					
					if ( ucN == 0 )		
            ucN = 1;    
					
					if ( ucN > MAXRLEN )
						ucN = MAXRLEN;   
					
					for ( ul = 0; ul < ucN; ul ++ )
					  pOutData [ ul ] =Read_Reg (FIFODataReg);   			
					}		
      }	
			else
				cStatus = MI_ERR;   
    }
   
   SetBitMask(ControlReg, 0x80);           // stop timer now
   Write_Reg(CommandReg, PCD_IDLE);  
   return cStatus;		
}


/*
 * 函数名：RC522_Request
 * 描述  ：寻卡
 * 输入  ：ucReq_code，寻卡方式
 *                     = 0x52，寻感应区内所有符合14443A标准的卡
 *                     = 0x26，寻未进入休眠状态的卡
 *         pTagType，卡片类型代码
 *                   = 0x4400，Mifare_UltraLight
 *                   = 0x0400，Mifare_One(S50)
 *                   = 0x0200，Mifare_One(S70)
 *                   = 0x0800，Mifare_Pro(X))
 *                   = 0x4403，Mifare_DESFire
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：外部调用
 */
char RC522_Request(uint8_t ucReq_code, uint8_t * pTagType )
{
   char cStatus;  
	 uint8_t ucComMF522Buf[MAXRLEN]; 
   uint32_t ulLen;
	
   ClearBitMask(Status2Reg, 0x08);	//清理指示MIFARECyptol单元接通以及所有卡的数据通信被加密的情况
   Write_Reg(BitFramingReg, 0x07);	//	发送的最后一个字节的 七位
   SetBitMask(TxControlReg, 0x03);	//TX1,TX2管脚的输出信号传递经发送调制的13.56的能量载波信号

   ucComMF522Buf[0]=ucReq_code;		//存入 卡片命令字

   cStatus = RC522_Command(PCD_TRANSCEIVE,ucComMF522Buf,1,ucComMF522Buf, &ulLen);	//寻卡  
  
   if ((cStatus == MI_OK) && (ulLen == 0x10))	//寻卡成功返回卡类型 
   {    
       * pTagType = ucComMF522Buf[0];
       * (pTagType+ 1) = ucComMF522Buf[1];
   }
	 
   else
     cStatus = MI_ERR;

   return cStatus;
}

/*
 * 函数名：RC522_Anticoll
 * 描述  ：防冲撞
 * 输入  ：pSnr，卡片序列号，4字节
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：外部调用
 */
 
char RC522_Anticoll(uint8_t * pSnr)
{
    char cStatus;
    uint8_t uc, ucSnr_check = 0;
    uint8_t ucComMF522Buf[MAXRLEN]; 
	  uint32_t ulLen;
    
    ClearBitMask(Status2Reg, 0x08);		//清MFCryptol On位 只有成功执行MFAuthent命令后，该位才能置位
    Write_Reg(BitFramingReg, 0x00);		//清理寄存器 停止收发
    ClearBitMask(CollReg, 0x80);			//清ValuesAfterColl所有接收的位在冲突后被清除
   
    ucComMF522Buf[0] = 0x93;	//卡片防冲突命令
    ucComMF522Buf[1] = 0x20;
   
    cStatus=RC522_Command(PCD_TRANSCEIVE, ucComMF522Buf, 2, ucComMF522Buf, &ulLen);//与卡片通信
	
    if(cStatus == MI_OK)		//通信成功
    {
			for ( uc = 0; uc < 4; uc ++ )
			{
         * ( pSnr + uc )  = ucComMF522Buf[uc];			//读出UID
         ucSnr_check ^= ucComMF522Buf[uc];
      }
			
      if ( ucSnr_check != ucComMF522Buf[uc])
				cStatus = MI_ERR;    
				 
    }
    
    SetBitMask(CollReg, 0x80 );
    return cStatus;	
}

/*
 * 函数名：CalulateCRC
 * 描述  ：用RC522计算CRC16
 * 输入  ：pIndata，计算CRC16的数组
 *         ucLen，计算CRC16的数组字节长度
 *         pOutData，存放计算结果存放的首地址
 * 返回  : 无
 * 调用  ：内部调用
 */
void CalulateCRC(uint8_t * pIndata,uint8_t ucLen,uint8_t * pOutData )
{
    uint8_t uc, ucN;
	
	
    ClearBitMask(DivIrqReg,0x04);
	
    Write_Reg(CommandReg,PCD_IDLE);
	
    SetBitMask(FIFOLevelReg,0x80);
	
    for(uc=0; uc < ucLen; uc++)
	    Write_Reg(FIFODataReg, *( pIndata + uc));   

    Write_Reg(CommandReg,PCD_CALCCRC);
    uc=0xFF;
    do 
    {
        ucN = Read_Reg(DivIrqReg);
        uc --;
    } while((uc != 0) && ! (ucN & 0x04));
		
    pOutData[0]=Read_Reg(CRCResultRegL);
    pOutData[1]=Read_Reg(CRCResultRegM);	
}

/*
 * 函数名：RC522_Select
 * 描述  ：选定卡片
 * 输入  ：pSnr，卡片序列号，4字节
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：外部调用
 */
char RC522_Select(uint8_t * pSnr )
{
    char ucN;
    uint8_t uc;
	  uint8_t ucComMF522Buf[MAXRLEN]; 
    uint32_t  ulLen;
    
    
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;
	
    for(uc=0;uc<4;uc++)
    {
    	ucComMF522Buf[uc+2] = *(pSnr + uc);
    	ucComMF522Buf[6] ^= *(pSnr + uc);
    }
		
    CalulateCRC(ucComMF522Buf,7,&ucComMF522Buf[7]);
  
    ClearBitMask(Status2Reg, 0x08);

    ucN=RC522_Command(PCD_TRANSCEIVE, ucComMF522Buf, 9, ucComMF522Buf, &ulLen);
    
    if((ucN == MI_OK) && (ulLen == 0x18))
      ucN=MI_OK;  
    else
      ucN=MI_ERR;    

    return ucN;	
}

/*
 * 函数名：RC522_AuthState
 * 描述  ：验证卡片密码
 * 输入  ：ucAuth_mode，密码验证模式
 *                     = 0x60，验证A密钥
 *                     = 0x61，验证B密钥
 *         u8 ucAddr，块地址
 *         pKey，密码
 *         pSnr，卡片序列号，4字节
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：外部调用
 */

//uint8_t status;

char RC522_AuthState(uint8_t ucAuth_mode, uint8_t ucAddr, uint8_t * pKey, uint8_t * pSnr)
{
    char cStatus;
	// uint8_t status;
	  uint8_t uc, ucComMF522Buf [ MAXRLEN ];
    uint32_t ulLen;
    
	
    ucComMF522Buf[0] = ucAuth_mode;
    ucComMF522Buf[1] = ucAddr;
	
    for(uc=0;uc<6;uc++)
	    ucComMF522Buf[uc+2] = *(pKey + uc);   
	
    for(uc=0; uc<4;uc++)
	    ucComMF522Buf[uc+8] = *(pSnr + uc);   

    cStatus=RC522_Command(PCD_AUTHENT, ucComMF522Buf, 12, ucComMF522Buf, &ulLen);
	
    if((cStatus != MI_OK) || (!(Read_Reg(Status2Reg) & 0x08 )))
      cStatus = MI_ERR;   
		
    return cStatus;
		
}

/*
 * 函数名：RC522_WriteBlock
 * 描述  ：写数据到M1卡一块
 * 输入  ：u8 ucAddr，块地址
 *         pData，写入的数据，16字节
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：外部调用
 */
char RC522_WriteBlock(uint8_t ucAddr, uint8_t * pData)
{
    char cStatus;
	  uint8_t uc, ucComMF522Buf [ MAXRLEN ];
    uint32_t ulLen;
     
    
    ucComMF522Buf[0] = PICC_WRITE;
    ucComMF522Buf[1] = ucAddr;
	
    CalulateCRC(ucComMF522Buf, 2, & ucComMF522Buf [2]);
 
    cStatus = RC522_Command(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, & ulLen );

    if((cStatus != MI_OK ) || ( ulLen != 4 ) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
      cStatus = MI_ERR;   
        
    if(cStatus == MI_OK )
    {
			//memcpy(ucComMF522Buf, pData, 16);
      for(uc = 0; uc < 16; uc ++)
			  ucComMF522Buf[uc] = *(pData + uc);  
			
      CalulateCRC(ucComMF522Buf, 16, & ucComMF522Buf [ 16 ] );

      cStatus = RC522_Command(PCD_TRANSCEIVE, ucComMF522Buf, 18, ucComMF522Buf, & ulLen );
			
			if((cStatus != MI_OK ) || (ulLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        cStatus = MI_ERR;   
    } 
		
    return cStatus;
		
}


/*
 * 函数名：RC522_ReadBlock
 * 描述  ：读取M1卡一块数据
 * 输入  ：u8 ucAddr，块地址
 *         pData，读出的数据，16字节
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：外部调用
 */
char RC522_ReadBlock(uint8_t ucAddr, uint8_t * pData)
{
    char cStatus;
	  uint8_t uc, ucComMF522Buf[MAXRLEN]; 
    uint32_t ulLen;
    

    ucComMF522Buf[0] = PICC_READ;
    ucComMF522Buf[1] = ucAddr;
	
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);
   
    cStatus = RC522_Command(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, & ulLen);
	
    if((cStatus == MI_OK) && (ulLen == 0x90))
    {
			for(uc = 0; uc < 16; uc++)
        *(pData + uc) = ucComMF522Buf[uc];   
    }
		
    else
      cStatus = MI_ERR;   
		
    return cStatus;
		
}

/*
 * 函数名：PcdHalt
 * 描述  ：命令卡片进入休眠状态
 * 输入  ：无
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：外部调用
 */
char RC522_Halt( void )
{
	uint8_t ucComMF522Buf [ MAXRLEN ]; 
	uint32_t  ulLen;
  

  ucComMF522Buf [ 0 ] = PICC_HALT;
  ucComMF522Buf [ 1 ] = 0;
	
  CalulateCRC ( ucComMF522Buf, 2, & ucComMF522Buf [ 2 ] );
 	RC522_Command ( PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, & ulLen );

  return MI_OK;
	
}

/*
 * 函数名：PcdHalt
 * 描述  ：命令卡片进入休眠状态
 * 输入  ：无
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：外部调用
 pTagType，卡片类型代码
 *                   = 0x4400，Mifare_UltraLight
 *                   = 0x0400，Mifare_One(S50)
 *                   = 0x0200，Mifare_One(S70)
 *                   = 0x0800，Mifare_Pro(X))
 *                   = 0x4403，Mifare_DESFire
 */

void PrintICData(_ICDATA data)
{
	int i;
	uint8_t SectionNum,BlockID,BlockNum;
	UART_Printf(&huart1,"Card Type：");
	switch(data.CardType[0])
	{
		case 0x44:
			if(data.CardType[1]==0x00)
				UART_Printf(&huart1,"Mifare_UltraLight");
			else if(data.CardType[1]==0x03)
				UART_Printf(&huart1,"Mifare_DESFire");
			else
				UART_Printf(&huart1,"Card Type Fail");
			break;
		case 0x04:
			UART_Printf(&huart1,"Mifare_One(S50)");break;
		case 0x08:
			UART_Printf(&huart1,"Mifare_Pro(X)");break;
		case 0x02:
			UART_Printf(&huart1,"Mifare_One(S70)");break;
		default:
			UART_Printf(&huart1,"Card Type Fail");break;
	}
	UART_Printf(&huart1,"\n");
	
	UART_Printf(&huart1,"Card UID：");
	for(i=0;i<4;i++)
	{
		UART_Printf(&huart1,"%02x ",data.CardUid[i]);
	}
	UART_Printf(&huart1,"\n");
	UART_Printf(&huart1,"\n");
	for(SectionNum=0;SectionNum<16;SectionNum++)
	{
		UART_Printf(&huart1,"Section%2d：\n",SectionNum);
		for(BlockID=0;BlockID<4;BlockID++)
		{
			UART_Printf(&huart1,"Block%d:",BlockID);
			if(BlockID<3)
			{
				for(BlockNum=0;BlockNum<16;BlockNum++)
				{
					UART_Printf(&huart1,"%02x ",data.Section[SectionNum].DataBlock[BlockID].Data[BlockNum]);
				}
			}
			else
			{
				for(BlockNum=0;BlockNum<6;BlockNum++)
				{
					UART_Printf(&huart1,"00 ");
				}
				for(BlockNum=0;BlockNum<4;BlockNum++)
				{
					UART_Printf(&huart1,"%02x ",data.Section[SectionNum].ControlBlock.ControlBit[BlockNum]);
				}
				
				for(BlockNum=0;BlockNum<6;BlockNum++)
				{
					UART_Printf(&huart1,"%02x ",data.Section[SectionNum].ControlBlock.KeyB[BlockNum]);
				}
			}
			UART_Printf(&huart1,"\n");
		}
	}
	
}

