/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/		 

// ---------------------------------------------------------------------------------------------------------
//	Functions:
//		- System clock configuration.
//		- Keypad configuration.
//		- SPI Flash configuration.
//		- Speaker configuration.
//		- MIC configuration.
//		- Output pin configuration.
//		- UltraIO configuration.
//		- Application Initiation.
//		- Processing loop:
//			* Codec processing(use functions in "AppFunctions.c").
//			* Voice effect processing(use functions in "AppFunctions.c").
//			* Keypad check and execution actions(use functions in "InputKeyActions.c").
//			* Etc.
//	
//	Reference "Readme.txt" for more information.
// ---------------------------------------------------------------------------------------------------------

#include <stdio.h>
#include "App.h"
#include "Framework.h"
#include "Keypad.h"
#include "ConfigSysClk.h"
#include "MicSpk.h"

#include "AudioRes/Output/AudioRes_AudioInfo.h"

//#include "N5162S.h"
//#include "gpio_spi.h"

#if( !defined(__CHIP_SERIES__) )
#error "Please update SDS version >= v5.0."
#endif

// SPI flash handler.
S_SPIFLASH_HANDLER g_sSpiFlash;

// Application control.
volatile UINT8 g_u8AppCtrl;

// Application handler.
S_APP g_sApp;

static UART_T	*g_pUART = UART0;                              /////modify 20170804


uart_type ap_uart0;
pwm_type pwm0;
pwm_type pwm1;
UINT8 Fan_Stauts = 0;

UINT8 SPIFlash_Initiate(void)
{ 
	UINT16 ui16Temp;
	UINT32 ui32Temp;
	UINT32 u32Count;

	// SPI0: GPA1=SSB00, GPA2=SCLK0, GPA3=MISO0, GPA4=MOSI0 
	SYS->GPA_MFP  = 
		(SYS->GPA_MFP & (~(SYS_GPA_MFP_PA0MFP_Msk|SYS_GPA_MFP_PA1MFP_Msk|SYS_GPA_MFP_PA2MFP_Msk|SYS_GPA_MFP_PA3MFP_Msk)) )
		| (SYS_GPA_MFP_PA0MFP_SPI_MOSI0|SYS_GPA_MFP_PA1MFP_SPI_SCLK|SYS_GPA_MFP_PA2MFP_SPI_SSB0|SYS_GPA_MFP_PA3MFP_SPI_MISO0);	
	
	// Reset IP module
	CLK_EnableModuleClock(SPI0_MODULE);
	SYS_ResetModule(SPI0_RST);
	SPIFlash_Open(SPI0, SPI_SS0, SPI0_CLOCK, &g_sSpiFlash );

	// Make SPI flash leave power down mode if some where or some time had made it entring power down mode
	SPIFlash_PowerDown(&g_sSpiFlash, FALSE);
	
	// Check SPI flash is ready for accessing
	u32Count = ui32Temp = 0;
	while(u32Count!=100)
	{
		SPIFlash_Read(&g_sSpiFlash, 0, (PUINT8) &ui16Temp, 2);
		if ( ui32Temp != (UINT32)ui16Temp )
		{
			ui32Temp = (UINT32)ui16Temp;
			u32Count = 0;
		}
		else
			u32Count++;
	}

	// The following code can be remove to save code if the flash size is not necessary for this application
	SPIFlash_GetChipInfo(&g_sSpiFlash);
	if (g_sSpiFlash.u32FlashSize == 0)
		return 0;
	
	// The above code can be remove to save code if the flash size is not necessary for this application
	return 1;
}

void UART_Init(void)
{
   /* Init UART to 115200-8n1 for print message */
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA8MFP_Msk) ) | SYS_GPA_MFP_PA8MFP_UART_TX;
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA9MFP_Msk) ) | SYS_GPA_MFP_PA9MFP_UART_RX;
	
	CLK_EnableModuleClock(UART_MODULE);//使能UART外设时钟
	
  UART_Open(UART0, 115200);
	UART_ClearIntFlag(UART0,UART_INTSTS_BUFERRINT_Msk);
	UART_ClearIntFlag(UART0,UART_INTSTS_RLSINT_Msk);
	UART_ClearIntFlag(UART0,UART_INTSTS_MODEMINT_Msk);
	UART_ClearIntFlag(UART0,UART_INTSTS_RXTOINT_Msk);
	UART0->INTEN |=(1<<0); //使能接收中断
  NVIC_EnableIRQ(UART0_IRQn);                      ////modify 20170804 
}

void pwm_init(void)
{
	pwm0.period = PWM_PERIOD;
	pwm0.rate = PWM_RATE;
	pwm0.Duty = 0;
/*  	if (USE_PWM0)
  	{
		pwm0.Duty = PWM_DUTY_INIT;
	}
	else
	{
		pwm0.Duty = 0;
	}
*/	

	CLK_SetModuleClock(PWM0_MODULE, CLK_CLKSEL1_PWM0CH01SEL_HCLK, 0);
	CLK_EnableModuleClock(PWM0_MODULE);
	SYS_ResetModule(PWM0_RST);
	
	// Set GPA multi-function pins for PWM0 Channel0
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA12MFP_Msk) ) | SYS_GPA_MFP_PA12MFP_PWM0CH0;
	//set PWM0 channel 0 output configuration 
  	PWM_ConfigOutputChannel(PWM0, PWM_CH0, pwm0.rate, pwm0.Duty);
  	//Enable PWM Output path for PWM0 channel 0 
  	PWM_EnableOutput(PWM0, 0x1);
	// Enable PWM channel 0 period interrupt
	PWM0->INTEN |= PWM_INTEN_PIEN0_Msk;
	NVIC_EnableIRQ(PWM0_IRQn);
	  
	PWM_Start(PWM0, 0x01);

  	if (USE_PWM1)
  	{
		pwm1.period = PWM_PERIOD;
		// pwm1.Duty = PWM_DUTY_INIT;
		pwm1.Duty = 0;
		pwm1.rate = PWM_RATE;
		PWM_DisableDeadZone(PWM0, 0);
		// Set GPA multi-function pins for PWM0 Channel1
		SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA13MFP_Msk) ) | SYS_GPA_MFP_PA13MFP_PWM0CH1;
		//set PWM0 channel 1 output configuration 
		PWM_ConfigOutputChannel(PWM0, PWM_CH1, pwm1.rate, pwm1.Duty);
		//Enable PWM Output path for PWM0 channel 0 and channel 1 
		PWM_EnableOutput(PWM0, 0x2);
		// Enable PWM channel 1 period interrupt
		PWM0->INTEN |= PWM_INTEN_PIEN1_Msk;
		PWM_Start(PWM0, 0x02);
  	}
	// PWM_Start(PWM0, 0x03);
	//GPIO_SET_OUT_DATA(PA, GPIO_GET_OUT_DATA(PA) & (~BIT12));
}
/*
void pwm_init_1(void)
{
	pwm1.period =200;
	pwm1.Duty =50;
	pwm1.rate = 30000;
	
	CLK_SetModuleClock(PWM0_MODULE, CLK_CLKSEL1_PWM0CH01SEL_HCLK, 0);
	CLK_EnableModuleClock(PWM0_MODULE);
	SYS_ResetModule(PWM0_MODULE);
	
	// Set GPA multi-function pins for PWM0 Channel1 
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA13MFP_Msk) ) | SYS_GPA_MFP_PA13MFP_PWM0CH1;
	//set PWM0 channel 1 output configuration 
  PWM_ConfigOutputChannel(PWM0, PWM_CH1, pwm1.rate, pwm1.Duty);
  //Enable PWM Output path for PWM0 channel 1 
  PWM_EnableOutput(PWM0, 0x2);
	// Enable PWM channel 1 period interrupt
  PWM0->INTEN = PWM_INTEN_PIEN1_Msk;
  NVIC_EnableIRQ(PWM0_IRQn);
    
  PWM_Start(PWM0, 0x00);
}*/

void time_init(void)
{
	CLK_EnableModuleClock(TMR0_MODULE);  //使能time0外设时钟
	CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_HCLK, 0);   /////modify 20170804
	TIMER_Open(TIMER0, TIMER_PERIODIC_MODE, 1000);
	NVIC_EnableIRQ(TMR0_IRQn);
	TIMER_Start(TIMER0);
	TIMER_EnableInt(TIMER0);
	
}

uint16_t vr_time=0;
uint16_t main_init_time=2000;
uint8_t spk_sta=0;
uint16_t PB_PIN_STA=0;
void TMR0_IRQHandler(void)
{
	static uint8_t time_10ms=0;

	static uint16_t tt=0;
	static uint8_t ll=0;
	if(pwm0.Breath_light)
	{
		tt++;
		if(tt>10)
		{
			tt =0;
			if(ll==0)
			{
				pwm0.Duty ++;
				if((pwm0.Duty)>= pwm0.period) ll=1;
			}
			else
			{
				if(pwm0.Duty)	pwm0.Duty--;
				else ll=0;
			}
		}
	}
	
	if(time_10ms)	time_10ms--;
	else
	{
		time_10ms=10;
		
		PB_PIN_STA <<=1;
		if(((PB->PIN)&(1<<6))==0)	PB_PIN_STA+=1;
		
		if (PB_PIN_STA==0xfF00)	spk_sta++;
	}
	
	if(ap_uart0.uart_rx_time)	ap_uart0.uart_rx_time --;
    //printf("Timer IRQ handler test #%d/3.\n", ++u8Counter );
    TIMER_ClearIntFlag(TIMER0);	
	if(vr_time)	vr_time--;
}

void UART0_IRQHandler(void)   ////modify 20170804
{
	uint32_t  u32IntStatus;
	u32IntStatus= UART0->INTSTS;
	
if(u32IntStatus & (1<<0))	//接收中断
	{
		ap_uart0.uart_rx_data[ap_uart0.uart_rx_len] = UART0->DAT;
		if((ap_uart0.uart_rx_len+1) <UART_DA)	ap_uart0.uart_rx_len ++;	//预防越界
		
		ap_uart0.uart_rx_time = UART_TT;
	}
	else if(u32IntStatus & (1<<1))	//发送缓冲空中断
	{
		UART0->INTEN &= ~(1<<1);     /////THREIEN
		if(ap_uart0.uart_tx_len)
		{
			UART0->DAT= ap_uart0.uart_tx_data[ap_uart0.uart_tx_per]  ;
			ap_uart0.uart_tx_len--;
			ap_uart0.uart_tx_per ++;
			UART0->INTEN |=(1<<1);
		}
		else
		{
			ap_uart0.uart_tx_per =0;
			UART0->INTEN &= ~(1<<1);		//发送完毕，关闭发送中断
			//UART0->INTEN |=(1<<1);
		}
	}

} 

void uart_send(uint8_t *dara, uint16_t len)
{
	uint16_t ii;
	UART0->INTEN |=(1<<1);
	for(ii=0; ii<len; ii++)
	{
		ap_uart0.uart_tx_data[ii] = *dara++;
	}
	ap_uart0.uart_tx_len =len;
	if(ap_uart0.uart_tx_len==0)	return;
	while((g_pUART->FIFOSTS &(1<<22)) == 0)	;
	g_pUART->DAT= ap_uart0.uart_tx_data[0];//发送
	ap_uart0.uart_tx_per++;
	ap_uart0.uart_tx_len --;
	UART0->INTEN |=(1<<1);	//打开发送中断
}


void uart_rev_task(void)
{
	if((ap_uart0.uart_rx_time==0)&&(ap_uart0.uart_rx_len))//有接收到数据
	{

		//uart_send(ap_uart0.uart_rx_data	, ap_uart0.uart_rx_len);	//发送接收到的数据

		ap_uart0.uart_rx_len =0;
	}
}



void PWM0_IRQHandler(void)
{
	if (PWM_GetIntFlag(PWM0, PWM_CH0))
	{
    	// Update PWM0 channel 0 period and duty
    	PWM_SET_CNR(PWM0, PWM_CH0, pwm0.period);
    	PWM_SET_CMR(PWM0, PWM_CH0, pwm0.Duty);
    	// Clear channel 0 period interrupt flag
    	PWM_ClearIntFlag(PWM0, PWM_CH0);
	}
	if(USE_PWM1)
	{
		if (PWM_GetIntFlag(PWM0, PWM_CH1))
		{
	    	// Update PWM0 channel 1 period and duty
	    	PWM_SET_CNR(PWM0, PWM_CH1, pwm1.period);
	    	PWM_SET_CMR(PWM0, PWM_CH1, pwm1.Duty);
	    	// Clear channel 0 period interrupt flag
	    	PWM_ClearIntFlag(PWM0, PWM_CH1);
		}
	}
}



uint8_t spk_add=0;
uint8_t key_spk;
uint8_t io_aa;
//---------------------------------------------------------------------------------------------------------
// Main Function                                                           
//---------------------------------------------------------------------------------------------------------
INT32 main()
{

	SYSCLK_INITIATE();								// Configure CPU clock source and operation clock frequency.
																		// The configuration functions are in "ConfigSysClk.h"

	CLK_EnableLDO(CLK_LDOSEL_3_3V);		// Enable interl 3.3 LDO.

	if (! SPIFlash_Initiate())				// Initiate SPI interface and checking flows for accessing SPI flash.
	while(1); 												// loop here for easy debug
	

	OUTPUTPIN_INITIATE();							// Initiate output pin configuration.
//																		// The output pins configurations are defined in "ConfigIO.h".

	ULTRAIO_INITIATE();								// Initiate ultraio output configurations.
																		// The ultraio output pin configurations are defined in "ConfigUltraIO.h"

	KEYPAD_INITIATE();								// Initiate keypad configurations including direct trigger key and matrix key
																		// The keypad configurations are defined in "ConfigIO.h".

	PDMA_INITIATE();									// Initiate PDMA.
																		// After initiation, the PDMA engine clock NVIC are enabled.
																		// Use PdmaCtrl_Open() to set PDMA service channel for desired IP.
																		// Use PdmaCtrl_Start() to trigger PDMA operation.
																		// Reference "PdmaCtrl.h" for PDMA related APIs.
																		// PDMA_INITIATE() must be call before SPK_INITIATE() and MIC_INITIATE(), if open MIC or speaker.

	SPK_INITIATE();										// Initiate speaker including pop-sound canceling.
																		// After initiation, the APU is paused.
																		// Use SPK_Resume(0) to start APU operation.
																		// Reference "MicSpk.h" for speaker related APIs.

	MIC_INITIATE();										// Initiate MIC.
																		// After initiation, the ADC is paused.
																		// Use ADC_Resume() to start ADC operation.
																		// Reference "MicSpk.h" for MIC related APIs.


	UART_Init();   
	time_init();
	pwm_init();																	
//			
	App_Initiate();										// Initiate application for audio decode.
	
//	GPIO_SetMode(PA, 12, GPIO_MODE_OUTPUT);
	
	//App_StartPlay(0);
	printf("\n\n");
	printf("INIT Finished!\n");
	printf("Starting VR...\n");
	while (1)
	{
		
		App_Process();	//语音识别
		uart_rev_task();	//uart任务
		
		
		//if((g_u8AppCtrl & APPCTRL_PLAY)==0)	 App_StartPlay(1);	//判断当前是否播放完毕
		
		
//		TRIGGER_KEY_CHECK();		// Check and execute direct trigger key actions defined in "InputKeyActions.c"
														// Default trigger key handler is "Default_KeyHandler()"
														// The trigger key configurations are defined in "ConfigIO.h".
		
//		MATRIX_KEY_CHECK();			// Check and execute matrix key actions defined in "InputKeyActions.c"
														// Default matrix key handler is "Default_KeyHandler()"
														// The matrix key configurations are defined in "ConfigIO.h".
	}

}

