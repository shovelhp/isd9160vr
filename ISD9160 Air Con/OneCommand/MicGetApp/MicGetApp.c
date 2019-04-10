/*--------------------------------------------------------------------------------------------------*/
/*                                                                                                  */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.       	                            */
/*                                                                                                  */
/*--------------------------------------------------------------------------------------------------*/
#include "MicGetApp.h"
#include "PlaybackRecord.h"
#include <string.h>
														   
//----------------------------------------------------------------------------------------------------
// Initialize ADC application.
//----------------------------------------------------------------------------------------------------
void
MicGetApp_Initiate(
	S_MICGETAPP *psMicGetApp)
{
	// clear memoy buffer of AdcApp data structure 
	memset(psMicGetApp,0,sizeof(S_MICGETAPP));
	
	BUF_CTRL_SET_INACTIVE(&psMicGetApp->sInBufCtrl);
}

//----------------------------------------------------------------------------------------------------
// Start to run ADC applicaiton.
//----------------------------------------------------------------------------------------------------
UINT32
MicGetApp_StartRec(
	S_MICGETAPP *psMicGetApp,
	INT32 u32AdcSampleRate)
{
	// Add to record.c in framework
	Record_SetInBufRecord(&psMicGetApp->sInBufCtrl,
						  MICGETAPP_IN_BUF_SIZE,
						  psMicGetApp->i16InputBuf,
						  MICGETAPP_IN_SAMPLES_PER_FRAME,
						  u32AdcSampleRate);
	
	// Add to ADC record channel
	Record_Add(&psMicGetApp->sInBufCtrl, u32AdcSampleRate);
	
	Record_StartRec();
	return u32AdcSampleRate;
}

//----------------------------------------------------------------------------------------------------
// Stop ADC application
//----------------------------------------------------------------------------------------------------
void 
MicGetApp_StopRec(
	S_MICGETAPP *psMicGetApp	// AutoTune app data structure
	)
{
	BUF_CTRL_SET_INACTIVE(&psMicGetApp->sInBufCtrl);
	
	Record_StopRec();
}

UINT32
MicGetApp_ProcessRec(S_MICGETAPP *psMicGetApp, UINT32 **ppu32BuffAddr)
{
	if (BUF_CTRL_IS_INACTIVE(&(psMicGetApp->sInBufCtrl)))
		return 0;
	
	if (psMicGetApp->sInBufCtrl.i16BufDataCount >= MICGETAPP_IN_SAMPLES_PER_FRAME)
	{
		*ppu32BuffAddr = (UINT32 *)&psMicGetApp->sInBufCtrl.pi16Buf[psMicGetApp->sInBufCtrl.u16BufReadIdx];
		BufCtrl_UpdateReadWithCount(&psMicGetApp->sInBufCtrl, MICGETAPP_IN_SAMPLES_PER_FRAME);
		return MICGETAPP_IN_SAMPLES_PER_FRAME;
	}else
		return 0;
}
