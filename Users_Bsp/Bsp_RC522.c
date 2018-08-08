/**
  ******************************************************************************
  * @file    Bsp_RC522.c
  * @author  AresZzz 
  * @version V1.0
  * @date    2018/04/17
  * @brief   RC522���Ƴ���
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
	
  Write_Reg(ModeReg,0x3D);   //���巢�ͺͽ��ճ���ģʽ ��Mifare��ͨѶ��CRC��ʼֵ0x6363
  Write_Reg(TReloadRegL, 30 );          //16λ��ʱ����λ    
	Write_Reg(TReloadRegH, 0 );			     //16λ��ʱ����λ
  Write_Reg(TModeReg, 0x8D );				   //�����ڲ���ʱ��������
  Write_Reg(TPrescalerReg, 0x3E );			 //���ö�ʱ����Ƶϵ��
	Write_Reg(TxAutoReg, 0x40 );				   //���Ʒ����ź�Ϊ100%ASK	
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
	AntennaOpen();//������
}


/*
 * ��������RC522_Command
 * ����  ��ͨ��RC522��ISO14443��ͨѶ
 * ����  ��ucCommand��RC522������
 *         pInData��ͨ��RC522���͵���Ƭ������
 *         ucInLenByte���������ݵ��ֽڳ���
 *         pOutData�����յ��Ŀ�Ƭ��������
 *         pOutLenBit���������ݵ�λ����
 * ����  : ״ֵ̬
 *         = MI_OK���ɹ�
 * ����  ���ڲ�����
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
       case PCD_AUTHENT:		//Mifare��֤
          ucIrqEn   = 0x12;		//��������ж�����ErrIEn  ��������ж�IdleIEn
          ucWaitFor = 0x10;		//��֤Ѱ���ȴ�ʱ�� ��ѯ�����жϱ�־λ
          break;
       case PCD_TRANSCEIVE:		//���շ��� ���ͽ���
          ucIrqEn   = 0x77;		//����TxIEn RxIEn IdleIEn LoAlertIEn ErrIEn TimerIEn
          ucWaitFor = 0x30;		//Ѱ���ȴ�ʱ�� ��ѯ�����жϱ�־λ�� �����жϱ�־λ
          break;
       default:
         break;		 
    }
 
    Write_Reg(ComIEnReg, ucIrqEn | 0x80);		//IRqInv��λ�ܽ�IRQ��Status1Reg��IRqλ��ֵ�෴ 
    ClearBitMask(ComIrqReg,0x80);			//Set1��λ����ʱ��CommIRqReg������λ����
    Write_Reg(CommandReg, PCD_IDLE);		//д��������
    SetBitMask(FIFOLevelReg, 0x80);			//��λFlushBuffer����ڲ�FIFO�Ķ���дָ���Լ�ErrReg��BufferOvfl��־λ�����
    
    for(ul = 0;ul < ucInLenByte; ul++)
		  Write_Reg(FIFODataReg, pInData[ul]);    		//д���ݽ�FIFOdata
		
    Write_Reg(CommandReg, ucCommand);					//д����
    if ( ucCommand == PCD_TRANSCEIVE )
			SetBitMask(BitFramingReg,0x80);  				//StartSend��λ�������ݷ��� ��λ���շ�����ʹ��ʱ����Ч
    
    ul = 100000;//����ʱ��Ƶ�ʵ���������M1�����ȴ�ʱ��25ms
    do 														//��֤ ��Ѱ���ȴ�ʱ��	
    {
         ucN = Read_Reg(ComIrqReg);							//��ѯ�¼��ж�
         ul --;
    } while ((ul!= 0) && (!(ucN & 0x01)) && (!(ucN & ucWaitFor)));		//�˳�����i=0,��ʱ���жϣ���д��������
		//} while ((!(ucN & 0x01)) && (!(ucN & ucWaitFor)));		//�˳�����i=0,��ʱ���жϣ���д��������
    ClearBitMask(BitFramingReg, 0x80 );					//��������StartSendλ
    if(ul!=0)
    {
			if (!(Read_Reg(ErrorReg) & 0x1B))			//�������־�Ĵ���BufferOfI CollErr ParityErr ProtocolErr
			{
				cStatus = MI_OK;
				
				if(ucN & ucIrqEn & 0x01 )					//�Ƿ�����ʱ���ж�
				  cStatus = MI_NOTAGERR;   
					
				if(ucCommand == PCD_TRANSCEIVE)
				{
					ucN = Read_Reg(FIFOLevelReg);			//��FIFO�б�����ֽ���
					ucLastBits = Read_Reg(ControlReg) & 0x07;	//�����յ����ֽڵ���Чλ��
					
					if (ucLastBits)
						* pOutLenBit = (ucN-1) * 8 + ucLastBits;   	//N���ֽ�����ȥ1�����һ���ֽڣ�+���һλ��λ�� ��ȡ����������λ��
					else
						* pOutLenBit = ucN * 8;   					//�����յ����ֽ������ֽ���Ч
					
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
 * ��������RC522_Request
 * ����  ��Ѱ��
 * ����  ��ucReq_code��Ѱ����ʽ
 *                     = 0x52��Ѱ��Ӧ�������з���14443A��׼�Ŀ�
 *                     = 0x26��Ѱδ��������״̬�Ŀ�
 *         pTagType����Ƭ���ʹ���
 *                   = 0x4400��Mifare_UltraLight
 *                   = 0x0400��Mifare_One(S50)
 *                   = 0x0200��Mifare_One(S70)
 *                   = 0x0800��Mifare_Pro(X))
 *                   = 0x4403��Mifare_DESFire
 * ����  : ״ֵ̬
 *         = MI_OK���ɹ�
 * ����  ���ⲿ����
 */
char RC522_Request(uint8_t ucReq_code, uint8_t * pTagType )
{
   char cStatus;  
	 uint8_t ucComMF522Buf[MAXRLEN]; 
   uint32_t ulLen;
	
   ClearBitMask(Status2Reg, 0x08);	//����ָʾMIFARECyptol��Ԫ��ͨ�Լ����п�������ͨ�ű����ܵ����
   Write_Reg(BitFramingReg, 0x07);	//	���͵����һ���ֽڵ� ��λ
   SetBitMask(TxControlReg, 0x03);	//TX1,TX2�ܽŵ�����źŴ��ݾ����͵��Ƶ�13.56�������ز��ź�

   ucComMF522Buf[0]=ucReq_code;		//���� ��Ƭ������

   cStatus = RC522_Command(PCD_TRANSCEIVE,ucComMF522Buf,1,ucComMF522Buf, &ulLen);	//Ѱ��  
  
   if ((cStatus == MI_OK) && (ulLen == 0x10))	//Ѱ���ɹ����ؿ����� 
   {    
       * pTagType = ucComMF522Buf[0];
       * (pTagType+ 1) = ucComMF522Buf[1];
   }
	 
   else
     cStatus = MI_ERR;

   return cStatus;
}

/*
 * ��������RC522_Anticoll
 * ����  ������ײ
 * ����  ��pSnr����Ƭ���кţ�4�ֽ�
 * ����  : ״ֵ̬
 *         = MI_OK���ɹ�
 * ����  ���ⲿ����
 */
 
char RC522_Anticoll(uint8_t * pSnr)
{
    char cStatus;
    uint8_t uc, ucSnr_check = 0;
    uint8_t ucComMF522Buf[MAXRLEN]; 
	  uint32_t ulLen;
    
    ClearBitMask(Status2Reg, 0x08);		//��MFCryptol Onλ ֻ�гɹ�ִ��MFAuthent����󣬸�λ������λ
    Write_Reg(BitFramingReg, 0x00);		//����Ĵ��� ֹͣ�շ�
    ClearBitMask(CollReg, 0x80);			//��ValuesAfterColl���н��յ�λ�ڳ�ͻ�����
   
    ucComMF522Buf[0] = 0x93;	//��Ƭ����ͻ����
    ucComMF522Buf[1] = 0x20;
   
    cStatus=RC522_Command(PCD_TRANSCEIVE, ucComMF522Buf, 2, ucComMF522Buf, &ulLen);//�뿨Ƭͨ��
	
    if(cStatus == MI_OK)		//ͨ�ųɹ�
    {
			for ( uc = 0; uc < 4; uc ++ )
			{
         * ( pSnr + uc )  = ucComMF522Buf[uc];			//����UID
         ucSnr_check ^= ucComMF522Buf[uc];
      }
			
      if ( ucSnr_check != ucComMF522Buf[uc])
				cStatus = MI_ERR;    
				 
    }
    
    SetBitMask(CollReg, 0x80 );
    return cStatus;	
}

/*
 * ��������CalulateCRC
 * ����  ����RC522����CRC16
 * ����  ��pIndata������CRC16������
 *         ucLen������CRC16�������ֽڳ���
 *         pOutData����ż�������ŵ��׵�ַ
 * ����  : ��
 * ����  ���ڲ�����
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
 * ��������RC522_Select
 * ����  ��ѡ����Ƭ
 * ����  ��pSnr����Ƭ���кţ�4�ֽ�
 * ����  : ״ֵ̬
 *         = MI_OK���ɹ�
 * ����  ���ⲿ����
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
 * ��������RC522_AuthState
 * ����  ����֤��Ƭ����
 * ����  ��ucAuth_mode��������֤ģʽ
 *                     = 0x60����֤A��Կ
 *                     = 0x61����֤B��Կ
 *         u8 ucAddr�����ַ
 *         pKey������
 *         pSnr����Ƭ���кţ�4�ֽ�
 * ����  : ״ֵ̬
 *         = MI_OK���ɹ�
 * ����  ���ⲿ����
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
 * ��������RC522_WriteBlock
 * ����  ��д���ݵ�M1��һ��
 * ����  ��u8 ucAddr�����ַ
 *         pData��д������ݣ�16�ֽ�
 * ����  : ״ֵ̬
 *         = MI_OK���ɹ�
 * ����  ���ⲿ����
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
 * ��������RC522_ReadBlock
 * ����  ����ȡM1��һ������
 * ����  ��u8 ucAddr�����ַ
 *         pData�����������ݣ�16�ֽ�
 * ����  : ״ֵ̬
 *         = MI_OK���ɹ�
 * ����  ���ⲿ����
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
 * ��������PcdHalt
 * ����  �����Ƭ��������״̬
 * ����  ����
 * ����  : ״ֵ̬
 *         = MI_OK���ɹ�
 * ����  ���ⲿ����
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
 * ��������PcdHalt
 * ����  �����Ƭ��������״̬
 * ����  ����
 * ����  : ״ֵ̬
 *         = MI_OK���ɹ�
 * ����  ���ⲿ����
 pTagType����Ƭ���ʹ���
 *                   = 0x4400��Mifare_UltraLight
 *                   = 0x0400��Mifare_One(S50)
 *                   = 0x0200��Mifare_One(S70)
 *                   = 0x0800��Mifare_Pro(X))
 *                   = 0x4403��Mifare_DESFire
 */

void PrintICData(_ICDATA data)
{
	int i;
	uint8_t SectionNum,BlockID,BlockNum;
	UART_Printf(&huart1,"Card Type��");
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
	
	UART_Printf(&huart1,"Card UID��");
	for(i=0;i<4;i++)
	{
		UART_Printf(&huart1,"%02x ",data.CardUid[i]);
	}
	UART_Printf(&huart1,"\n");
	UART_Printf(&huart1,"\n");
	for(SectionNum=0;SectionNum<16;SectionNum++)
	{
		UART_Printf(&huart1,"Section%2d��\n",SectionNum);
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

