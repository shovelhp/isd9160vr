#include "BufCtrl.h"
#include "string.h"

void BufCtrl_ReadWithCount(S_BUF_CTRL *psSrc, UINT16 u16ReadCount, INT16 *pi16Des)
{
	BufCtrl_UpdateReadDataCount(psSrc, u16ReadCount);
	while ( u16ReadCount )
	{
		u16ReadCount --;
		*pi16Des ++ = psSrc->pi16Buf[psSrc->u16BufReadIdx ++];
		if (psSrc->u16BufReadIdx>=psSrc->u16BufCount)
			psSrc->u16BufReadIdx = 0;		
	}
}

void BufCtrl_WriteWithCount(S_BUF_CTRL *psDes, UINT16 u16WriteCount, INT16 *pi16Src)
{
	BufCtrl_UpdateWriteDataCount(psDes, u16WriteCount);
	
	while ( u16WriteCount )
	{
		u16WriteCount --;
		psDes->pi16Buf[psDes->u16BufWriteIdx ++] = *pi16Src ++;
		if (psDes->u16BufWriteIdx >= psDes->u16BufCount)
			psDes->u16BufWriteIdx = 0;		
	}
}

void 
BufCtrl_SetCallback(
	S_BUF_CTRL_CALLBACK *psBufCtrl,
	PFN_DATA_REQUEST_CALLBACK pfnFunc,
	void* pParamOfFunc
)
{
	psBufCtrl->pfnFunc = pfnFunc;
	psBufCtrl->pu8Param = pParamOfFunc;
	psBufCtrl->u8Flag = S_BUF_CTRL_FLAG_CALLBACK;
	BUF_CTRL_SET_ACTIVE(psBufCtrl);
}

void
BufCtrl_SetBuf(
	S_BUF_CTRL* psBufCtrl,
	UINT16 u16BufCount,
	INT16* pi16Buf,
	UINT16 u16WriteStartIndex,
	UINT16 u16FrameSize,
	UINT16 u16SampleRate
)
{
	psBufCtrl->u16BufCount = u16BufCount;
	psBufCtrl->pi16Buf = pi16Buf;
	psBufCtrl->u16ReSamplingCalculation = psBufCtrl->u16BufReadIdx = 0;
	psBufCtrl->i16BufDataCount = psBufCtrl->u16BufWriteIdx = u16WriteStartIndex;
	psBufCtrl->u16FrameSize = u16FrameSize;
	psBufCtrl->u16SampleRate = u16SampleRate;
	psBufCtrl->u8Flag = S_BUF_CTRL_FLAG_BUF;
	BUF_CTRL_SET_ACTIVE(psBufCtrl);
	memset( psBufCtrl->pi16Buf, 0,  psBufCtrl->u16BufCount*sizeof(UINT16));
}

void BufCtrl_UpdateReadWithCount(
	S_BUF_CTRL* psBufCtrl, UINT16 u16Count
)
{
	__disable_irq();
	psBufCtrl->i16BufDataCount -= (INT16)u16Count;
	if ((psBufCtrl->u16BufReadIdx+=u16Count) >= psBufCtrl->u16BufCount)
		psBufCtrl->u16BufReadIdx = 0;

#if EANBLE_CHECK_BUFCTRL
	if ((psBufCtrl->u16BufReadIdx > psBufCtrl->u16BufCount) || (psBufCtrl->i16BufDataCount<0))
		while(1);
#endif
	__enable_irq();

}

void BufCtrl_UpdateWriteWithCount(
	S_BUF_CTRL* psBufCtrl, UINT16 u16Count
)
{
	__disable_irq();
	psBufCtrl->i16BufDataCount += (INT16)u16Count;
	if ((psBufCtrl->u16BufWriteIdx+=u16Count) >= psBufCtrl->u16BufCount)
		psBufCtrl->u16BufWriteIdx = 0;

#if EANBLE_CHECK_BUFCTRL
	if ((psBufCtrl->u16BufWriteIdx > psBufCtrl->u16BufCount) || (psBufCtrl->i16BufDataCount>psBufCtrl->u16BufCount))
		while(1);
#endif
	__enable_irq();
}


