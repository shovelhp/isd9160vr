/******************************************************************************
 * @file     main.c
 * @version  V3.00
 * $Revision: 2 $
 * $Date: 14/07/10 10:14a $
 * @brief    Demonstrate ADC function:
 *           1. MIC to speaker.
 *           2. ADC With ALC.
 *           3. ADC Compare Monitor.
 *           4. VMID selection.
 *           5. MICBIAS selection. 
 *           6. ADC Single Mode.
 *           7. ADC Cycle Mode.
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "ISD9100.h"
/*---------------------------------------------------------------------------------------------------------*/
/* Macro, type and constant definitions                                                                    */
/*---------------------------------------------------------------------------------------------------------*/

#define abs(x)   (x>=0 ? x : -x)
#define sign(x)  (x>=0 ? '+' : '-')

#define CH          0
#define PDMA        PDMA0
#define PGA_GAIN    3525  //default 0
#define ADC_SAMPLE_RATE  (8000)  //default 16000
#define FRAME_SZIE       (4)
#define BUFFER_LENGTH    (FRAME_SZIE*2)
//#define FRAMEGAP_LENGTH       (2)
#define UARTBAUD		921600
#define BLOCK_SIZE 	ADC_SAMPLE_RATE/BUFFER_LENGTH //about 1 second per block
#define FRAMETIME 1	//
//#define FRAMESIZE FRAMETIME * BLOCK_SIZE	//
#define TRANSTIME		60 //total record length
//#define TRANSSIZE		BLOCK_SIZE * TRANSTIME+1 //total record length
#define SENDFRAMEGAP 1
#define TIMELIMITE 1

__align(4) int16_t i16Buffer[BUFFER_LENGTH];
uint16_t DPWMModFreqDiv[8] = {228, 156, 76, 52, 780, 524, 396, 268}; //Modulation Division
volatile uint8_t u8CmpMatch;
volatile uint32_t u32RecTime = 0;
volatile uint32_t u32FrameTime = FRAMETIME;
volatile uint32_t u32Cnt = TRANSTIME * BLOCK_SIZE;
volatile uint32_t u32BlockCnt = FRAMETIME * BLOCK_SIZE;
uint8_t FrameGap[] = {0x55,0xaa,0xaa,0x55,0x55,0xaa,0xaa,0x55}; 
#define FRAMEGAP_LENGTH       sizeof(FrameGap)
	
#define MENU 0

/*
volatile char cmd[32] = {0};  //command length < 32 chars
volatile char cmdc = 0;
volatile uint8_t cmd_cnt = 0;
volatile uint8_t cmd_len = 0;
*/
/*---------------------------------------------------------------------------------------------------------*/
/* Define Function Prototypes                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
void ADC_MICTest(void);

void PDMA_IRQHandler(void)
{
		
	if(PDMA_GCR->GLOBALIF & (1 << CH))
	{
		if (PDMA->CHIF & (0x4 << PDMA_CHIF_WAIF_Pos)) //Current transfer half complete flag
		{	
			PDMA->CHIF = (0x4 << PDMA_CHIF_WAIF_Pos); //Clear interrupt
			// DPWM_WriteFIFO(&i16Buffer[0], FRAME_SZIE);
			UART_Write( UART0, (uint8_t *)&i16Buffer[0], BUFFER_LENGTH);
		}
		else //Current transfer finished flag 
		{		
			PDMA->CHIF = 0x1 << PDMA_CHIF_WAIF_Pos; //Clear interrupt
/*			if(!u32RecTime)				u32Cnt = 10;
			u32Cnt--;
			if(u32Cnt <= 0)
			{
				ADC_STOP_CONV(ADC);
				UART_Write( UART0, (uint8_t *)&i16Buffer[FRAME_SZIE], BUFFER_LENGTH);
				UART_Write( UART0, FrameGap, FRAMEGAP_LENGTH);
				u32FrameTime = 0;
			}
			else if(u32FrameTime)
			{
				u32BlockCnt--;
				if(u32BlockCnt <= 0)
				{
					u32BlockCnt = u32FrameTime;
					ADC_STOP_CONV(ADC);
					UART_Write( UART0, (uint8_t *)&i16Buffer[FRAME_SZIE], BUFFER_LENGTH);
					UART_Write( UART0, FrameGap, FRAMEGAP_LENGTH);
					ADC_START_CONV(ADC);
				}
				else
					UART_Write( UART0, (uint8_t *)&i16Buffer[FRAME_SZIE], BUFFER_LENGTH);
			}
			// DPWM_WriteFIFO(&i16Buffer[FRAME_SZIE], FRAME_SZIE);
			else*/
				UART_Write( UART0, (uint8_t *)&i16Buffer[FRAME_SZIE], BUFFER_LENGTH);
			//UART_Write( UART0, FrameGap, FRAMEGAP_LENGTH);
			if(u32RecTime)  u32Cnt--;		
			if(u32FrameTime)  u32BlockCnt--;
		}
	}
		
}

void ADC_IRQHandler(void)
{
	if (ADC_GetIntFlag(ADC_CMP0_INT))
	{
		u8CmpMatch |= 0x1;
		ADC_ClearIntFlag(ADC_CMP0_INT);
	}else if (ADC_GetIntFlag(ADC_CMP1_INT))
	{
		u8CmpMatch |= 0x2;
		ADC_ClearIntFlag(ADC_CMP1_INT);
	}
}

void SYS_Init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Enable External OSC49M */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);
	
    /* Switch HCLK clock source to CLK2X a frequency doubled output of OSC48M */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKSEL0_HIRCFSEL_48M, CLK_CLKDIV0_HCLK(1));

	/* Set ADC divisor from HCLK */
    CLK_SetModuleClock(ADC_MODULE, MODULE_NoMsk, CLK_CLKDIV0_ADC(1));
	
	/* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate SystemCoreClock. */
    SystemCoreClockUpdate();

    /* Set GPG multi-function pins for UART0 RXD and TXD */
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA8MFP_Msk) ) | SYS_GPA_MFP_PA8MFP_UART_TX;
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA9MFP_Msk) ) | SYS_GPA_MFP_PA9MFP_UART_RX;

    /* Lock protected registers */
    SYS_LockReg();
}

void UART_Init(void)
{
    /* Reset IP */
	CLK_EnableModuleClock(UART_MODULE);
    SYS_ResetModule(UART0_RST);

    /* Configure UART0 and set UART0 Baudrate(115200) */
    UART_Open( UART0,UARTBAUD );
}

void ADC_Init(void)
{
	uint32_t u32Div;
	
	/* Reset IP */
	CLK_EnableModuleClock(ADC_MODULE);
	CLK_EnableModuleClock(ANA_MODULE);
    SYS_ResetModule(EADC_RST);
	SYS_ResetModule(ANA_RST);
	
	/* Enable Analog block power */
	ADC_ENABLE_SIGNALPOWER(ADC,
	                       ADC_SIGCTL_ADCMOD_POWER|
						   ADC_SIGCTL_IBGEN_POWER|
	                       ADC_SIGCTL_BUFADC_POWER|
	                       ADC_SIGCTL_BUFPGA_POWER);
	
	/* PGA Setting */
	ADC_MUTEON_PGA(ADC, ADC_SIGCTL_MUTE_PGA);
	ADC_MUTEOFF_PGA(ADC, ADC_SIGCTL_MUTE_IPBOOST);
	ADC_ENABLE_PGA(ADC, ADC_PGACTL_REFSEL_VMID, ADC_PGACTL_BOSST_GAIN_26DB);
	ADC_SetPGAGaindB(PGA_GAIN); // 0dB
	
	/* MIC circuit configuration */
	ADC_ENABLE_VMID(ADC, ADC_VMID_HIRES_DISCONNECT, ADC_VMID_LORES_CONNECT);
	ADC_EnableMICBias(ADC_MICBSEL_17V);
	ADC_SetAMUX(ADC_MUXCTL_MIC_PATH, ADC_MUXCTL_POSINSEL_NONE, ADC_MUXCTL_NEGINSEL_NONE);
	
	/* Open ADC block */
	ADC_Open();
	ADC_SET_OSRATION(ADC, ADC_OSR_RATION_192);
	u32Div = CLK_GetHIRCFreq()/ADC_SAMPLE_RATE/192;
	ADC_SET_SDCLKDIV(ADC, u32Div);
	ADC_SET_FIFOINTLEVEL(ADC, 7);
	
	ADC_MUTEOFF_PGA(ADC, ADC_SIGCTL_MUTE_PGA);
	
	ADC_DISABLE_ALC(ADC);
	
}

void DPWM_Init(void)
{
	/* Reset IP */
	CLK_EnableModuleClock(DPWM_MODULE);
	SYS_ResetModule(DPWM_RST);
	
	DPWM_Open();	 
	DPWM_SetSampleRate(ADC_SAMPLE_RATE); //Set sample rate
	DPWM_SET_MODFREQUENCY(DPWM,DPWM_CTL_MODUFRQ0);//Set FREQ_0
	
	/* Set GPG multi-function pins for SPK+ and SPK- */
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA12MFP_Msk) ) | SYS_GPA_MFP_PA12MFP_SPKP;
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA13MFP_Msk) ) | SYS_GPA_MFP_PA13MFP_SPKM;
}

void PDMA_Init(void)
{
	volatile int32_t i = 10;
	
	/* Reset IP */
	CLK_EnableModuleClock(PDMA_MODULE);
	SYS_ResetModule(PDMA_RST);

	
	PDMA_GCR->GLOCTL |= (1 << CH) << PDMA_GLOCTL_CHCKEN_Pos; //PDMA Controller Channel Clock Enable
			
	PDMA->DSCT_CTL |= PDMA_DSCT_CTL_SWRST_Msk;   //Writing 1 to this bit will reset the internal state machine and pointers
	PDMA->DSCT_CTL |= PDMA_DSCT_CTL_CHEN_Msk;    //Setting this bit to 1 enables PDMA assigned channel operation 
	while(i--);                                  //Need a delay to allow reset
	
	PDMA_GCR->SVCSEL &= 0xfffff0ff;  //DMA channel is connected to ADC peripheral transmit request.
	//PDMA_GCR->SVCSEL |= CH << PDMA_SVCSEL_DPWMTXSEL_Pos;  //DMA channel is connected to DPWM peripheral transmit request.
	PDMA_GCR->SVCSEL |= CH << PDMA_SVCSEL_ADCRXSEL_Pos;  //DMA channel is connected to ADC peripheral transmit request.

	PDMA->DSCT_ENDSA = (uint32_t)&ADC->DAT;    //Set source address
	PDMA->DSCT_ENDDA = (uint32_t)i16Buffer;    //Set destination address
	
	PDMA->DSCT_CTL |= 0x2 << PDMA_DSCT_CTL_SASEL_Pos;    //Transfer Source address is fixed.
	PDMA->DSCT_CTL |= 0x3 << PDMA_DSCT_CTL_DASEL_Pos;    //Transfer Destination Address is wrapped.
	PDMA->DSCT_CTL |= 0x2 << PDMA_DSCT_CTL_TXWIDTH_Pos;  //One half-word (16 bits) is transferred for every PDMA operation
	PDMA->DSCT_CTL |= 0x1 << PDMA_DSCT_CTL_MODESEL_Pos;  //Memory to IP mode (APB-to-SRAM).
	PDMA->DSCT_CTL |= 0x5 << PDMA_DSCT_CTL_WAINTSEL_Pos; //Wrap Interrupt: Both half and end buffer.
	
	PDMA->TXBCCH = BUFFER_LENGTH*2;          // Audio array total length, unit: sample.
	
	PDMA->INTENCH = 0x1 << PDMA_INTENCH_WAINTEN_Pos;;   //Wraparound Interrupt Enable
	
	ADC_ENABLE_PDMA(ADC);
	
	NVIC_ClearPendingIRQ(PDMA_IRQn);
	NVIC_EnableIRQ(PDMA_IRQn);
	PDMA->DSCT_CTL |= PDMA_DSCT_CTL_TXEN_Msk;    //Start PDMA transfer
}

/*---------------------------------------------------------------------------------------------------------*/
/* Main function                                                                                           */
/*---------------------------------------------------------------------------------------------------------*/
int main(void)
{
	uint8_t u8Option;
		
	/* Lock protected registers */
    if(SYS->REGLCTL == 1) // In end of main function, program issued CPU reset and write-protection will be disabled.
        SYS_LockReg();

    /* Init System, IP clock and multi-function I/O */
    SYS_Init(); //In the end of SYS_Init() will issue SYS_LockReg() to lock protected register. If user want to write protected register, please issue SYS_UnlockReg() to unlock protected register.

    /* Init UART for printf */
    UART_Init();
	
#if MENU
	while (1)
    {
		printf("\n\nCPU @ %dHz\n", SystemCoreClock);
		
		printf("\n\n");
		printf("+----------------------------------------------------------------------+\n");
		printf("|                       MIC Data Record                                |\n");
		printf("+----------------------------------------------------------------------+\n");
		printf("|  [s] Start Record                                                    |\n");       
		printf("|  [q] Quit                                                            |\n");
		printf("  Press 's' to start or 'q' to quit:");
		u8Option = getchar();
	
	
	//getchar();
	//printf("\nADC Test Begin...............\n");
		if(u8Option == 's')
        {
            printf("%c", u8Option);
            ADC_MICTest();    
        }
         else if( (u8Option == 'q') || (u8Option == 'Q') )
        {                                
            printf("\nExit.\n");
            break;
        }
	}
#else
	ADC_MICTest();    
#endif
	while(1);
}

/*---------------------------------------------------------------------------------------------------------*/
/* Define Test Function Items                                                                              */
/*---------------------------------------------------------------------------------------------------------*/

void ADC_MICTest(void)
{
	// int32_t i32PGAGain, i32RealGain;
	// uint8_t u8Div, u8Option;
	uint8_t u8Option;

	u32RecTime = 0;
	u32FrameTime = FRAMETIME;
	u32Cnt = TRANSTIME * BLOCK_SIZE;
	u32BlockCnt = FRAMETIME * BLOCK_SIZE;
	/* Init ADC */
	// i32PGAGain = PGA_GAIN;
	ADC_Init();
#if MENU
	printf("\n\n=== MIC To SPK test ===\n");
	
	/* Init PDMA to move ADC FIFO to MIC buffer with wrapped-around mode  */
	//PDMA_Init();
	printf("  The ADC and MIC configuration is ready.\n");
	printf("\n  Please Set Record Length (seconds, 0 for unlimite):");
	while(scanf("%u", &u32RecTime)!=1);
	u32Cnt = u32RecTime * BLOCK_SIZE;
	printf("\n  Please Set Frame Length (seconds, 0 for no frame header):");
	while(scanf("%u", &u32FrameTime)!=1);
	u32BlockCnt = u32FrameTime * BLOCK_SIZE;
	printf("\n  Press any key to start...\n");
	printf("  You can press [p] to exit when recording.\n");
	getchar();

	//DPWM_START_PLAY(DPWM);
	printf("\n	Change Record Status\n");
    printf("  [p] Stop Record\n");
    // printf("  [q] Exit\n");
	//fflush(stdin);
	//UART_Write( UART0, FrameGap, FRAMEGAP_LENGTH);
#endif
  printf("ADC START\n %s%d", FrameGap,0);
	PDMA_Init();
	ADC_START_CONV(ADC);
	//u32Cnt = TRANSTIME;
/*	while(1)
	{
		u8Option = getchar();
	
		if( (u8Option == 'p') || (u8Option == 'P') )
		{
			ADC_STOP_CONV(ADC);
		    printf("%s%d", FrameGap,0);
				break;
		}
	}*/
	while(u32Cnt)
	{
		if(u32Cnt <= 0)
		{
			ADC_STOP_CONV(ADC);
			// UART_Write( UART0, FrameGap, FRAMEGAP_LENGTH);
		  //printf("%s%d", FrameGap,0);
			break;
		}
		if(u32BlockCnt <= 0)
		{
			ADC_STOP_CONV(ADC);
			// UART_Write( UART0, FrameGap, FRAMEGAP_LENGTH);
			NVIC_ClearPendingIRQ(PDMA_IRQn);
			NVIC_DisableIRQ(PDMA_IRQn);
	    printf("%s%d", FrameGap,0);
			u32BlockCnt = u32FrameTime * BLOCK_SIZE;
			NVIC_ClearPendingIRQ(PDMA_IRQn);
			NVIC_EnableIRQ(PDMA_IRQn);
			ADC_START_CONV(ADC);
//			PDMA->DSCT_CTL |= PDMA_DSCT_CTL_TXEN_Msk;    //Start PDMA transfer
		}

		if(UART_IS_RX_READY(UART0))
			u8Option = UART_READ(UART0);

		// u8Option = getchar();
		if( (u8Option == 'p') || (u8Option == 'P') )
		{
			ADC_STOP_CONV(ADC);
		    //printf("%s%d", FrameGap,0);
				break;
		}
	}

	//DPWM_STOP_PLAY(DPWM);
	ADC_STOP_CONV(ADC);
	printf("%s%d", FrameGap,0);
	printf("\nRecord Process Stoped...\n");
	
	NVIC_ClearPendingIRQ(PDMA_IRQn);
	NVIC_DisableIRQ(PDMA_IRQn);
}

/*** (C) COPYRIGHT 2014 Nuvoton Technology Corp. ***/

