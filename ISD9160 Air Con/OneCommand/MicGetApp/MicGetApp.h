#ifndef _MICGETAPP_H_
#define _MICGETAPP_H_

#include "Platform.h"
#include "ConfigApp.h"

#include "BufCtrl.h"
#include "PlaybackRecord.h"

// -------------------------------------------------------------------------------------------------------------------------------
// PCM Input Related Definitions 
// -------------------------------------------------------------------------------------------------------------------------------
#define MICGETAPP_IN_FRAME_NUM	  		2
#define MICGETAPP_IN_SAMPLES_PER_FRAME	160								
#define MICGETAPP_IN_BUF_SIZE 			(MICGETAPP_IN_FRAME_NUM*MICGETAPP_IN_SAMPLES_PER_FRAME)
 							
#if ( MICGETAPP_IN_BUF_SIZE%8 )
	#error "ADCAPP_OUT_BUF_SIZE must be multiple of '8'."	
#endif

typedef struct
{
	S_BUF_CTRL 	sInBufCtrl;							// Input buffer controller
	INT16 	i16InputBuf[MICGETAPP_IN_BUF_SIZE];		// Buffer to store recorded PCM samples from MIC
}S_MICGETAPP;


//----------------------------------------------------------------------------------------------------
// Initialize auto tune application.
//----------------------------------------------------------------------------------------------------
void
MicGetApp_Initiate(
	S_MICGETAPP *psMicGetApp);		// AutoTune app data structure

//----------------------------------------------------------------------------------------------------
// Start to run AutoTune applicaiton.
//----------------------------------------------------------------------------------------------------
UINT32							// output sample rate
MicGetApp_StartRec(
	S_MICGETAPP *psMicGetApp,	// ADC app data structure
	INT32 u32AdcSampleRate);	// ADC record sample rate

//----------------------------------------------------------------------------------------------------
// Stop AutoTune application
//----------------------------------------------------------------------------------------------------
void 
MicGetApp_StopRec(
	S_MICGETAPP *psAdcApp	// ADC app data structure
	);

UINT32
MicGetApp_ProcessRec(S_MICGETAPP *psMicGetApp, UINT32 **ppu32BuffAddr);
	
#endif
