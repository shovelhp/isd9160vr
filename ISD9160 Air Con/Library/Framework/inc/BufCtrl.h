#ifndef _BUFCTRL_H_
#define _BUFCTRL_H_

#include "Platform.h"

// 1: enable  check buf is over flow or under flow
// 0: disable check buf is over flow or under flow
#define EANBLE_CHECK_BUFCTRL	0	

#define S_BUF_CTRL_FLAG_BUF			0
#define S_BUF_CTRL_FLAG_CALLBACK	1
#define S_BUF_CTRL_FLAG_INACTIVE	2
#define S_BUF_CTRL_FLAG_MUTE		4
#define S_BUF_CTRL_FLAG_PAUSE		8
	
#define BUF_CTRL_IS_ACTIVE(psBufCtrl)				(((psBufCtrl)->u8Flag & S_BUF_CTRL_FLAG_INACTIVE)==0)
#define BUF_CTRL_IS_INACTIVE(psBufCtrl)				(((psBufCtrl)->u8Flag & S_BUF_CTRL_FLAG_INACTIVE) != 0)	
#define BUF_CTRL_IS_PAUSE_OR_INACTIVE(psBufCtrl)	(((psBufCtrl)->u8Flag & (S_BUF_CTRL_FLAG_INACTIVE|S_BUF_CTRL_FLAG_PAUSE))!=0)
#define BUF_CTRL_IS_MUTE_OR_INACTIVE(psBufCtrl)		(((psBufCtrl)->u8Flag & (S_BUF_CTRL_FLAG_INACTIVE|S_BUF_CTRL_FLAG_MUTE))!=0)
#define BUF_CTRL_IS_PAUSE(psBufCtrl)				(((psBufCtrl)->u8Flag & S_BUF_CTRL_FLAG_PAUSE)!=0)
#define BUF_CTRL_IS_MUTE(psBufCtrl)					(((psBufCtrl)->u8Flag & S_BUF_CTRL_FLAG_MUTE)!=0)
#define BUF_CTRL_ISNOT_CALLBACK(psBufCtrl)			(((psBufCtrl)->u8Flag & S_BUF_CTRL_FLAG_CALLBACK)==0)
#define BUF_CTRL_SET_ACTIVE(psBufCtrl)				((psBufCtrl)->u8Flag &= (~S_BUF_CTRL_FLAG_INACTIVE))
#define BUF_CTRL_SET_INACTIVE(psBufCtrl)			((psBufCtrl)->u8Flag |= S_BUF_CTRL_FLAG_INACTIVE)
#define BUF_CTRL_SET_CALLBACK(psBufCtrl) 			((psBufCtrl)->u8Flag |= S_BUF_CTRL_FLAG_CALLBACK)

#define BUF_CTRL_SET_MUTE(psBufCtrl)				((psBufCtrl)->u8Flag |= S_BUF_CTRL_FLAG_MUTE)
#define BUF_CTRL_SET_UNMUTE(psBufCtrl)				((psBufCtrl)->u8Flag &= (~S_BUF_CTRL_FLAG_MUTE))
#define BUF_CTRL_SET_PAUSE(psBufCtrl)				((psBufCtrl)->u8Flag|=S_BUF_CTRL_FLAG_PAUSE)
#define BUF_CTRL_SET_RESUME(psBufCtrl)				((psBufCtrl)->u8Flag&=(~S_BUF_CTRL_FLAG_PAUSE))

#define BUF_CTRL_CAN_WRITE_BUF(psBufCtrl) \
	( (psBufCtrl)->u16BufCount >= ((psBufCtrl)->i16BufDataCount+(psBufCtrl)->u16FrameSize) )
#define BUF_CTRL_CAN_READ_BUF(psBufCtrl) \
	( (psBufCtrl)->i16BufDataCount >= (psBufCtrl)->u16FrameSize )

typedef struct sBufCtrl
{
	UINT8 u8Flag;		// should not change this "u8Flag" order, to be the same order as S_CALLBACK_CTRL
	
	UINT16 u16BufCount;					// buffer total count
	INT16  *pi16Buf;					// buffer pointer.(Ring buffer)
	UINT16 u16BufReadIdx;				// buffer read index.
	UINT16 u16BufWriteIdx;				// buffer write index.
	INT16  i16BufDataCount;				// the count of legal data stored in the buffer.
	UINT16 u16FrameSize;				// frame size that output samples by processing.
	
	UINT16 u16SampleRate;				// Playback sample rate
	UINT16 u16ReSamplingCalculation;	// Meta data for re-sampling calculation
}S_BUF_CTRL;


typedef UINT8 (*PFN_DATA_REQUEST_CALLBACK)(void *pParam, INT16 i16DataBufCount, INT16 ai16DataBuf[]);
typedef struct sCallbackCtrl
{
	UINT8 u8Flag;		// should not change this "u8Flag" order, to be the same order as S_BUF_CTRL
	
	PFN_DATA_REQUEST_CALLBACK  pfnFunc;
	UINT8 *pu8Param;
} S_BUF_CTRL_CALLBACK;


void BufCtrl_ReadWithCount(S_BUF_CTRL *psSrc, UINT16 u16ConsumeCount, INT16 *pi16Des);
void BufCtrl_WriteWithCount(S_BUF_CTRL *psDes, UINT16 u16WriteCount, INT16 *pi16Src);
void BufCtrl_SetCallback(S_BUF_CTRL_CALLBACK *psBufCtrl, PFN_DATA_REQUEST_CALLBACK pfnFunc, void* pParamOfFunc);
void BufCtrl_SetBuf(S_BUF_CTRL* psBufCtrl,UINT16 u16BufCount,INT16* pi16Buf,
	UINT16 u16WriteStartIndex, UINT16 u16FrameSize,	UINT16 u16SampleRate);
void BufCtrl_UpdateReadWithCount(S_BUF_CTRL* psOutBufCtrl, UINT16 u16Count);
void BufCtrl_UpdateWriteWithCount(S_BUF_CTRL* psOutBufCtrl, UINT16 u16Count);

__STATIC_INLINE
void BufCtrl_UpdateRead(S_BUF_CTRL* psBufCtrl)
{
	BufCtrl_UpdateReadWithCount(psBufCtrl, psBufCtrl->u16FrameSize);
}

__STATIC_INLINE
void BufCtrl_UpdateWrite(S_BUF_CTRL* psBufCtrl)
{
	BufCtrl_UpdateWriteWithCount(psBufCtrl, psBufCtrl->u16FrameSize);
}

__STATIC_INLINE
void BufCtrl_UpdateReadDataCount(S_BUF_CTRL* psBufCtrl, UINT16 u16Count)
{
	__disable_irq();
	psBufCtrl->i16BufDataCount -= (INT16)u16Count;
	
#if EANBLE_CHECK_BUFCTRL
	if ((psBufCtrl->u16BufReadIdx > psBufCtrl->u16BufCount) || (psBufCtrl->i16BufDataCount<0))
		while(1);
#endif
	__enable_irq();
}

__STATIC_INLINE
void BufCtrl_UpdateWriteDataCount(S_BUF_CTRL* psBufCtrl, UINT16 u16Count)
{
	__disable_irq();
	psBufCtrl->i16BufDataCount += (INT16)u16Count;
	
#if EANBLE_CHECK_BUFCTRL
	if ((psBufCtrl->u16BufReadIdx > psBufCtrl->u16BufCount) || (psBufCtrl->i16BufDataCount<0))
		while(1);
#endif
	__enable_irq();
}
#endif // _BUFCTRL_H_
