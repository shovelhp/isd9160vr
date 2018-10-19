/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/

#ifndef __VOLUMECTRL_H__
#define __VOLUMECTRL_H__

#include "Platform.h"
#include "BufCtrl.h"
#include "ConfigApp.h"

#ifdef  __cplusplus
extern "C"
{
#endif

#define VOLUMECTRL_FADE_ENABLE		(1)	 // 1:enable fading effect, 0:disable fading effect	
	
// Decimal 6.10 fixed point for step dB value
// BIT0~BIT10: fraction part
// BIT11~BIT15: integer part
#define VOLUMECTRL_FIXEDPOINT_SHIFT	(10)

#define VOLUMECTRL_0DB_RATIO	(1<<VOLUMECTRL_FIXEDPOINT_SHIFT)

#define VOLUMECTRL_ROUND_OFF	((1<<VOLUMECTRL_FIXEDPOINT_SHIFT)>>1)

#define VOLUMECTRL_CALCULATE_VOL(i32Pcm, u16Ratio)		((i32Pcm*u16Ratio+VOLUMECTRL_ROUND_OFF)>>VOLUMECTRL_FIXEDPOINT_SHIFT)

#define VOLUMECTRL_VOL_FADEOUT(psVolumeInfo, eTargetDb, u16FadeSpeed)		VolumeCtrl_SetVolumeDB(psVolumeInfo, eTargetDb, u16FadeSpeed)
	
#define VOLUMECTRL_VOL_FADEIN(psVolumeInfo, eTargetDb, u16FadeSpeed)		VolumeCtrl_SetVolumeDB(psVolumeInfo, eTargetDb, u16FadeSpeed)
	
typedef enum{
	eVOLUMETRL_VOLUME_MUTE = 0,
	eVOLUMETRL_VOLUME_NEG35_DB,
	eVOLUMETRL_VOLUME_NEG30_DB,
	eVOLUMETRL_VOLUME_NEG25_DB ,
	eVOLUMETRL_VOLUME_NEG20_DB,
	eVOLUMETRL_VOLUME_NEG15_DB,
	eVOLUMETRL_VOLUME_NEG10_DB ,
	eVOLUMETRL_VOLUME_NEG6_DB ,
	eVOLUMETRL_VOLUME_NEG3_DB,
	eVOLUMETRL_VOLUME_0_DB,
	eVOLUMETRL_VOLUME_3_DB,
	eVOLUMETRL_VOLUME_6_DB,
	eVOLUMETRL_VOLUME_10_DB,
} E_VOLUMETRL_VOLUME_DB;

#define PLAYBACK_DECLARE_VOLUME_DB_TABLE()			\
uint16_t const g_u16Playback_Volume_DB_Table[] =  	\
{ 													\
	(0),							\
	(0.0178*(1<<VOLUMECTRL_FIXEDPOINT_SHIFT)),		\
	(0.0316*(1<<VOLUMECTRL_FIXEDPOINT_SHIFT)),		\
	(0.0562*(1<<VOLUMECTRL_FIXEDPOINT_SHIFT)),		\
	(0.1000*(1<<VOLUMECTRL_FIXEDPOINT_SHIFT)),		\
	(0.1778*(1<<VOLUMECTRL_FIXEDPOINT_SHIFT)),		\
	(0.3162*(1<<VOLUMECTRL_FIXEDPOINT_SHIFT)),		\
	(0.5012*(1<<VOLUMECTRL_FIXEDPOINT_SHIFT)),		\
	(0.7079*(1<<VOLUMECTRL_FIXEDPOINT_SHIFT)),		\
	VOLUMECTRL_0DB_RATIO,							\
	(1.4125*(1<<VOLUMECTRL_FIXEDPOINT_SHIFT)),		\
	(1.9953*(1<<VOLUMECTRL_FIXEDPOINT_SHIFT)),		\
	(3.1623*(1<<VOLUMECTRL_FIXEDPOINT_SHIFT))		\
};

typedef struct
{
	UINT16 u16CurrentRatio;
#if (VOLUMECTRL_FADE_ENABLE)	
	UINT16 u16FadeCount;
	INT16 i16RatioStep;
#endif
	INT8 i8KeepOldVol;
	UINT8 u8TargetdBIdx;
}S_VOLUMECTRL_INFO;

#define PLAYBACK_DECLARE_VOLUME_INFO()			\
S_VOLUMECTRL_INFO g_sChannelVolInfo[PLAYBACK_CHANNEL_COUNT];


void VolumeCtrl_Reset(S_VOLUMECTRL_INFO *psVolumeInfo);

void VolumeCtrl_VolumeFade(S_VOLUMECTRL_INFO *psVolumeInfo);

void VolumeCtrl_Process(S_VOLUMECTRL_INFO *psVolumeInfo, int16_t* pi16Data);

void VolumeCtrl_SetVolumeDB(S_VOLUMECTRL_INFO *psVolumeInfo, E_VOLUMETRL_VOLUME_DB eTargetDb, UINT16 u16FadeSpeed);

__STATIC_INLINE
E_VOLUMETRL_VOLUME_DB VolumeCtrl_GetVolumeDB(S_VOLUMECTRL_INFO *psVolumeInfo)
{
	return ((E_VOLUMETRL_VOLUME_DB)psVolumeInfo->u8TargetdBIdx);
}

#endif //#ifndef __VOLUMECTRL_H__

