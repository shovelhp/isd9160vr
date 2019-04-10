/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/	 

// ---------------------------------------------------------------------------------------------------------
//	Functions:
//		- APU ISR to handle:
//			* Get PCM data from:
//				a. Ring buffer.
//				b. Callback for special usage(example: call audio mixer function).
//			* Up sampling(1x, 2x, 4x) to reduce "metal sound".
//			* Clipping at PCM value > 4095 or < -4096.
//			* Ramp down(if force to end playback).
//		- Playback start/stop.
//
//	Reference "Readme.txt" for more information.
// ---------------------------------------------------------------------------------------------------------

#include "App.h"
#include "SPIFlash.h"
#if ((APU_FILTER_ENABLE == 1))
#include "NuDACFilterEx.h"
#endif
#include "MicSpk.h"
#include <string.h>

#define PLAYBACK_NOACTION   (0)
#define PLAYBACK_START		(1)
#define PLAYBACK_RAMP		(2)

extern INT16 g_ai16DACSamples[];

extern uint16_t const g_u16Playback_Volume_DB_Table[];

extern S_VOLUMECTRL_INFO g_sChannelVolInfo[];

volatile static UINT8	s_u8PlayCtrl = PLAYBACK_NOACTION;	

S_BUF_CTRL *g_psDacBufCtrl;

INT16 *g_pi16PcmBuf = 0;

extern volatile UINT8 g_u8AppCtrl;

#if ((APU_FILTER_ENABLE == 1)&&(APU_UPSAMPLE == 2))
#define DAC_BUF_CONSUME_SAMPLES_COUNT	2
		
__align(2) UINT8  g_au8Up2WorkBuf[NUDACFILTEREX_UP2_WORK_BUF_SIZE];
#elif ((APU_FILTER_ENABLE == 1)&&(APU_UPSAMPLE == 4))
#define DAC_BUF_CONSUME_SAMPLES_COUNT	1
		
__align(2) UINT8  g_au8Up4WorkBuf[NUDACFILTEREX_UP4_WORK_BUF_SIZE];	
#else
#define DAC_BUF_CONSUME_SAMPLES_COUNT	4
#endif

#if ( PLAYBACK_CHANNEL_COUNT > 1)
#include "AudioMixer.h"
S_BUF_CTRL * g_sMixerInBufCtrlList[PLAYBACK_CHANNEL_COUNT+1];
#else
S_BUF_CTRL *g_psDacBufCtrl;
#endif

#define DPWM_FLAUSH_CNT			(2)
static UINT8 s_u8FlaushCnt = DPWM_FLAUSH_CNT;

// ---------------------------------------------------------------------------------------------------------
#if (PLAYBACK_CHANNEL_COUNT > 1)
PINT16 Playback_Process(PINT16 pi16Buff)
{
	//UINT8 u8Count = 0;
	
	if (s_u8PlayCtrl)
	{
		if (( s_u8PlayCtrl&PLAYBACK_RAMP) == 0)
		{	
			g_pi16PcmBuf = pi16Buff;

			AudioMixer_MixProcess(g_sMixerInBufCtrlList, DAC_BUF_CONSUME_SAMPLES_COUNT, &g_pi16PcmBuf[4]);
			#if ((APU_FILTER_ENABLE == 1)&&(APU_UPSAMPLE == 2))
			// Up sampling 2x to reduce "metal sound".
			NuDACFilterEx_Up2Process(g_au8Up2WorkBuf,g_pi16PcmBuf[4], g_pi16PcmBuf);
			NuDACFilterEx_Up2Process(g_au8Up2WorkBuf,g_pi16PcmBuf[5], g_pi16PcmBuf+2);
			#elif ((APU_FILTER_ENABLE == 1)&&(APU_UPSAMPLE == 4))
			// Up sampling 4x to reduce "metal sound".
			NuDACFilterEx_Up4Process(g_au8Up4WorkBuf,g_pi16PcmBuf[4], g_pi16PcmBuf);
			#else
			g_pi16PcmBuf = &g_pi16PcmBuf[4];
			#endif
		}
		else
		{	
			if (g_pi16PcmBuf[3] == 0)  
			{
				if (s_u8FlaushCnt == 0)
				{
					#if (APU_PDMA_ENABLE)
					PdmaCtrl_Stop(APU_PDMA_CH);
					#endif
					s_u8PlayCtrl &= ~PLAYBACK_RAMP;
					s_u8FlaushCnt = DPWM_FLAUSH_CNT;
					return g_pi16PcmBuf;
				}
				s_u8FlaushCnt--;
			}else
			{
				if (g_pi16PcmBuf[3]>0)
					g_pi16PcmBuf[3] -= APU_RAMP_STEP;
				else
					g_pi16PcmBuf[3] += APU_RAMP_STEP;
				if ((g_pi16PcmBuf[3]<=APU_RAMP_STEP) && (g_pi16PcmBuf[3]>=-APU_RAMP_STEP))
					g_pi16PcmBuf[3] = 0;
				g_pi16PcmBuf[0] = g_pi16PcmBuf[1] = g_pi16PcmBuf[2] = g_pi16PcmBuf[3];	
			
			}
		}
		
		#if (APU_PDMA_ENABLE)
		PdmaCtrl_Start(APU_PDMA_CH, (UINT32 *)g_pi16PcmBuf,(uint32_t *)&DPWM->DATA, 4);
		#endif
		
		return g_pi16PcmBuf;
	}else
		return 0;	
}
#else
PINT16 Playback_Process(PINT16 pi16Buff)
{
	
	if (s_u8PlayCtrl)
	{
		if (( s_u8PlayCtrl&PLAYBACK_RAMP) == 0) // check ramp 
		{
			if ((g_psDacBufCtrl) && (g_psDacBufCtrl->i16BufDataCount>0))// need to check channel whether add and decoded
			{	
				if (! BUF_CTRL_IS_PAUSE(g_psDacBufCtrl))
					BufCtrl_UpdateReadWithCount(g_psDacBufCtrl, DAC_BUF_CONSUME_SAMPLES_COUNT);
			
				g_pi16PcmBuf = &g_psDacBufCtrl->pi16Buf[g_psDacBufCtrl->u16BufReadIdx];
			
			
				#if ((APU_FILTER_ENABLE == 1)&&(APU_UPSAMPLE == 2))
				// Up sampling 2x to reduce "metal sound".
				NuDACFilterEx_Up2Process(g_au8Up2WorkBuf,*g_pi16PcmBuf, &pi16Buff[0]);
				NuDACFilterEx_Up2Process(g_au8Up2WorkBuf,*(g_pi16PcmBuf+1), &pi16Buff[2]);
				g_pi16PcmBuf = pi16Buff;
				#elif ((APU_FILTER_ENABLE == 1)&&(APU_UPSAMPLE == 4))
				// Up sampling 4x to reduce "metal sound".
				NuDACFilterEx_Up4Process(g_au8Up4WorkBuf,*g_pi16PcmBuf, &pi16Buff[0]);
				g_pi16PcmBuf = pi16Buff;
				#endif
			
				#if (PLAYBACK_VOLUME_CONTROL)
				{
					UINT8 u8Count = 0;
					VolumeCtrl_VolumeFade(&g_sChannelVolInfo[0]);
					do
					{
						g_pi16PcmBuf[u8Count] = VOLUMECTRL_CALCULATE_VOL(((INT32)g_pi16PcmBuf[u8Count]),g_sChannelVolInfo[0].u16CurrentRatio);
					}while(++u8Count<4);		
				}
				#endif
			}else
				g_pi16PcmBuf = pi16Buff;
		}else
		{
			if (g_pi16PcmBuf[3] == 0)  
			{
				if (s_u8FlaushCnt == 0)
				{
					#if (APU_PDMA_ENABLE)
					PdmaCtrl_Stop(APU_PDMA_CH);
					#endif
					s_u8PlayCtrl &= ~PLAYBACK_RAMP;
					s_u8FlaushCnt = DPWM_FLAUSH_CNT;
					return g_pi16PcmBuf;
				}
				s_u8FlaushCnt--;
			}else
			{
				if (g_pi16PcmBuf[3]>0)
					g_pi16PcmBuf[3] -= APU_RAMP_STEP;
				else
					g_pi16PcmBuf[3] += APU_RAMP_STEP;
				if ((g_pi16PcmBuf[3]<=APU_RAMP_STEP) && (g_pi16PcmBuf[3]>=-APU_RAMP_STEP))
					g_pi16PcmBuf[3] = 0;
				g_pi16PcmBuf[0] = g_pi16PcmBuf[1] = g_pi16PcmBuf[2] = g_pi16PcmBuf[3];	
			
			}
		}
		
		#if (APU_PDMA_ENABLE)
		PdmaCtrl_Start(APU_PDMA_CH, (UINT32 *)g_pi16PcmBuf,(uint32_t *)&DPWM->DATA, 4);
		#endif
		
		return g_pi16PcmBuf;
	}else
		return 0;	
}
#endif

static void Playback_SetPauseMute(uint8_t u8Channel, BOOL bEnable, uint8_t u8Option)
{
	S_BUF_CTRL *psBufCtrl;

	if ( u8Channel >= PLAYBACK_CHANNEL_COUNT )
		return;
	
	#if ( PLAYBACK_CHANNEL_COUNT > 1)
	psBufCtrl = g_sMixerInBufCtrlList[u8Channel];
	#else
	psBufCtrl = g_psDacBufCtrl;
	#endif
	
	if (psBufCtrl)
	{
		#if (APU_PDMA_ENABLE)
		PDMA_DisableInt(APU_PDMA_CH, PDMA_INTENCH_TXOKIEN_Msk);
		#endif
		if ( bEnable)
		{
			#if (PLAYBACK_VOLUME_CONTROL)
			if (((psBufCtrl->u8Flag & (S_BUF_CTRL_FLAG_PAUSE|S_BUF_CTRL_FLAG_MUTE)) == 0))
			{
				g_sChannelVolInfo[u8Channel].i8KeepOldVol = g_sChannelVolInfo[u8Channel].u8TargetdBIdx;
				VolumeCtrl_SetVolumeDB(&g_sChannelVolInfo[u8Channel], eVOLUMETRL_VOLUME_MUTE, 2);
			}
			#endif
			
			if (u8Option)
				BUF_CTRL_SET_PAUSE(psBufCtrl);
			else
				BUF_CTRL_SET_MUTE(psBufCtrl);
		}
		else
		{
			if (u8Option)
				BUF_CTRL_SET_RESUME(psBufCtrl);
			else
				BUF_CTRL_SET_UNMUTE(psBufCtrl);
			
			#if (PLAYBACK_VOLUME_CONTROL)
			if (((psBufCtrl->u8Flag & (S_BUF_CTRL_FLAG_PAUSE|S_BUF_CTRL_FLAG_MUTE)) == 0))
				VolumeCtrl_SetVolumeDB(&g_sChannelVolInfo[u8Channel], (E_VOLUMETRL_VOLUME_DB)g_sChannelVolInfo[u8Channel].i8KeepOldVol, 2);
			#endif
		}
		#if (APU_PDMA_ENABLE)
		PDMA_EnableInt(APU_PDMA_CH, PDMA_INTENCH_TXOKIEN_Msk);
		#endif
	}
}

//---------------------------------------------------------------------------------------------------------
void Playback_ResetChannelVolume(uint8_t u8Channel)
{
#if (PLAYBACK_VOLUME_CONTROL)
	if (u8Channel>=PLAYBACK_CHANNEL_COUNT)
		return;
	VolumeCtrl_Reset(&g_sChannelVolInfo[u8Channel]);
#endif
}

//---------------------------------------------------------------------------------------------------------
BOOL Playback_SetVolumeDB(uint8_t u8Channel,E_VOLUMETRL_VOLUME_DB eDBValue)
{
#if (PLAYBACK_VOLUME_CONTROL)
	if (u8Channel>=PLAYBACK_CHANNEL_COUNT)
		return FALSE;
	
	VolumeCtrl_SetVolumeDB(&g_sChannelVolInfo[u8Channel], eDBValue, 40);
	return TRUE;
#else
	return FALSE;
#endif
}

// ---------------------------------------------------------------------------------------------------------
void Playback_StartPlay(void)
{
	INT16 *pi16PcmBuf;
	
	if( s_u8PlayCtrl == PLAYBACK_NOACTION )
	{
		#if ( PLAYBACK_CHANNEL_COUNT > 1)
		pi16PcmBuf = g_ai16DACSamples;
		#else
		pi16PcmBuf = &g_psDacBufCtrl->pi16Buf[g_psDacBufCtrl->u16BufReadIdx];
		#endif
		
		#if ((APU_FILTER_ENABLE == 1)&&(APU_UPSAMPLE == 2))
		NuDACFilterEx_Up2Initial(g_au8Up2WorkBuf);
		#elif ((APU_FILTER_ENABLE == 1)&&(APU_UPSAMPLE == 4))
		NuDACFilterEx_Up4Initial(g_au8Up4WorkBuf);
		#endif

		s_u8PlayCtrl |= PLAYBACK_START;

		#if (APU_ENABLE)
		{
			UINT8 u8Count;
			
			for( u8Count = 0; u8Count < 8; u8Count ++)
				g_ai16DACSamples[u8Count] = 0;		//Clear virtual buffer
		}
		#endif
		
		SPK_Start();
		
		#if (APU_PDMA_ENABLE)
		PdmaCtrl_Start(APU_PDMA_CH, (uint32_t *)pi16PcmBuf, (uint32_t *)&DPWM->DATA, 8);
		#endif
		
		g_u8AppCtrl |= APPCTRL_PLAY;
	}
}

// ---------------------------------------------------------------------------------------------------------
void Playback_StopPlay(void)
{
	if( s_u8PlayCtrl&PLAYBACK_START)
	{
		if (g_pi16PcmBuf)
		{
			#if (APU_PDMA_ENABLE)
			PDMA_DisableInt(APU_PDMA_CH, PDMA_INTENCH_TXOKIEN_Msk);
			#endif
			
			s_u8PlayCtrl |= PLAYBACK_RAMP;
			
			#if (APU_PDMA_ENABLE)
			PDMA_EnableInt(APU_PDMA_CH, PDMA_INTENCH_TXOKIEN_Msk);
			#endif
	
			while(s_u8PlayCtrl&PLAYBACK_RAMP);
		}
		
		s_u8PlayCtrl &= ~PLAYBACK_START;	
		SPK_Stop();
		g_u8AppCtrl &= ~APPCTRL_PLAY;
	}
}

// ---------------------------------------------------------------------------------------------------------
void
Playback_SetOutputBuf(
	S_BUF_CTRL* psOutBufCtrl,
	UINT16 u16BufSize,
	INT16* pi16Buf,
	UINT16 u16FrameSize,
	UINT16 u16SampleRate
)
{
	psOutBufCtrl->u16BufCount = u16BufSize;
	psOutBufCtrl->u16FrameSize = u16FrameSize;
	psOutBufCtrl->pi16Buf = pi16Buf;
	psOutBufCtrl->u16SampleRate = u16SampleRate;
	psOutBufCtrl->i16BufDataCount = 0;
	psOutBufCtrl->u16ReSamplingCalculation = 0;
	//psOutBufCtrl->u8Flag didn't clear as 0, keep old pause or mute state!
	psOutBufCtrl->u16BufReadIdx = psOutBufCtrl->u16BufWriteIdx = 0;
	BUF_CTRL_SET_ACTIVE( psOutBufCtrl);
	memset( psOutBufCtrl->pi16Buf, 0, psOutBufCtrl->u16BufCount*sizeof(UINT16));
}

// ---------------------------------------------------------------------------------------------------------
void
Playback_UpdateOutputBuf(
	S_BUF_CTRL* psOutBufCtrl
)
{
	if ((psOutBufCtrl->u16BufWriteIdx+=psOutBufCtrl->u16FrameSize)>=psOutBufCtrl->u16BufCount)
		psOutBufCtrl->u16BufWriteIdx = 0;
}

// ---------------------------------------------------------------------------------------------------------
BOOL
Playback_NeedUpdateOutputBuf(
	S_BUF_CTRL* psOutBufCtrl
)
{
	return ( ( psOutBufCtrl->u16BufWriteIdx > psOutBufCtrl->u16BufReadIdx ) ||
		( psOutBufCtrl->u16BufReadIdx - psOutBufCtrl->u16BufWriteIdx >= psOutBufCtrl->u16FrameSize ) );
}

// ---------------------------------------------------------------------------------------------------------
void Playback_Initiate(void)
{
#if ( PLAYBACK_CHANNEL_COUNT > 1)
	{
		int i;
		
		for(i=0; i < PLAYBACK_CHANNEL_COUNT; i++ )
		{
			g_sMixerInBufCtrlList[i] = NULL;
			Playback_ResetChannelVolume(i);
		}
		g_sMixerInBufCtrlList[PLAYBACK_CHANNEL_COUNT] = AUDIOMIXER_BUL_CTRL_END;
	}
#else
	Playback_ResetChannelVolume(0);
#endif
}

// ---------------------------------------------------------------------------------------------------------
void Playback_Add(
	UINT8 u8Channel,
	S_BUF_CTRL *psBufCtrl
)
{
	UINT32 u32SampleRate;

	if ( u8Channel >= PLAYBACK_CHANNEL_COUNT )
		return;
	
	#if (APU_PDMA_ENABLE)
	PDMA_DisableInt(APU_PDMA_CH, PDMA_INTENCH_TXOKIEN_Msk);
	#endif
	
#if ( PLAYBACK_CHANNEL_COUNT > 1)
	g_sMixerInBufCtrlList[u8Channel] = psBufCtrl;
	u32SampleRate = AudioMixer_ChangeSR(g_sMixerInBufCtrlList);
#else
	g_psDacBufCtrl = psBufCtrl;
	u32SampleRate = g_psDacBufCtrl->u16SampleRate;
#endif
		
	if( u32SampleRate == 0 )
		Playback_StopPlay();
	else
	{
		SPK_ChangeSR(0, u32SampleRate);
		#if (APU_PDMA_ENABLE)
		PDMA_EnableInt(APU_PDMA_CH, PDMA_INTENCH_TXOKIEN_Msk);
		#endif
	}
}

// ---------------------------------------------------------------------------------------------------------
void Playback_Remove(
	UINT8 u8Channel
)
{	
	UINT32 u32SampleRate;
	
	if ( u8Channel >= PLAYBACK_CHANNEL_COUNT )
		return;
	
	#if (APU_PDMA_ENABLE)
	PDMA_DisableInt(APU_PDMA_CH, PDMA_INTENCH_TXOKIEN_Msk);
	#endif
	
#if ( PLAYBACK_CHANNEL_COUNT > 1)
	g_sMixerInBufCtrlList[u8Channel] = NULL;
	u32SampleRate = AudioMixer_ChangeSR(g_sMixerInBufCtrlList);
#else
	g_psDacBufCtrl = NULL;
	u32SampleRate = 0;
#endif
	
	if( u32SampleRate == 0 )
		Playback_StopPlay();
	else
	{	
		SPK_ChangeSR(0, u32SampleRate);
		#if (APU_PDMA_ENABLE)
		PDMA_EnableInt(APU_PDMA_CH, PDMA_INTENCH_TXOKIEN_Msk);
		#endif
	}
	
}

// ---------------------------------------------------------------------------------------------------------
void Playback_PauseCtrl(uint8_t u8Channel,BOOL bEnable)
{
	Playback_SetPauseMute(u8Channel, bEnable, 1); // '1' option is pause
}

// ---------------------------------------------------------------------------------------------------------
void Playback_MuteCtrl(uint8_t u8Channel,BOOL bEnable)
{
	Playback_SetPauseMute(u8Channel, bEnable, 0); // '0' option is mute
}

// ---------------------------------------------------------------------------------------------------------
UINT32 Playback_GetPlayingChannels(void)
{
#if ( PLAYBACK_CHANNEL_COUNT > 1)
	int i;
	UINT32 u32ChannelStatus = 0;
		
	for(i = 0; i < PLAYBACK_CHANNEL_COUNT; i++ )
	{
		if ( g_sMixerInBufCtrlList[i] && BUF_CTRL_IS_ACTIVE(g_sMixerInBufCtrlList[i]) )
			u32ChannelStatus |= 1<<i;
	}
	return u32ChannelStatus;
#else
	if ( g_psDacBufCtrl && BUF_CTRL_IS_ACTIVE(g_psDacBufCtrl) )
		return 1;

	return 0;
#endif
}
