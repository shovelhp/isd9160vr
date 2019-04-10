/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/

#ifndef __PLAYBACKRECORD_H__
#define __PLAYBACKRECORD_H__

#include "Platform.h"
#include "BufCtrl.h"
#include "VolumeCtrl.h"

#ifdef  __cplusplus
extern "C"
{
#endif

// ---------------------------------------------------------------------------------------------------------
// Function: Playback_StartPlay
//
// Description:
//	Enable APU to play audio data
//
// Argument:
//	psDacBufCtrl:
//					the poiner o buffer control(S_BUF_CTRL*) which contain PCMs data for playback or
//					the pointer callback function(S_BUF_CTRL_CALLBACK*) which provides PCMs data for playback
//	u32SampleRate:	the playback sample rate
//
// Return:
//	TRUE:	sucessful
// ---------------------------------------------------------------------------------------------------------	
void Playback_StartPlay(void);

// ---------------------------------------------------------------------------------------------------------
// Function: Playback_StopPlay
//
// Description:
//	Stop APU to play audio data and wait for APU do stop.
//
// Argument:
//
// Return:
//
// ---------------------------------------------------------------------------------------------------------
void Playback_StopPlay(void);

// ---------------------------------------------------------------------------------------------------------
// Function: Playback_SetOutputBuf
//
// Description:
//	Set parameters to output buffer structure.
//
// Argument:
//	psOutBufCtrl: structure pointer of parameter for output buffer
//	u16BufSize: output buffer size.
//	pi16Buf: adddress pointer of output buffer.
//	u16FrameSize: frame size by processing
//	u16SampleRate: APU output sample rate
//
// Return:
//	None
// ---------------------------------------------------------------------------------------------------------
void
Playback_SetOutputBuf(
	S_BUF_CTRL* psOutBufCtrl,
	UINT16 u16BufSize,
	INT16* pi16Buf,
	UINT16 u16FrameSize,
	UINT16 u16SampleRate
);

// ---------------------------------------------------------------------------------------------------------
// Function: Playback_NeedUpdateOutputBuf
//
// Description:
//	According to free space to make decisions for output buffer updated .
//
// Argument:
//	psOutBufCtrl: structure pointer of parameter for output buffer.
//
// Return:
//	TRUE: free space  
//	FALSE: no space
// ---------------------------------------------------------------------------------------------------------
BOOL
Playback_NeedUpdateOutputBuf(
	S_BUF_CTRL* psOutBufCtrl
);

// ---------------------------------------------------------------------------------------------------------
// Function: Playback_UpdateOutputBuf
//
// Description:
//	Update write index of output buffer and avoid output buffer overflow.
//
// Argument:
//	psOutBufCtrl: structure pointer of parameter for output buffer
//
// Return:
//	None
// ---------------------------------------------------------------------------------------------------------
void
Playback_UpdateOutputBuf(
	S_BUF_CTRL* psOutBufCtrl
);

void 
Playback_Initiate(
	void
	);

void 
Playback_Add(
	UINT8 u8Channel,
	S_BUF_CTRL *psBufCtrl
);

void
Playback_Remove(
	UINT8 u8Channel
	);

// ---------------------------------------------------------------------------------------------------------
// Description:
//	Audio playback control to pause/resume one channel play.
//  (Note. This API is needed to config "PLAYBACK_VOLUME_CONTROL = 1" in "ConfigApp.h")
//
// Argument:
//	u8Channel : Control channel.
//  bEnable : TRUE:Pause channel play, FALSE:Resume channel play. 
// ---------------------------------------------------------------------------------------------------------
void Playback_PauseCtrl(uint8_t u8Channel,BOOL bEnable);

// ---------------------------------------------------------------------------------------------------------
// Description:
//	Audio playback control to mute/unmute one channel play.
//  (Note. This API is needed to config "PLAYBACK_VOLUME_CONTROL = 1" in "ConfigApp.h")
//
// Argument:
//	u8Channel : Control channel.
//  bEnable : TRUE:Mute channel play, FALSE:UnMute channel play. 
// ---------------------------------------------------------------------------------------------------------
void Playback_MuteCtrl(uint8_t u8Channel,BOOL bEnable);

// ---------------------------------------------------------------------------------------------------------
// Description:
//	1. Audio playback control to set play volume DB value.
//  2. DB value is defined in file "PlaybackRecord.h".
//  (Note. This API is needed to config "PLAYBACK_VOLUME_CONTROL = 1" in "ConfigApp.h")
//
// Argument:
//	u8Channel : Control channel.
//  u32DBValue : Config DB Value(Defined in "PlaybackRecord.h", ex. PLAYBACK_VOLUME_DB_MUTE, PLAYBACK_VOLUME_DB_NEG3)
// 
// Return:
//	TRUE : Success.
//  FALSE : DB value isn't in "g_u32AudioVolume_DB_Table" table.
// ---------------------------------------------------------------------------------------------------------
BOOL Playback_SetVolumeDB(uint8_t u8Channel,E_VOLUMETRL_VOLUME_DB eDBValue);

// ---------------------------------------------------------------------------------------------------------
// Description:
//	Audio playback control to reset one channel volume to default DB index value.
//  (Note. This API is needed to config "PLAYBACK_VOLUME_CONTROL = 1" in "ConfigApp.h")
//
// Argument:
//	u8Channel : Control channel.
// ---------------------------------------------------------------------------------------------------------
void Playback_ResetChannelVolume(uint8_t u8Channel);

void 
Record_SetInBufCallback(
	S_BUF_CTRL_CALLBACK *psAdcBufCtrl,
	PFN_DATA_REQUEST_CALLBACK pfnSetIntputData,
	void* pWorkBuf
);

void 
Record_SetInBufRecord(
	S_BUF_CTRL* psInBufCtrl,
	UINT16 u16BufSize,
	INT16* pi16Buf,
	UINT16 u16FrameSize,
	UINT16 u16SampleRate
);

void Record_Add(
	S_BUF_CTRL *psAdcBufCtrl,
	UINT32 u32SampleRate
);

void Record_StartRec(void);

void Record_StopRec(void);

#ifdef  __cplusplus
}
#endif

#endif //#ifndef __PLAYBACKRECORD_H__
