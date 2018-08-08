#ifndef __BSP_RC522_H
#define __BSP_RC522_H

#include "task_init.h"


/////////////////////////////////////////////////////////////////////
//MF522������
/////////////////////////////////////////////////////////////////////
#define PCD_IDLE              0x00               //ȡ����ǰ����
#define PCD_AUTHENT           0x0E               //��֤��Կ
#define PCD_RECEIVE           0x08               //��������
#define PCD_TRANSMIT          0x04               //��������
#define PCD_TRANSCEIVE        0x0C               //���Ͳ���������
#define PCD_RESETPHASE        0x0F               //��λ
#define PCD_CALCCRC           0x03               //CRC����

/////////////////////////////////////////////////////////////////////
//Mifare_One��Ƭ������
/////////////////////////////////////////////////////////////////////
#define PICC_REQIDL           0x26               //Ѱ��������δ��������״̬
#define PICC_REQALL           0x52               //Ѱ��������ȫ����
#define PICC_ANTICOLL1        0x93               //����ײ
#define PICC_ANTICOLL2        0x95               //����ײ
#define PICC_AUTHENT1A        0x60               //��֤A��Կ
#define PICC_AUTHENT1B        0x61               //��֤B��Կ
#define PICC_READ             0x30               //����
#define PICC_WRITE            0xA0               //д��
#define PICC_DECREMENT        0xC0               //��ֵ
#define PICC_INCREMENT        0xC1               //��ֵ
#define PICC_RESTORE          0xC2               //�������ݵ�������
#define PICC_TRANSFER         0xB0               //���滺����������
#define PICC_HALT             0x50               //����
/////////////////////////////////////////////////////////////////////
//MF522 FIFO���ȶ���
/////////////////////////////////////////////////////////////////////
#define DEF_FIFO_LENGTH       64                 //FIFO size=64byte
#define MAXRLEN  18

/////////////////////////////////////////////////////////////////////
//MF522�Ĵ�������
/////////////////////////////////////////////////////////////////////
// PAGE 0
#define     RFU00                 0x00    
#define     CommandReg            0x01    
#define     ComIEnReg             0x02    
#define     DivlEnReg             0x03    
#define     ComIrqReg             0x04    
#define     DivIrqReg             0x05
#define     ErrorReg              0x06    
#define     Status1Reg            0x07    
#define     Status2Reg            0x08    
#define     FIFODataReg           0x09
#define     FIFOLevelReg          0x0A
#define     WaterLevelReg         0x0B
#define     ControlReg            0x0C
#define     BitFramingReg         0x0D
#define     CollReg               0x0E
#define     RFU0F                 0x0F
// PAGE 1     
#define     RFU10                 0x10
#define     ModeReg               0x11
#define     TxModeReg             0x12
#define     RxModeReg             0x13
#define     TxControlReg          0x14
#define     TxAutoReg             0x15
#define     TxSelReg              0x16
#define     RxSelReg              0x17
#define     RxThresholdReg        0x18
#define     DemodReg              0x19
#define     RFU1A                 0x1A
#define     RFU1B                 0x1B
#define     MifareReg             0x1C
#define     RFU1D                 0x1D
#define     RFU1E                 0x1E
#define     SerialSpeedReg        0x1F
// PAGE 2    
#define     RFU20                 0x20  
#define     CRCResultRegM         0x21
#define     CRCResultRegL         0x22
#define     RFU23                 0x23
#define     ModWidthReg           0x24
#define     RFU25                 0x25
#define     RFCfgReg              0x26
#define     GsNReg                0x27
#define     CWGsCfgReg            0x28
#define     ModGsCfgReg           0x29
#define     TModeReg              0x2A
#define     TPrescalerReg         0x2B
#define     TReloadRegH           0x2C
#define     TReloadRegL           0x2D
#define     TCounterValueRegH     0x2E
#define     TCounterValueRegL     0x2F
// PAGE 3      
#define     RFU30                 0x30
#define     TestSel1Reg           0x31
#define     TestSel2Reg           0x32
#define     TestPinEnReg          0x33
#define     TestPinValueReg       0x34
#define     TestBusReg            0x35
#define     AutoTestReg           0x36
#define     VersionReg            0x37
#define     AnalogTestReg         0x38
#define     TestDAC1Reg           0x39  
#define     TestDAC2Reg           0x3A   
#define     TestADCReg            0x3B   
#define     RFU3C                 0x3C   
#define     RFU3D                 0x3D   
#define     RFU3E                 0x3E   
#define     RFU3F		  		        0x3F

/////////////////////////////////////////////////////////////////////
//��MF522ͨѶʱ���صĴ������
/////////////////////////////////////////////////////////////////////
#define 	MI_OK                 0x26
#define 	MI_NOTAGERR           0xcc
#define 	MI_ERR                0xbb

 
 
/*********************************** RC522 ���Ŷ��� *********************************************/

#define zRC522_NSS_GPIO_PORT  GPIOA
#define zRC522_NSS_GPIO_PIN   GPIO_PIN_4

#define zRC522_RST_GPIO_PORT  GPIOC
#define zRC522_RST_GPIO_PIN   GPIO_PIN_0


#define zRC522_RST_Enable()     HAL_GPIO_WritePin(zRC522_RST_GPIO_PORT, zRC522_RST_GPIO_PIN, GPIO_PIN_RESET)
#define zRC522_RST_Disable()    HAL_GPIO_WritePin(zRC522_RST_GPIO_PORT, zRC522_RST_GPIO_PIN, GPIO_PIN_SET)


#define zRC522_CS_Enable()      HAL_GPIO_WritePin(zRC522_NSS_GPIO_PORT, zRC522_NSS_GPIO_PIN, GPIO_PIN_RESET)
#define zRC522_CS_Disable()     HAL_GPIO_WritePin(zRC522_NSS_GPIO_PORT, zRC522_NSS_GPIO_PIN, GPIO_PIN_SET)

/*********************************** �ṹ�� *********************************************/

typedef struct 
{
	//uint8_t DataID;
	uint8_t Data[16];
	uint8_t ControlBit[4];
	uint8_t KeyA[6];
	uint8_t KeyB[6];
	
}_BLOCK;

typedef struct 
{
	//uint8_t BlockID;
	_BLOCK DataBlock[3];
	_BLOCK ControlBlock;
	
}_SECTION;

typedef struct 
{
	//uint8_t BlockNum;          //0~63
	//uint8_t SectionID;
	_SECTION Section[16];
	uint8_t CardType[4];
	uint8_t CardUid[4];
	uint8_t DafultKey[6];
}_ICDATA;



/*********************************** ���� *********************************************/
void Write_Reg(uint8_t ucRegAddr,uint8_t ucCommand);
uint8_t Read_Reg(uint8_t ucRegAddr);
void ClearBitMask(uint8_t ucRegAddr,uint8_t ucMask);
void SetBitMask(uint8_t ucRegAddr,uint8_t ucMask);
void AntennaOpen(void);
void AntennaClose(void);
void RC522_Init(void);
void RC522_A_SET(void);
char RC522_Command(uint8_t ucCommand, uint8_t * pInData, uint8_t ucInLenByte, uint8_t * pOutData, uint32_t * pOutLenBit);		
char RC522_Request ( uint8_t ucReq_code, uint8_t * pTagType );
char RC522_Anticoll(uint8_t * pSnr);
void CalulateCRC(uint8_t * pIndata,uint8_t ucLen,uint8_t * pOutData);
char RC522_Select(uint8_t * pSnr);
char RC522_AuthState(uint8_t ucAuth_mode, uint8_t ucAddr, uint8_t * pKey, uint8_t * pSnr);
char RC522_WriteBlock(uint8_t ucAddr, uint8_t * pData);
char RC522_ReadBlock(uint8_t ucAddr, uint8_t * pData);
char RC522_Halt( void );
void PrintICData(_ICDATA data);
#endif
