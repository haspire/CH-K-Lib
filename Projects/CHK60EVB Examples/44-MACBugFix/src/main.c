#include "sys.h"
#include "uart.h"
#include "gpio.h"
#include "adc.h"
#include "can.h"
#include "dac.h"
#include "delay.h"
#include "dma.h"
#include "enet2.h"
#include "flash.h"
#include "ftm.h"
#include "i2c.h"
#include "lptm.h"
#include "pdb.h"
#include "pit.h"
#include "rtc.h"
#include "sd.h"
#include "spi.h"
#include "tsi.h"
#include "wdog.h"

//LED Devices
#include "led.h"
#include "led_chk60evb.h"
//KBI Devices
#include "kbi.h"
#include "kbi_chk60evb.h"
//SPI Flash Devices;
#include "spiflash.h"
//LCD Devices
#include "lcd_chk60evb.h"  
#include "ads7843.h"

//CHGUI 
#include "chgui.h"         
#include "chgui_char.h"    
#include "chgui_bmp.h"     
#include "chgui_touch.h"

//MiniShell
#include "minishell.h"

//���� ��˼���� Kinetis�̼�������
//�̼���汾 V2.4
//startup:        �����ļ�
//devices:        ����оƬ֧�ֿ�(����ʵ���õ�)
//utilities:      ����ģ��֧�ֿ�(����ʵ���õ�)
//drivers:        �̼���Դ��

/*
* ����֪ʶ:
* 1- MiniShell ���˿����ļ���Shellϵͳ
* 2- MiniShell�� ʹ�� ����ʵ��GetChar �� PutChar����Ȼ���尲װ�ṹ�� ����MINISHELL_Install����
* 3- �û���������SHELL_Register ע����� ����ע�����64���û�����
* 4- ע����ɺ����MINISHELL_Init()��ʼ��
* 5- ���º󻰵���LOOP������ʼSHELLѭ��
* 6- ����λ���ն����� ���������
*/

//ʵ��MINISHELL����Ҫ��PutChar����
static void Putc(uint8_t data)
{
	UART_SendData(UART4, data);
}
//ʵ��MINISHELL����Ҫ��GetChar����
static uint8_t Getc(void)
{
	uint8_t ch;
  while(UART_ReceiveData(UART4, &ch) == FALSE);
	return ch;
}

//�û����� LED���� ��ֻ��һ������
int LED_Control(int argc, char *argv[])
{
    //��������ֻ��2��(�ո�Ϊ�ָ���)   
    if(argc == 2)
		{
        if(!strcmp(argv[1], "ON"))
				{
            LED_Ctrl(kLED1, Bit_RESET);
				}
        if(!strcmp(argv[1], "OFF"))
				{
            LED_Ctrl(kLED1, Bit_SET);
				}
		}
		else
		{
        MINISHELL_printf("Unknown Command!\r\n");
		}
		return 0;
}


uint8_t  gCfgLoca_MAC[] = {0x22, 0x22, 0x22, 0x00, 0x00, 0x01};
extern MINISHELL_CommandTableTypeDef Command_FunEnet;
//��̫֡���ͻ�����
uint8_t  gBuffer[1520]; 
//ע�� MiniShell������װ�ṹ
MINISHELL_InstallTypeDef MiniShell_InstallStruct1 = 
{
    .ctrl_putchar = Putc,
    .ctrl_getchar = Getc,
};

//�û�����ע��ṹ
MINISHELL_CommandTableTypeDef MyCommand1 =
{
    .name = "LED",            //��������
    .maxargs = 2,             //������������
    .cmd = LED_Control,       //ʵ�ֺ����ӿ�
    .usage = "LED ON(OFF)",   //��;˵��
};

extern uint8_t gEnetFlag ;
int main(void)
{
    uint32_t len;
    uint32_t i;
    ENET_InitTypeDef ENET_InitStruct1;
    SystemClockSetup(ClockSource_EX50M,CoreClock_96M);
    DelayInit();
    LED_Init(LED_PinLookup_CHK60EVB, kNumOfLED);
    UART_DebugPortInit(UART4_RX_C14_TX_C15, 115200);
	  DisplayCPUInfo();
		
	  ENET_InitStruct1.pMacAddress = gCfgLoca_MAC;
	  ENET_Init(&ENET_InitStruct1);
	  if(ENET_MiiLinkState() == TRUE)
		{
        UART_printf("ENET Phy Connected succ.\r\n");
		}
		else
		{
			  UART_printf("ENET Phy Connected failed.\r\n");
		}
	  //��װ������
    MINISHELL_Install(&MiniShell_InstallStruct1);
    //ע��ӿں���
		MINISHELL_Register(&MyCommand1, 1);
		MINISHELL_Register(&Command_FunEnet, 1);
		
	  //��ʼ��SHELL
	  MINISHELL_Init();
	  //����SHELL
	  MINISHELL_CmdHandleLoop("SHELL>>");
		while(1)
		{
				//�ȴ������豸(PC�� ��������Ϣ)
				if(gEnetFlag == 1)
				{
						len = ENET_MacRecData(gBuffer);
						UART_printf("Enet frame received, len:%d\r\n", len);
						//��ӡ���յ�������
						for(i = 0; i < len; i++)
						{
								UART_printf("0x%x ", gBuffer[i]);
						}
						UART_printf("\r\n");
				 gEnetFlag = 0;
			 }
		}


}
