#include <stdio.h>
#include "VROneCmdApp/VROneCmdApp.h"

/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function									                                           			                         */
/*---------------------------------------------------------------------------------------------------------*/
UINT32 VROneCmdApp_UnpackBin(S_VRONECMD_APP *psVRApp, UINT32 u32BinStartAddr)
{
	// $$----------------------------------------------------------------$$
	//  VR model bin file information
	// --------------------------------------------------------------------
	// {   4-byte int  }
	// |     n+1       | --> Number of bin files in this pack 
	// |     Size(0)   | --> Size of CYBase.mod in bytes
	// |     Size(1)   | --> Size of group model 1 in bytes
	// |   .......     |
	// |     Size(n)   | --> Size of group model n in bytes
	// |  CYBase.mod   | --> Content of CYBase.mod of Size(0) bytes, round up to 4-byte boundary
	// |  	 model 1   | --> Content of group model 1 of Size(1), round up to 4-byte boundary
	// |   .......     |
	// |  	 model n   | --> Content of group model n of Size(n), round up to 4-byte boundary
	UINT32 u32i;
	UINT32 *pu32Addr = (UINT32 *)u32BinStartAddr;
	
	UINT32 u32ModelNum = *pu32Addr;
	
	if (u32ModelNum > VRONECMDAPP_MAX_CMD_GROUPS)
		return 0;
	
	// Content of CYBase.mod
	psVRApp->au32VRModel[0] = (UINT32)(pu32Addr+1+u32ModelNum);
	
	pu32Addr++;
	
	for (u32i = 1; u32i<u32ModelNum; u32i++)
		psVRApp->au32VRModel[u32i] = psVRApp->au32VRModel[u32i-1] + *pu32Addr++;
	
	return u32i;
}

INT32 VROneCmdApp_Initiate(S_VRONECMD_APP *psVRApp)
{
	int32_t nErr, i32Size;
	
	i32Size = CSpotter_GetMemoryUsage_Sep((UINT8 *)psVRApp->au32VRModel[0], (UINT8 *)psVRApp->au32VRModel[1], VRONECMDAPP_MAX_CMD_TIME);
	
	if (i32Size > VRONECMDAPP_WORK_BUF_SIZE)
		return -1;
	
	psVRApp->pVRHandle = CSpotter_Init_Sep((UINT8 *)psVRApp->au32VRModel[0],
								  (UINT8 *)psVRApp->au32VRModel[1], 
	                              VRONECMDAPP_MAX_CMD_TIME,
                                  (UINT8 *)psVRApp->au32VRWorkBuf,
  	                              VRONECMDAPP_WORK_BUF_SIZE, 
	                              (UINT8 *)psVRApp->au32VRStateBuf, VRONECMDAPP_STATE_BUF_SIZE, &nErr);
	if (psVRApp->pVRHandle)
		CSpotter_Reset(psVRApp->pVRHandle);
	
	return (INT32)psVRApp->pVRHandle;
}

INT32 VROneCmdApp_GetCMDID(S_VRONECMD_APP *psVRApp, INT16 *pi16PcmBuf, UINT16 u16SampleNum)
{
	INT32	i32ID = 0xff;
	
	if (u16SampleNum != VRONECMDAPP_IN_SAMPLES_PER_FRAME)
		return 0xff;
	
	if (CSpotter_AddSample(psVRApp->pVRHandle, pi16PcmBuf, VRONECMDAPP_IN_SAMPLES_PER_FRAME) == CSPOTTER_SUCCESS)
	{
		i32ID = CSpotter_GetResult(psVRApp->pVRHandle);
		//i32ID += 1;
	}
	
	return i32ID;
}

