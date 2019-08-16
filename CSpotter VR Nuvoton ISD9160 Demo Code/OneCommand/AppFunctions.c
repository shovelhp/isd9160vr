/*---------------------------------------------------------------------------------------------------------*/
/*																										   */
/* Copyright (c) Nuvoton Technology	Corp. All rights reserved.											   */
/*																										   */
/*---------------------------------------------------------------------------------------------------------*/

// ---------------------------------------------------------------------------------------------------------
//	Functions:
//		- Functions to handle main operations:
//			* Initiate application.
//			* Start audio playback.
//			* Stop  audio playback.
//			* Produce PCM data for audio playback.
//			* Start audio recording.
//			* Stop  audio recording.
//			* Use recorded data to do:
//				a. encoding
//				b. doing voice effect
//				c. write to storage
//				d. etc.
//		- The above functions use codes in "xxxApp" folder and "Utility" folder to complete necessary operations.
//
//	Reference "Readme.txt" for more information.
// ---------------------------------------------------------------------------------------------------------
#include <stdio.h>
#include "App.h"
#include "MicSpk.h"
#include "AudioRom.h"
#include "PreLoad.h"
//#include "VRModelsID.h"
//#include "OneCommandAllBinID.h"
#include "AudioRes/Output/AudioRes_AudioInfo.h"
#include "AudioRes/Output/AudioResText.h"
//#include "DrvGPIO.h"

UINT8 u8TotalAudioNum=0;


extern S_SPIFLASH_HANDLER g_sSpiFlash;
extern UINT32 SPIFlash_ReadDataCallback(void *pu8Buf, UINT32 u32StartAddr, UINT32 u32Count);
extern UINT8  SPIFlash_Initiate(void);

extern void PowerDown_Enter(void);
extern void PowerDown(void);
extern void PowerDown_Exit(void);

extern S_APP g_sApp;
extern volatile UINT8 g_u8AppCtrl;
extern uint32_t VR_count;

extern char * AudioResStr[];

// Continue spk control.
volatile UINT8 g_u8Con_Spk;

extern pwm_type pwm0;
extern pwm_type pwm1;
extern uint8_t Fan_Stauts;
extern uint8_t Fan_StautsLine;
extern volatile uint8_t flag_1p25ms;
extern volatile uint8_t sendbline;
#if USEDUMYCMD
extern volatile uint8_t flag_150ms;
extern volatile uint8_t flag_0p5ms;
#endif

//---------------------------------------------------------------------------------------------------------
// Function: App_Initiate
//
// Description:
//	Initiate application.
//
// Argument:
//
// Return:
//
//---------------------------------------------------------------------------------------------------------
void App_Initiate(void)
{

	g_u8AppCtrl = APPCTRL_NO_ACTION;

// #ifndef NOFLASH
	// Initiate the audio playback.
	Playback_Initiate();
// #endif

	// Initiate the buffer control from MIC.
	MicGetApp_Initiate(&g_sApp.Mic_MD4.sMicGetApp);

	#if ( ULTRAIO_FW_CURVE_ENABLE )
	NVIC_SetPriority(ULTRAIO_TMR_IRQ, 0);
	#endif

//#ifndef NOFLASH
#if USEFLASH
	u8TotalAudioNum= AudioRom_GetAudioNum( SPIFlash_ReadDataCallback, 0 );
#endif

	g_sApp.u32AudioStartAddr=0;

	Fan_Stauts = FAN_CLOSED;

//	SPIFlash_ReadDataCallback((PUINT8) &g_sApp.u32AudioStartAddr, 8 + (8*AudioRes_AudioInfoMerge), 4);
	// Read VR model start address in SPIFlash
//	SPIFlash_ReadDataCallback((PUINT8) &g_sApp.u32VRModelStartAddr, 8 + (8*VRModelsMerged), 4);

//	if (PreLoad_VRBin((UINT32)VRCMD_START_ADDR, VRCMD_MAX_SIZE, g_sApp.u32VRModelStartAddr, N570C_VR_Wakeup_pack) == FALSE)
//		while(1);



	//??VR bin ???		//Must adjust parameter with model amount
	if (VROneCmdApp_UnpackBin(&g_sApp.sVRapp, (uint32_t)VRCMD_START_ADDR) == 0)
		while(1);

	// Light stand by(PA14) led for initial ready().
//	OUT3(0);

	// Light (PA12) led for load VR model group1.
//	OUT1(0);
//    OUT1(1);
//    OUT2(1);

	App_StartRecognize();
}

//---------------------------------------------------------------------------------------------------------
// Function: App_StartRecognize
//
// Description:
//	Start to recognize.
//
// Return:
// 	FALSE: fail
//	TRUE:  success
//---------------------------------------------------------------------------------------------------------
BOOL App_StartRecognize(void)
{
	// Initiate the voice recognition.
	if (VROneCmdApp_Initiate(&g_sApp.sVRapp) == NULL )
		return FALSE;

	MicGetApp_StartRec(&g_sApp.Mic_MD4.sMicGetApp, VRONECMDAPP_SAMPLE_RATE);

	return TRUE;
}

//---------------------------------------------------------------------------------------------------------
// Function: App_StartRecognize
//
// Description:
//	Stop to recognize.
//
// Return:
// 	void
//---------------------------------------------------------------------------------------------------------
void App_StopRecognize(void)
{
	MicGetApp_StopRec(&g_sApp.Mic_MD4.sMicGetApp);
	VROneCmdApp_UnInitiate(&g_sApp.sVRapp);
}

//---------------------------------------------------------------------------------------------------------
// Function: App_StartPlayMIDI
//
// Description:
//	Start MD4 audio playback.
//
// Return:
// 	FALSE: fail
//	TRUE:  success
//---------------------------------------------------------------------------------------------------------
BOOL App_StartPlay(UINT32 u32PlayID)
{
	// Start MD4 decode lib to decode MD4 file stored from current audio id and played from audio channel 0.
	// The ROM file is placed on SPI Flash address "g_sApp.u32AudioStartAddr".
	// And decode the first frame of PCMs.

//#ifndef NOFLASH
#if USEFLASH
	MicGetApp_StopRec(&g_sApp.Mic_MD4.sMicGetApp);	//2017-9-5 hwh  stop mic

	MD4App_DecodeInitiate(&g_sApp.Mic_MD4.asMD4App, NULL, 0);

	if ( MD4App_DecodeStartPlayByID(&g_sApp.Mic_MD4.asMD4App, u32PlayID, g_sApp.u32AudioStartAddr, 0) == FALSE )
		return FALSE;

	// Start Ultraio Timer & HW pwm for UltraIO curve output
	ULTRAIO_START();

	// Start to playback audio.
	Playback_StartPlay();
#endif
	return TRUE;
}

//---------------------------------------------------------------------------------------------------------
// Description:
//	Stop audio playback.
//
// Return:
// 	FALSE: fail
//	TRUE:  success
//---------------------------------------------------------------------------------------------------------
BOOL App_StopPlay(void)
{
//#ifndef NOFLASH
#if USEFLASH
	// Stop speaker.
	Playback_StopPlay();

	MD4App_DecodeStopPlay(&g_sApp.Mic_MD4.asMD4App);

	// Stop Ultraio Timer & HW pwm.
	ULTRAIO_STOP();
#endif

	return TRUE;
}

#define LED_ON	PA->DOUT &= (~(1<<12))
#define LED_OFF	PA->DOUT |= (1<<12)


uint8_t *vr_uart_send(uint8_t num)
{
	static uint8_t re_data[4];
	re_data[0] =0xaa;
	re_data[1] =0x11;
	re_data[2] =num;
	re_data[3] =(re_data[0] + re_data[1]+ re_data[2]);

	return re_data;
}


//---------------------------------------------------------------------------------------------------------
// Function: App_Process
//
// Description:
//   Produce PCM data for audio playback; Voice recognition
//
// Return:
//	void
//---------------------------------------------------------------------------------------------------------
extern uint16_t vr_time;
extern pwm_type pwm0;
extern void uart_send(uint8_t *dara, uint16_t len);
	uint8_t i32ID = 0;
void App_Process(void)
{
	INT16 *pi16BuffAddr;

	UINT16 u16GetSamples = 0;
	//uint16_t u16CMDforMCU = 0;
	uint8_t u8CMDforMCU = 0;
	uint8_t flag_CMD = 0;
//	static BOOL VR_flag = FALSE;
#if DEBUGTIMER
	// GPIO_TOGGLE_BIT(PA, FANON);
	if(flag_150ms)
	{
		GPIO_TOGGLE_BIT(PA, WAVEON);
		flag_150ms =  0;
	}
	if(flag_0p5ms)
	{
		GPIO_TOGGLE_BIT(PA, FANON);
		flag_0p5ms =  0;
	}
#endif
	// Play response
	if (vr_time<=0)
	{
		//GPIO_SET_OUT_DATA(PA, GPIO_GET_OUT_DATA(PA) & (~VRTIMELED));
		GPIO_CLR_BIT(PA, VRTIMELED);
	}
#if USESTATLINE
	Fan_Stauts = Fan_StautsLine;
	if(Fan_Stauts == FAN_RUNING)
		GPIO_SET_BIT(PA, FANON);
	else
		GPIO_CLR_BIT(PA, FANON);
#endif
#if DEBUGSTATLINE
	if (Fan_Stauts == FAN_RUNING)
	{
		printf("Fan is Runing\n");
	}
#endif
	if( g_u8AppCtrl&APPCTRL_PLAY )
	{
		if (MD4App_DecodeProcess(&g_sApp.Mic_MD4.asMD4App) == FALSE )
		{
			// Stop to decode audio data from ROM file for stoping to play audio codec.
			// Remove audio codec output buffer from play channel.
			App_StopPlay();
			// Start mic after play response
			MicGetApp_StartRec(&g_sApp.Mic_MD4.sMicGetApp, VRONECMDAPP_SAMPLE_RATE);
		}
	}
	else
	{
		u16GetSamples = MicGetApp_ProcessRec(&g_sApp.Mic_MD4.sMicGetApp, (UINT32 **)&pi16BuffAddr);

		//VR application needs 7.2~7.8ms computing in 10ms (160 samples) interval,
		//Other application must work completely at 2.2~2.8ms if cooperation with VR.
		if ((i32ID = VROneCmdApp_GetCMDID(&g_sApp.sVRapp, pi16BuffAddr, u16GetSamples)) != 0xff)
		{
				// Stop mic before play response
				//MicGetApp_StopRec(&g_sApp.Mic_MD4.sMicGetApp);

			if(i32ID==0)	//唤醒词：智能电扇
			{
				if((Fan_Stauts == FAN_RUNING)| USEWAKEUP )
				{
					vr_time=VRTIME;
#if USEWAKEUPRES
					App_StartPlay(0);	//播放回答声音
#endif					
				}
				else
				{
					vr_time=VRTIMEON;
				}
				// GPIO_SET_OUT_DATA(PA, GPIO_GET_OUT_DATA(PA) | VRTIMELED);
				GPIO_SET_BIT(PA, VRTIMELED);
#if USEUART
				printf("\n");
				printf("Command is : %s\n", AudioResStr[i32ID]);
				printf("%s\n", AudioOptStr[i32ID]);
#endif				
			}
			else if(vr_time)	//在唤醒计时内
			{
				if(Fan_Stauts == FAN_CLOSED)
				{
					if (i32ID == 1)
					{
						// GPIO_SET_OUT_DATA(PA, GPIO_GET_OUT_DATA(PA) | FANON);
						GPIO_SET_BIT(PA, FANON);
						//pwm1.Duty = PWM_DUTY_INIT;
						//pwm0.Duty = PWM_DUTY_INIT;
						//printf("Fan on!\n");
						//App_StartPlay(1);
						// printf("Command is : %s\n", AudioResStr[i32ID]);
						//u16CMDforMCU = i32ID;
						u8CMDforMCU = i32ID;
						flag_CMD = 1;
						//SendCMD(u8CMDforMCU);
						Fan_Stauts = FAN_RUNING;
						vr_time = 0;
						//App_StartPlay(i32ID);
						//printf("%s\n", AudioOptStr[i32ID]);
					}
				}
				else
				{
					vr_time=VRTIME;
					/*if (i32ID <= MAXIDNUM)
					{
						printf("Command is : %s\n", AudioResStr[i32ID]);
					}*/
					switch(i32ID)
					{
/*						case 1:	//开风扇
							GPIO_SET_OUT_DATA(PA, GPIO_GET_OUT_DATA(PA) | FANON);
							pwm1.Duty = PWM_DUTY_INIT;
							pwm0.Duty = PWM_DUTY_INIT;
							App_StartPlay(i32ID);
							printf("%s\n", AudioOptStr[i32ID]);
						break;*/
						case 2:	//关风扇
							// printf("Command is : %s\n", AudioResStr[i32ID]);
							// GPIO_SET_OUT_DATA(PA, GPIO_GET_OUT_DATA(PA) & (~FANON));
							// GPIO_SET_OUT_DATA(PA, GPIO_GET_OUT_DATA(PA) & (~WAVEON));
							GPIO_CLR_BIT(PA, FANON);
							GPIO_CLR_BIT(PA, WAVEON);
							//GPIO_SET_OUT_DATA(PA, 0);
							pwm1.Duty = 0;
							pwm0.Duty = 0;
							vr_time = 0;
							u8CMDforMCU = i32ID;
							flag_CMD = 1;
							// SendCMD(u8CMDforMCU);
							Fan_Stauts = FAN_CLOSED;
							// App_StartPlay(i32ID);
							// printf("%s\n", AudioOptStr[i32ID]);
						break;
#if USEALLCMD
						case 3:	//请摇头
							// GPIO_SET_OUT_DATA(PA, GPIO_GET_OUT_DATA(PA) | WAVEON);
							GPIO_SET_BIT(PA, WAVEON);
							// vr_time = 0;
							u8CMDforMCU = i32ID;
							flag_CMD = 1;
							// SendCMD(u8CMDforMCU);
							// App_StartPlay(i32ID);
							// printf("%s\n", AudioOptStr[i32ID]);
						break;
						case 4:	//关摇头
							// GPIO_SET_OUT_DATA(PA, GPIO_GET_OUT_DATA(PA) & (~WAVEON));
							GPIO_CLR_BIT(PA, WAVEON);
							// vr_time = 0;
							u8CMDforMCU = i32ID;
							flag_CMD = 1;
							// SendCMD(u8CMDforMCU);
							// App_StartPlay(i32ID);
							// printf("%s\n", AudioOptStr[i32ID]);
						break;
						case 5://增加风量
							// GPIO_SET_OUT_DATA(PA, GPIO_GET_OUT_DATA(PA) | WINDUP);
							// GPIO_SET_OUT_DATA(PA, GPIO_GET_OUT_DATA(PA) & (~WINDDOWN));
						  	if (USE_PWM1)
						  	{
								pwm1.Breath_light =0;
								pwm1.Duty /= PWM_INT;
								pwm1.Duty *= PWM_INT;
								if(pwm1.Duty < pwm1.period)	//已调暗
								{
									PWM_Start(PWM0, 0x2);
									pwm1.Duty += PWM_STEP;
									if(pwm1.Duty >= PWM_HIGH)	
										pwm1.Duty = PWM_HIGH;
								}
						  	}
							// vr_time = 0;
							u8CMDforMCU = i32ID;
							flag_CMD = 1;
							// SendCMD(u8CMDforMCU);
							// App_StartPlay(i32ID);
							// printf("%s\n", AudioOptStr[i32ID]);
						break;
						case 6://减少风量
							// GPIO_SET_OUT_DATA(PA, GPIO_GET_OUT_DATA(PA) | WINDDOWN);
							// GPIO_SET_OUT_DATA(PA, GPIO_GET_OUT_DATA(PA) & (~WINDUP));
						  	if (USE_PWM1)
						  	{
								pwm1.Breath_light =0;
								pwm1.Duty /= PWM_INT;
								pwm1.Duty *= PWM_INT;
								if(pwm1.Duty)	//已调亮
								{
									PWM_Start(PWM0, 0x2);
									if(pwm1.Duty <= PWM_LOW)	
										pwm1.Duty = PWM_LOW;
									else
										pwm1.Duty -= PWM_STEP;
								}
							}
							// vr_time = 0;
							u8CMDforMCU = i32ID;
							flag_CMD = 1;
							// SendCMD(u8CMDforMCU);
							// App_StartPlay(i32ID);
							// printf("%s\n", AudioOptStr[i32ID]);
						break;
						case 7://增加定时
							// GPIO_SET_OUT_DATA(PA, GPIO_GET_OUT_DATA(PA) | TIMEADD);
							// GPIO_SET_OUT_DATA(PA, GPIO_GET_OUT_DATA(PA) & (~TIMESUB));
						  	if (USE_PWM0)
						  	{
								pwm0.Breath_light =0;
								pwm0.Duty /= PWM_INT;
								pwm0.Duty *= PWM_INT;
								if(pwm0.Duty < pwm0.period)	//已调暗
								{
									PWM_Start(PWM0, 0x1);
									pwm0.Duty += PWM_STEP;
									if(pwm0.Duty >= PWM_HIGH)	
										pwm0.Duty = PWM_HIGH;
								}
							}
							// vr_time = 0;
							u8CMDforMCU = i32ID;
							flag_CMD = 1;
							// SendCMD(u8CMDforMCU);
							// App_StartPlay(i32ID);
							// printf("%s\n", AudioOptStr[i32ID]);
						break;
						case 8://减少定时
							// GPIO_SET_OUT_DATA(PA, GPIO_GET_OUT_DATA(PA) | TIMESUB);
							// GPIO_SET_OUT_DATA(PA, GPIO_GET_OUT_DATA(PA) & (~TIMEADD));
						  	if (USE_PWM0)
						  	{
								pwm0.Breath_light =0;
								pwm0.Duty /= PWM_INT;
								pwm0.Duty *= PWM_INT;
								if(pwm0.Duty)	//已调亮
								{
									PWM_Start(PWM0, 0x1);
									if(pwm0.Duty <= PWM_LOW)	
										pwm0.Duty = PWM_LOW;
									else
										pwm0.Duty -= PWM_STEP;
								}
							}
							// vr_time = 0;
							u8CMDforMCU = i32ID;
							flag_CMD = 1;
							// SendCMD(u8CMDforMCU);
							// App_StartPlay(i32ID);
							// printf("%s\n", AudioOptStr[i32ID]);
						break;
#endif
						default:
							//App_StartPlay(i32ID);//播放对应回应声音
							// printf("%s\n", AudioResStr[i32ID]);//串口打印命令内容
							u8CMDforMCU = i32ID;
							flag_CMD = 0;
						break;
					}
				}
/*				if ((i32ID > 1) && (i32ID <= MAXIDNUM))
				{
					App_StartPlay(i32ID);
					printf("%s\n", AudioOptStr[i32ID]);
				}*/
				//if(i32ID < 11)	 App_StartPlay(i32ID+1);	//播放
				//uart_send(vr_uart_send(i32ID+32), 4);
			}
		// else return;

		}
	}
#if USEDUMYCMD	
	if(flag_CMD)
	{
		while(!flag_150ms);
	}
	if(flag_150ms)
	{
		SendCMD(u8CMDforMCU, flag_CMD);
		// App_StartPlay(i32ID);
		// printf("%s\n", AudioOptStr[i32ID]);
		flag_150ms = 0;
	}
#else
		SendCMD(u8CMDforMCU, flag_CMD);
#endif
		return;
}



//---------------------------------------------------------------------------------------------------------
// Function: App_PowerDown
//
// Description:
//   Process flow of power-down for application. Include,
//   1. App_PowerDownProcess:Pre-process befor entering power-down.
//   2. PowerDown:Power down base process(PowerDown.c).
//   3. App_WakeUpProcess:Post-process after exiting power-down.
//   User could disable or enable this flow from flag(POWERDOWN_ENABLE) in ConfigApp.h.
//---------------------------------------------------------------------------------------------------------
void App_PowerDown(void)
{
	App_StopRecognize();
	App_StopPlay();

	#if(POWERDOWN_ENABLE)
	PowerDown_Enter();
	PowerDown();
	PowerDown_Exit();
	#endif
	// Initiate the audio playback.
	Playback_Initiate();

	// Initiate the buffer control from MIC.
	MicGetApp_Initiate(&g_sApp.Mic_MD4.sMicGetApp);

	App_StartRecognize();

//	if (g_sApp.u8VRModelID == ISD9160C_Demo_Group1_pack)
//		OUT1(0);
//	else
//		OUT2(0);

}
/*
void delay1p25ms(void)
{
	uint32_t loopi = 0;
	//uint32_t loopj = 0;
	for (loopi = 0; loopi < MS1P25CNT; ++loopi);
	return;
}
*/
/*
void delay1p25ms(void)
{
	uint32_t loopi = 0;
	uint32_t count = CLK_GetHCLKFreq() / 800;
	for (loopi = 0; loopi < count; ++loopi);
	return;
}
*/
void delay1p25ms(void)
{
	flag_1p25ms = 0;
	while(!flag_1p25ms);
	return;
}
/*
void delayms(uint32_t ms)
{
	uint32_t loopi = 0;
	uint32_t loopj = 0;
	for (loopi = 0; loopi < ms; ++loopi)
	{
		for (loopj = 0; loopj < MSCNT; ++loopj);
	}
	return;
}
*/
/*
void delayms(uint32_t ms)
{
	uint32_t loopi = 0;
	uint32_t SysFreq = CLK_GetHCLKFreq() / 1000;
	uint32_t count = ms * SysFreq;
	for (loopi = 0; loopi < count; ++loopi);
	return;
}
*/
void delayms(uint32_t ms)
{
	uint32_t loop = ms * 4 / 5;
	while(loop--)
	{
		flag_1p25ms = 0;
		while(!flag_1p25ms);
	}
	return;
}
/*
void delayus(uint32_t us)
{
	uint32_t loopi = 0;
	for (loopi = 0; loopi < us; ++loopi)
	{
		for (loopj = 0; loopj < USCNT; ++loopj)
		{
			// code 
		}
	}
	return;
}

void delay50us(void)
{
	uint32_t loopi = 0;
	for (loopi = 0; loopi < 50; ++loopi)
	{
		for (loopj = 0; loopj < USCNT; ++loopj)
		{
			// code 
		}
	}
	return;
}
*/

/*
void SendCMDByte(uint8_t u8CMDByte)
{
	uint32_t loopi = 0;
	GPIO_SET_OUT_DATA(PA, GPIO_GET_OUT_DATA(PA) | SCMDLINE);
	delay1p25ms();
	GPIO_SET_OUT_DATA(PA, GPIO_GET_OUT_DATA(PA) & (~SCMDLINE));
	delayms(20);
	GPIO_SET_OUT_DATA(PA, GPIO_GET_OUT_DATA(PA) | SCMDLINE);
	delay1p25ms();
	for(loopi = 0; loopi < 8; ++loopi)
	{
		GPIO_SET_OUT_DATA(PA, GPIO_GET_OUT_DATA(PA) & (~SCMDLINE));
		delay1p25ms();
		if (u8CMDByte && 0x1)
		{
			delay1p25ms();
		}
		GPIO_SET_OUT_DATA(PA, GPIO_GET_OUT_DATA(PA) | SCMDLINE);
		delay1p25ms();
		u8CMDByte = u8CMDByte >> 1;
	}
		delay1p25ms();
	return;
}
*/
/*
void SetSignalLine(uint32_t SignalLine, uint8_t Val)
{
	uint32_t PAdata = GPIO_GET_OUT_DATA(PA) & (~SignalLine);
	GPIO_SET_OUT_DATA(PA, PAdata | (Val << SignalLine));
	return;
}
*/
void SendCMDByteTimer(uint8_t u8CMDByte)
{
	uint8_t time20ms = 16;
	uint8_t loopi = 0;
	uint8_t sendbit = 1;
	flag_1p25ms = 0;
	sendbline = 1;
	//TIMER_Start(TIMER1);
	while(!flag_1p25ms);
	while(time20ms--)
	{
		sendbline = 0;
		flag_1p25ms = 0;
		while(!flag_1p25ms);
		//time20ms--;
	}
	sendbline = 1;
	flag_1p25ms = 0;
	for(loopi = 0; loopi < 8; loopi++)
	{
		sendbit = (u8CMDByte >> loopi) & 0x1;
		while(!flag_1p25ms);
			flag_1p25ms = 0;	
			sendbline = 0;
		if(sendbit)
		{//if sbit=1, pulse = 2.5ms
			while(!flag_1p25ms);
			sendbline = 0;
			flag_1p25ms = 0;	
		}
		while(!flag_1p25ms);
		sendbline = 1;
		flag_1p25ms = 0;
	}
	while(!flag_1p25ms);
	sendbline = 1;
	flag_1p25ms = 0;
	//TIMER_Stop(TIMER1);
	return;
}

void SendCMD1time(uint8_t u8CMDforMCU, uint8_t u8SendTimes)
{
	uint8_t u8CMD0 = 0;
	uint8_t u8CMD1 = 0;
	uint8_t u8CRC = 0;
	u8CMD0 |= (1 << (u8SendTimes-1)) << SENDTIMEOFFSET;
	SendCMDByteTimer(u8CMD0);
	u8CMD1 = u8CMDforMCU;
	SendCMDByteTimer(u8CMD1);
	u8CRC = u8CMD0 + u8CMD1;
	SendCMDByteTimer(u8CRC);
	return;
}

void SendCMD1time1B(uint8_t u8CMDforMCU)
{
	uint8_t u8CMD0 = 0;
	u8CMD0 = (u8CMDforMCU&0x0f) | (((~u8CMDforMCU)<<4)&0xf0);
	SendCMDByteTimer(u8CMD0);
	return;
}
/*
void SendCMD(uint8_t u8CMDforMCU)
{//send command 3 times
	uint8_t u8SendTimes = 1;
	for (u8SendTimes = 1; u8SendTimes <= SENDCMDTIMES ; u8SendTimes++)
	{
		SendCMD1time(u8CMDforMCU, u8SendTimes);
		delayms(CMDGAPTIME);
	}
	return;
}
*/
#if USEDUMYCMD
void SendCMDNULL(void)
{
	uint8_t time2ms = TIME2MSINIT;
	// flag_0p5ms = 0;
	sendbline = 1;
	// TIMER_Start(TIMER1);
	while(!flag_0p5ms);
	while(time2ms--)
	{
		sendbline = 0;
		flag_0p5ms = 0;
		while(!flag_0p5ms);
		//time20ms--;
	}
	sendbline = 1;
	flag_0p5ms = 0;
	while(!flag_0p5ms);
	sendbline = 0;
	flag_0p5ms = 0;
	while(!flag_0p5ms);
	sendbline = 1;
	flag_0p5ms = 0;
	while(!flag_0p5ms);
	sendbline = 0;
	flag_0p5ms = 0;
	while(!flag_0p5ms);
	sendbline = 0;
	flag_0p5ms = 0;
	while(!flag_0p5ms);
	sendbline = 1;
	flag_0p5ms = 0;
	//TIMER_Stop(TIMER1);
	return;
}
#endif

void SendCMD(uint8_t u8CMDforMCU, uint8_t flag_CMD)
{//send command 3 times
	uint8_t u8SendTimes = 1;
	if (flag_CMD)
	{
		for (u8SendTimes = 1; u8SendTimes <= SENDCMDTIMES ; u8SendTimes++)
		{
			// SendCMD1time(u8CMDforMCU, u8SendTimes);
			SendCMD1time1B(u8CMDforMCU);
			delayms(CMDGAPTIME);
		}
		App_StartPlay(i32ID);
#if USEUART
		printf("Command is : %s\n", AudioResStr[i32ID]);
		printf("%s\n", AudioOptStr[i32ID]);
#endif		
	}
#if USEDUMYCMD
	else
	{
		SendCMDNULL();
	}
#endif	
	return;
}

