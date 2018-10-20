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
	
	// Initiate the audio playback.
	Playback_Initiate();
	
	// Initiate the buffer control from MIC.
	MicGetApp_Initiate(&g_sApp.Mic_MD4.sMicGetApp);
	
	#if ( ULTRAIO_FW_CURVE_ENABLE )
	NVIC_SetPriority(ULTRAIO_TMR_IRQ, 0);
	#endif	
	
	u8TotalAudioNum= AudioRom_GetAudioNum( SPIFlash_ReadDataCallback, 0 );

	g_sApp.u32AudioStartAddr=0;
		

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
	
	MicGetApp_StopRec(&g_sApp.Mic_MD4.sMicGetApp);	//2017-9-5 hwh  stop mic
	
	MD4App_DecodeInitiate(&g_sApp.Mic_MD4.asMD4App, NULL, 0);
	
	if ( MD4App_DecodeStartPlayByID(&g_sApp.Mic_MD4.asMD4App, u32PlayID, g_sApp.u32AudioStartAddr, 0) == FALSE )
		return FALSE;
	
	// Start Ultraio Timer & HW pwm for UltraIO curve output
	ULTRAIO_START();
	
	// Start to playback audio. 
	Playback_StartPlay();
	
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
	// Stop speaker.
	Playback_StopPlay();
	
	MD4App_DecodeStopPlay(&g_sApp.Mic_MD4.asMD4App);
	
	// Stop Ultraio Timer & HW pwm.
	ULTRAIO_STOP();
	
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
//	static BOOL VR_flag = FALSE;
	
	// Play response
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
		
			if(i32ID==0)	//唤醒词
			{
				vr_time=10000;
				App_StartPlay(0);	//播放音乐
				printf("\n");
				printf("wake up!\n");
			}
			else if(vr_time)	//在唤醒计时内
			{
				vr_time=10000;
				switch(i32ID)
				{
					case 1:	//请开灯
						pwm0.Breath_light =0;
						pwm0.period =200;
						pwm0.Duty =50;
						PWM_Start(PWM0, 0x1);
						printf("Fan on!\n");
						App_StartPlay(1);
					break;
					case 2:	//请关灯
						pwm0.Breath_light =0;
						pwm0.Duty = pwm0.period;
						PWM_Start(PWM0, 0x0);
						printf("Fan off!\n");
						App_StartPlay(2);
					break;
					case 3:	//呼吸灯
						PWM_Start(PWM0, 0x1);
						pwm0.Breath_light =1;
						printf("Wave start!\n");
						App_StartPlay(3);
					break;
					case 4:	//调亮一点
						pwm0.Breath_light =0;
						pwm0.Duty = pwm0.period;
						PWM_Start(PWM0, 0x0);
						printf("Wave stop!\n");
						App_StartPlay(4);
					break;
					case 9:	//调亮一点
						pwm0.Breath_light =0;
						pwm0.Duty /= 10;
						pwm0.Duty *= 10;
						if(pwm0.Duty)	//已调亮
						{
							PWM_Start(PWM0, 0x1);
							pwm0.Duty -=10;
							if(pwm0.Duty < 140)	pwm0.Duty =0;
							App_StartPlay(9);
						}
						else App_StartPlay(9); //已最亮
						printf("Timer ++!\n");
					break;
					case 10:	//调暗一点
						pwm0.Breath_light =0;
						pwm0.Duty /= 10;
						pwm0.Duty *= 10;
						if(pwm0.Duty < pwm0.period)	//已调暗
						{
							PWM_Start(PWM0, 0x1);
							pwm0.Duty +=10;
							if(pwm0.Duty <140)	pwm0.Duty=140;
							App_StartPlay(10);
						}
						else App_StartPlay(10); //已最暗
						printf("Timer --!\n");
					break;
					default:
						App_StartPlay(i32ID);
						printf("%s\n", AudioResStr[i32ID]);
					//break;
				}
				
				//if(i32ID < 11)	 App_StartPlay(i32ID+1);	//播放
				
				//uart_send(vr_uart_send(i32ID+32), 4);
			}
			else return;

			
			
			
		}
	}

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
