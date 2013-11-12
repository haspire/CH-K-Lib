/**
  ******************************************************************************
  * @file    nrf2401.c
  * @author  YANDLD
  * @version V2.4
  * @date    2013.5.23
  * @brief   ����ԭ�Ӻ˺��İ� BSP���� NRF2401����
  ******************************************************************************
  */
#include "nrf2401.h"

NRF2401_InitTypeDef* gpNRF2401;


/***********************************************************************************************
 ���ܣ�NRF24L01 ��дһ���ֽ�
 �βΣ����͵�����
 ���أ�����������
 ��⣺����SPIģ��
************************************************************************************************/
static uint8_t NRF2401_ReadWriteByte(uint8_t dat)
{
	uint8_t RecByte = 0;
	RecByte = SPI_ReadWriteByte(gpNRF2401->DATA_SPI_CSMap,dat,SPI_PCS_Inactive);
	return(RecByte);           		  // return read byte
}
/***********************************************************************************************
 ���ܣ�NRF24L01 д�Ĵ���
 �βΣ�reg: �Ĵ���
       value:д���ֵ
 ���أ�����������
 ��⣺��
************************************************************************************************/
static uint8_t NRF2401_ReadWriteReg(uint8_t reg,uint8_t value)
{
	uint8_t status;
	GPIO_ResetBits((GPIO_Type*) gpNRF2401->CS_GPIO_Instance, gpNRF2401->CS_GPIO_Pin);
	status = NRF2401_ReadWriteByte(reg);      // select register
	NRF2401_ReadWriteByte(value);             // ..and write value to it..
	GPIO_SetBits((GPIO_Type*) gpNRF2401->CS_GPIO_Instance, gpNRF2401->CS_GPIO_Pin);
	return(status);            // return nRF24L01 status byte
}
/***********************************************************************************************
 ���ܣ�NRF24L01 д�Ĵ���
 �βΣ�reg: �Ĵ���
       value:д���ֵ
 ���أ�����������
 ��⣺��
************************************************************************************************/
static uint8_t NRF2401_WriteBuffer(uint8_t reg,uint8_t *pBuf,uint8_t bytes)
{
	uint8_t status,i;
	GPIO_ResetBits((GPIO_Type*) gpNRF2401->CS_GPIO_Instance, gpNRF2401->CS_GPIO_Pin);
	status = NRF2401_ReadWriteByte(reg);    // Select register to write to and read status byte
	for(i=0; i<bytes; i++) // then write all byte in buffer(*pBuf)
	{
		NRF2401_ReadWriteByte(*pBuf++);
	}
	GPIO_SetBits((GPIO_Type*) gpNRF2401->CS_GPIO_Instance, gpNRF2401->CS_GPIO_Pin);
	return(status);          // return nRF24L01 status byte
}
/***********************************************************************************************
 ���ܣ�NRF24L01 ��Buffer
 �βΣ�reg: �Ĵ���
       pBuf:����ָ��
		 	 bytes: д����ֽ���
 ���أ���״̬
 ��⣺��
************************************************************************************************/
static uint8_t NRF2401_ReadBuffer(uint8_t reg,uint8_t *pBuf,uint8_t bytes)
{
	uint8_t status,i;
	GPIO_ResetBits((GPIO_Type*) gpNRF2401->CS_GPIO_Instance, gpNRF2401->CS_GPIO_Pin);
	status = NRF2401_ReadWriteByte(reg);       		// Select register to write to and read status byte
	for(i=0;i<bytes;i++)
	{
		pBuf[i] = NRF2401_ReadWriteByte(0);    // Perform SPI_RW to read byte from nRF24L01
	}
	GPIO_SetBits((GPIO_Type*) gpNRF2401->CS_GPIO_Instance, gpNRF2401->CS_GPIO_Pin);
	return(status);                    // return nRF24L01 status byte
}

/***********************************************************************************************
 ���ܣ����NRF24L01 �Ƿ����
 �βΣ�
 ���أ�NRF_OK : �ɹ�   NRF_ERR:ʧ��
 ��⣺��
************************************************************************************************/
 uint8_t NRF24L01_Check(void)
{
	uint8_t buf[5]={0XA5,0XA5,0XA5,0XA5,0XA5};
	uint8_t i;
	NRF2401_WriteBuffer(WRITE_REG+TX_ADDR,buf,5);//д��5���ֽڵĵ�ַ.	
	NRF2401_ReadBuffer(TX_ADDR,buf,5); //����д��ĵ�ַ  
	for(i=0;i<5;i++)
	{
		if(buf[i] != 0xA5) return NRF_ERR;
	}
	return NRF_OK;
}	 	

uint8_t NRF2401_Init(NRF2401_InitTypeDef *NRF2401_InitStruct)
{
	SPI_InitTypeDef SPI_InitStruct1;
	GPIO_InitTypeDef GPIO_InitStruct1;
	gpNRF2401 = NRF2401_InitStruct;
	if(gpNRF2401 == NULL)
	{
		return 1;
	}
	//CE���ų�ʼ��
	GPIO_InitStruct1.GPIO_Pin = NRF2401_InitStruct->CE_GPIO_Pin;
	GPIO_InitStruct1.GPIO_InitState = Bit_SET;
	GPIO_InitStruct1.GPIO_IRQMode = GPIO_IT_DISABLE;
	GPIO_InitStruct1.GPIO_Mode = GPIO_Mode_OPP;
	GPIO_InitStruct1.GPIOx = (GPIO_Type*) NRF2401_InitStruct->CE_GPIO_Instance;
	GPIO_Init(&GPIO_InitStruct1);

	 //IRQ���ų�ʼ��
	GPIO_InitStruct1.GPIO_Pin = NRF2401_InitStruct->IRQ_GPIO_Pin;
	GPIO_InitStruct1.GPIO_InitState = Bit_SET;
	GPIO_InitStruct1.GPIO_IRQMode = GPIO_IT_DISABLE;
	GPIO_InitStruct1.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct1.GPIOx = (GPIO_Type*) NRF2401_InitStruct->IRQ_GPIO_Instance;
	GPIO_Init(&GPIO_InitStruct1);
	//��ʼ��SPI���нӿ�����
	SPI_InitStruct1.SPIxDataMap = NRF2401_InitStruct->DATA_SPI_DataMap;
	SPI_InitStruct1.SPIxPCSMap =  NRF2401_InitStruct->DATA_SPI_CSMap;
	SPI_InitStruct1.SPI_DataSize = 8;
	SPI_InitStruct1.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
	SPI_InitStruct1.SPI_Mode = SPI_Mode_Master;
	SPI_InitStruct1.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStruct1.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStruct1.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_Init(&SPI_InitStruct1);
	//��ʼ��Ӳ��GPIO����
	//������Ӳ�� CS ʹ��GPIO��CS
	GPIO_InitStruct1.GPIO_Pin = NRF2401_InitStruct->CS_GPIO_Pin;
	GPIO_InitStruct1.GPIO_InitState = Bit_SET;
	GPIO_InitStruct1.GPIO_IRQMode = GPIO_IT_DISABLE;
	GPIO_InitStruct1.GPIO_Mode = GPIO_Mode_OPP;
	GPIO_InitStruct1.GPIOx = (GPIO_Type*) NRF2401_InitStruct->CS_GPIO_Instance;
	GPIO_Init(&GPIO_InitStruct1);
	//IO��ƽ��ʼ��
	GPIO_ResetBits((GPIO_Type*) gpNRF2401->CE_GPIO_Instance, gpNRF2401->CE_GPIO_Pin);// chip enable
	GPIO_ResetBits((GPIO_Type*) gpNRF2401->CS_GPIO_Instance, gpNRF2401->CS_GPIO_Pin);// cs disable
	//���NRF2401 �Ƿ����
	return 0;
}

const uint8_t TX_ADDRESS[TX_ADR_WIDTH]={0x34,0x43,0x10,0x10,0x01}; //���͵�ַ
const uint8_t RX_ADDRESS[RX_ADR_WIDTH]={0x34,0x43,0x10,0x10,0x01}; //���յ�ַ
//�ú�����ʼ��NRF24L01��RXģʽ
//����RX��ַ,дRX���ݿ���,ѡ��RFƵ��,�����ʺ�LNA HCURR
//��CE��ߺ�,������RXģʽ,�����Խ���������		   
void NRF2401_SetRXMode(void)
{
	GPIO_ResetBits((GPIO_Type*) gpNRF2401->CE_GPIO_Instance, gpNRF2401->CE_GPIO_Pin);// chip enable
	NRF2401_WriteBuffer(WRITE_REG+RX_ADDR_P0,(uint8_t*)RX_ADDRESS,RX_ADR_WIDTH);//дRX�ڵ��ַ
	NRF2401_ReadWriteReg(WRITE_REG+EN_AA,0x01);    //ʹ��ͨ��0���Զ�Ӧ��    
	NRF2401_ReadWriteReg(WRITE_REG+EN_RXADDR,0x01);//ʹ��ͨ��0�Ľ��յ�ַ  	 
	NRF2401_ReadWriteReg(WRITE_REG+RF_CH,40);	     //����RFͨ��Ƶ��		  
	NRF2401_ReadWriteReg(WRITE_REG+RX_PW_P0,RX_PLOAD_WIDTH);//ѡ��ͨ��0����Ч���ݿ��� 	    
	NRF2401_ReadWriteReg(WRITE_REG+RF_SETUP,0x0f);//����TX�������,0db����,2Mbps,���������濪��   
	NRF2401_ReadWriteReg(WRITE_REG+CONFIG, 0x0f);//���û�������ģʽ�Ĳ���;PWR_UP,EN_CRC,16BIT_CRC,����ģʽ 
  GPIO_SetBits((GPIO_Type*) gpNRF2401->CE_GPIO_Instance, gpNRF2401->CE_GPIO_Pin); //CEΪ��,�������ģʽ 
}			
//�ú�����ʼ��NRF24L01��TXģʽ
//����TX��ַ,дTX���ݿ���,����RX�Զ�Ӧ��ĵ�ַ,���TX��������,ѡ��RFƵ��,�����ʺ�LNA HCURR
//PWR_UP,CRCʹ��
//��CE��ߺ�,������RXģʽ,�����Խ���������		   
//CEΪ�ߴ���10us,����������.	 
void NRF2401_SetTXMode(void)
{				
  GPIO_ResetBits((GPIO_Type*) gpNRF2401->CE_GPIO_Instance, gpNRF2401->CE_GPIO_Pin); 
	NRF2401_WriteBuffer(WRITE_REG+TX_ADDR,(uint8_t*)TX_ADDRESS,TX_ADR_WIDTH);//дTX�ڵ��ַ 
	NRF2401_WriteBuffer(WRITE_REG+RX_ADDR_P0,(uint8_t*)RX_ADDRESS,RX_ADR_WIDTH); //����TX�ڵ��ַ,��ҪΪ��ʹ��ACK	  
	NRF2401_ReadWriteReg(WRITE_REG+EN_AA,0x01);     //ʹ��ͨ��0���Զ�Ӧ��    
	NRF2401_ReadWriteReg(WRITE_REG+EN_RXADDR,0x01); //ʹ��ͨ��0�Ľ��յ�ַ  
	NRF2401_ReadWriteReg(WRITE_REG+SETUP_RETR,0x1a);//�����Զ��ط����ʱ��:500us + 86us;����Զ��ط�����:10��
	NRF2401_ReadWriteReg(WRITE_REG+RF_CH,40);       //����RFͨ��Ϊ40
	NRF2401_ReadWriteReg(WRITE_REG+RF_SETUP,0x0f);  //����TX�������,0db����,2Mbps,���������濪��   
	NRF2401_ReadWriteReg(WRITE_REG+CONFIG,0x0e);    //���û�������ģʽ�Ĳ���;PWR_UP,EN_CRC,16BIT_CRC,����ģʽ,���������ж�
  GPIO_SetBits((GPIO_Type*) gpNRF2401->CE_GPIO_Instance, gpNRF2401->CE_GPIO_Pin); //CEΪ��,�������ģʽ 
}


//����NRF24L01����һ������
//txbuf:�����������׵�ַ
//����ֵ:�������״��
uint8_t NRF2401_SendData(uint8_t *txbuf)
{
	uint8_t sta;
  GPIO_ResetBits((GPIO_Type*) gpNRF2401->CE_GPIO_Instance, gpNRF2401->CE_GPIO_Pin); 
	NRF2401_WriteBuffer(WR_TX_PLOAD,txbuf,TX_PLOAD_WIDTH);//д���ݵ�TX BUF  32���ֽ�
 	GPIO_SetBits((GPIO_Type*) gpNRF2401->CE_GPIO_Instance, gpNRF2401->CE_GPIO_Pin); //��������	  
	while(GPIO_ReadInputDataBit((GPIO_Type*) gpNRF2401->IRQ_GPIO_Instance, gpNRF2401->IRQ_GPIO_Pin) != 0);//�ȴ��������
	sta = NRF2401_ReadWriteReg(STATUS,0xFF);  //��ȡ״̬�Ĵ�����ֵ	
	NRF2401_ReadWriteReg(WRITE_REG+STATUS,sta); //���TX_DS��MAX_RT�жϱ�־
	
	if(sta&MAX_TX)//�ﵽ����ط�����
	{
		NRF2401_ReadWriteReg(FLUSH_TX,0xff);//���TX FIFO�Ĵ��� 
		return MAX_TX; 
	}
	if(sta&TX_OK)//�������
	{
		return TX_OK;
	}
	return 0xff;//����ԭ����ʧ��
}

//����NRF24L01����һ������
//txbuf:�����������׵�ַ
//����ֵ:0��������ɣ��������������
uint8_t NRF2401_RecData(uint8_t *rxbuf)
{
	uint8_t sta;		    							   
	sta=NRF2401_ReadWriteReg(STATUS,0xFF);  //��ȡ״̬�Ĵ�����ֵ    	 
	NRF2401_ReadWriteReg(WRITE_REG+STATUS,sta); //���TX_DS��MAX_RT�жϱ�־
	if(sta&RX_OK)//���յ�����
	{
		NRF2401_ReadBuffer(RD_RX_PLOAD,rxbuf,RX_PLOAD_WIDTH);//��ȡ����
		NRF2401_ReadWriteReg(FLUSH_RX,0xff);//���RX FIFO�Ĵ��� 
		return 0; 
	}	   
	return 1;//û�յ��κ�����
}			


