/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#ifndef _APP_H_
#define _APP_H_	  	 

#include "ConfigApp.h"
#include "Framework.h"

// -------------------------------------------------------------------------------------------------------------------------------
// g_u8AppCtrl Bit Field Definitions 
// -------------------------------------------------------------------------------------------------------------------------------
#define APPCTRL_NO_ACTION			0
#define APPCTRL_PLAY				BIT0
#define APPCTRL_PLAY_STOP			BIT1
#define APPCTRL_MODE_EQUATION		BIT3
#define APPCTRL_RECORD				BIT4
#define APPCTRL_RECORD_END			BIT5
#define APPCTRL_PAUSE				BIT6
#define APPCTRL_MUTE				BIT7

// -------------------------------------------------------------------------------------------------------------------------------
// Application Related Definitions 
// -------------------------------------------------------------------------------------------------------------------------------
#include "MD4App/MD4App.h"
#include "VROneCmdApp/VROneCmdApp.h"
#include "MicGetApp/MicGetApp.h"
#include "PlaybackRecord.h"
#include "BufCtrl.h"
#include "ConfigIO.h"
#include "Preload.h"

// -------------------------------------------------------------------------------------------------------------------------------
// VR Commands Model Maximum Size For Load To Data Flash
// -------------------------------------------------------------------------------------------------------------------------------
// User must enable data flash area via ICP config. This area saves VR model loaded
// from SPIFLah.
//#define VRCMD_MAX_SIZE		(72*1024) 		//Unit: byte; 
//#define VRCMD_START_ADDR	(0x0012400)		//User needs to estimate code size and fix the VR model start address in Data Flash. 

//Data Flash erasable sector is 1Kbyte.
#define VRCMD_MAX_SIZE		(105*1024) 		//Unit: byte; 
#define VRCMD_START_ADDR	(0x00a000)		//User needs to estimate code size and fix the VR model start address in Data Flash. 											
// -------------------------------------------------------------------------------------------------------------------------------
// N575 APROM End Address
// -------------------------------------------------------------------------------------------------------------------------------
#define APROM_END_ADDR	(0x00024400)
#if ((VRCMD_START_ADDR+VRCMD_MAX_SIZE)>APROM_END_ADDR)
	#error "VR mdole size exceed APROM 145k bytes!"
#endif

typedef struct
{
	union
	{
		S_MICGETAPP sMicGetApp;
		
		S_MD4_APP asMD4App;
	
	}Mic_MD4;

//	S_MICGETAPP sMicGetApp;
	S_VRONECMD_APP sVRapp;
//	S_MD4_APP asMD4App;
	// Current audio play id.
	UINT32 u32PlayID;
	// Total audio number(load from rom header.)
	UINT32 u32TotalAudioNum;
	// Audio start address in SPIFlash
	UINT32 u32AudioStartAddr;
	// VR models start address in SPIFlash
	UINT32 u32VRModelStartAddr;
	// VR model ID.
	UINT8 u8VRModelID;
	
} S_APP;


void App_Initiate(void);

void App_Process(void);

BOOL App_StartRecognize(void);

void App_StopRecognize(void);
	
BOOL App_StartPlay(UINT32 u32PlayID);

BOOL App_StopPlay(void);

void App_PowerDown(void);

#define UART_TT		80	//uart数据每帧之间有时间间隔，         /////modify 20170804
#define UART_DA		60                                         /////modify 20170804
typedef struct		//uart中断收发数据结构                     /////modify 20170804
{
	uint16_t uart_tx_len;
	uint8_t uart_tx_data[UART_DA];
	uint16_t uart_rx_len;
	uint8_t uart_rx_data[UART_DA];
	uint16_t uart_tx_per;
	uint16_t uart_rx_time;
}uart_type;

typedef struct		//
{
	uint16_t period;	//周期
	uint16_t Duty;		//占空比
	uint32_t rate;	  //频率
	uint8_t Breath_light; //呼吸灯使能
}pwm_type;





#endif //#ifndef _APP_H_

