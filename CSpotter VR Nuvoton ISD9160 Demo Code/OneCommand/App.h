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
#include "adc.h"

// -------------------------------------------------------------------------------------------------------------------------------
// VR Commands Model Maximum Size For Load To Data Flash
// -------------------------------------------------------------------------------------------------------------------------------
// User must enable data flash area via ICP config. This area saves VR model loaded
// from SPIFLah.
//#define VRCMD_MAX_SIZE		(72*1024) 		//Unit: byte; 
//#define VRCMD_START_ADDR	(0x0012400)		//User needs to estimate code size and fix the VR model start address in Data Flash. 

//Data Flash erasable sector is 1Kbyte.
//#define VRCMD_MAX_SIZE		(105*1024) 		//Unit: byte; 
//#define VRCMD_START_ADDR	(0x00A000)		//User needs to estimate code size and fix the VR model start address in Data Flash. 											
//Data Flash erasable sector is 1Kbyte.
#define VRCMD_MAX_SIZE		(97*1024) 		//Unit: byte; 
#define VRCMD_START_ADDR	(0x00C000)		//User needs to estimate code size and fix the VR model start address in Data Flash. 											
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

#define UART_TT		80	//uart����ÿ֮֡����ʱ������         /////modify 20170804
#define UART_DA		60                                         /////modify 20170804
typedef struct		//uart�ж��շ����ݽṹ                     /////modify 20170804
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
	uint16_t period;	//����
	uint16_t Duty;		//ռ�ձ�
	uint32_t rate;	  //Ƶ��
	uint8_t Breath_light; //������ʹ��
}pwm_type;

//PWM parameter define
#define MUL 10
#define PWM_INT 10*MUL
#define PWM_STEP 30*MUL
#define PWM_LOW 10*MUL
#define PWM_HIGH 100*MUL
#define PWM_PERIOD 200*MUL
#define PWM_DUTY_INIT 100*MUL
#define PWM_RATE 30000
#define USE_PWM0 1
#define USE_PWM1 1
//LED function define
#define FANON BIT14
#define WAVEON BIT15
// #define WINDUP BIT7
// #define WINDDOWN BIT6
// #define TIMEADD BIT5
// #define TIMESUB BIT4
// #define HIGHSPEED BIT6
// #define MIDSPEED BIT5
// #define LOWSPEED BIT4
// #define ANIONON BIT7
#define COLORLIGHT BIT13
// #define SLEEPWIND BIT11
// #define NATURALWIND BIT10
#define TIMERON BIT12
#define SCMDLINE BIT7
#define SCMDLINE1 BIT11
#define STATLINE BIT10
#define STATFANON 1
//VR active time
#define VRTIME 10000
#define VRTIMELED BIT13
#define VRTIMEON 5000

#define FAN_RUNING 1
#define FAN_CLOSED 0
//timer parameter
#define MS1P25CNT 4000
#define MSCNT 3000
#define USCNT 3
#define TIMER1FREQ 8772
#define TIME150MSINIT 120-1 //151ms
#define TIME1P25MSINIT 10  //1.25ms
#define TIME0P5MSINIT 4-1 //0.46ms
#define TIME2MSINIT 5  //2.1ms

#define CMDPRE 2
#define CMDPREBIT 6
#define CMDNUMBIT 4
#define LOW4BIT 0x0F
#define SENDTIMEOFFSET 4
#define SENDCMDTIMES 3
#define CMDGAPTIME 50
#define DEBUGTIMER 0

//Compile Parameter
#define USEFLASH 1 		//if use external flash and voice response
#define USEWAKEUP 0		//if use wakeup voice to turn on the fan
#define USEWAKEUPRES 1	//if use voice response for wakeup command
#define USEALLCMD 1		//if use all commands on list
#define USEUART 0		//if use uart to see VR result
#define USEEX32K 0		//if use external 32.768k xtal
#define USEDUMYCMD 1	//if send dumy command in 150ms
#define USESTATLINE 1   //if check FAN ON STATUS from signal line

//#define NOFLASH

#define ADC_PGA_GAIN_USE 3525  //800,2600, -1200~3525, step75
#define ADC_PGACTL_BOSST_GAIN_USE ADC_PGACTL_BOSST_GAIN_26DB //ADC_PGACTL_BOSST_GAIN_26DB��ADC_PGACTL_BOSST_GAIN_0DB
#define ADC_ALC_TARLEV_USE -750  //-750,-600, -600~-2850, step150
#define ADC_ALCCTL_NGTH_USE ADC_ALCCTL_NGTH7  //7,4, 0~7
#define ADC_MICBSEL_USE ADC_MICBSEL_17V //ADC_MICBSEL_90_VCCA, acd.h
#define USEADCALC 0

void delay1p25ms(void);
void delayms(uint32_t ms);
//void delayus(uint32_t us);
//void delay50us(void);
//void SendCMDByte(uint8_t u8CMDByte);
void SendCMD(uint8_t u8CMDforMCU, uint8_t flag_CMD);
void SendCMDByteTimer(uint8_t u8CMDByte);
void SendCMD1time(uint8_t u8CMDforMCU, uint8_t u8SendTimes);
void SendCMDNULL(void);

//#define SET_GPIO_PA_BIT_HIGH(sbit)	GPIO_SET_OUT_DATA(PA, GPIO_GET_OUT_DATA(PA) | sbit);
//#define SET_GPIO_PA_BIT_LOW(sbit)	GPIO_SET_OUT_DATA(PA, GPIO_GET_OUT_DATA(PA) & (~sbit));

/**
 * @brief       Set GPIO Port OUT Data
 *
 * @param[in]   gpio        GPIO port. It could be PA, PB.
 * @param[in]   u32Bit    	GPIO Bit to set/clear/toggle.
 *
 * @retval      None
 *
 * @details     Set the Data into specified GPIO port.
 */
#define GPIO_SET_BIT(gpio, u32Bit)   ((gpio)->DOUT |= (u32Bit))
#define GPIO_CLR_BIT(gpio, u32Bit)   ((gpio)->DOUT &= (~u32Bit))
#define GPIO_TOGGLE_BIT(gpio, u32Bit)    ((gpio)->DOUT ^= (u32Bit))
//#define GPIO_GET_IN_BIT(gpio, u32Bit)   ((gpio)->PIN &= (~u32Bit))

#endif //#ifndef _APP_H_

