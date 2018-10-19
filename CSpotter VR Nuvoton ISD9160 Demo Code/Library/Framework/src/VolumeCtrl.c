/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2010 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <string.h>
#include "VolumeCtrl.h"

PLAYBACK_DECLARE_VOLUME_DB_TABLE()
PLAYBACK_DECLARE_VOLUME_INFO()

void VolumeCtrl_Reset(S_VOLUMECTRL_INFO *psVolumeInfo)
{
	memset(psVolumeInfo, 0, sizeof(S_VOLUMECTRL_INFO));
	psVolumeInfo->u16CurrentRatio = VOLUMECTRL_0DB_RATIO;
	psVolumeInfo->u8TargetdBIdx = eVOLUMETRL_VOLUME_0_DB;
}

void VolumeCtrl_SetVolumeDB(S_VOLUMECTRL_INFO *psVolumeInfo, E_VOLUMETRL_VOLUME_DB eTargetDb, UINT16 u16FadeSpeed)
{

#if (VOLUMECTRL_FADE_ENABLE)	
	if (u16FadeSpeed)
		psVolumeInfo->i16RatioStep = (g_u16Playback_Volume_DB_Table[eTargetDb] - psVolumeInfo->u16CurrentRatio)/u16FadeSpeed;
	psVolumeInfo->u16FadeCount =  u16FadeSpeed;
#else
	psVolumeInfo->i16CurrentRatio = g_u32Playback_Volume_DB_Table[eTargetDb];
#endif
	psVolumeInfo->u8TargetdBIdx = eTargetDb;
}

void VolumeCtrl_VolumeFade(S_VOLUMECTRL_INFO *psVolumeInfo)
{
#if (VOLUMECTRL_FADE_ENABLE)
	if (psVolumeInfo->u16FadeCount)
	{
		psVolumeInfo->u16CurrentRatio+=psVolumeInfo->i16RatioStep;	
		psVolumeInfo->u16FadeCount--;		
	}
	else
		psVolumeInfo->u16CurrentRatio = g_u16Playback_Volume_DB_Table[psVolumeInfo->u8TargetdBIdx];
#endif
}

void VolumeCtrl_Process(S_VOLUMECTRL_INFO *psVolumeInfo, int16_t* pi16Data)
{
	VolumeCtrl_VolumeFade(psVolumeInfo);
	*pi16Data = VOLUMECTRL_CALCULATE_VOL((int32_t)*pi16Data, psVolumeInfo->u16CurrentRatio);
}

