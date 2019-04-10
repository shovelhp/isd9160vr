typedef void VOID;

#ifndef HANDLE
#define HANDLE	VOID*
#endif

#ifndef __CSPOTTERSDK_API_H
#define __CSPOTTERSDK_API_H

#if defined(_WIN32)
	#ifdef CSpotterDll_EXPORTS
		#define CSPDLL_API __declspec(dllexport)
	#endif
#endif

#ifndef CSPDLL_API
#define CSPDLL_API
#endif

//#include "base_types.h"
#include "Platform.h"
#include "CSpotterSDKApi_Const.h"

#ifdef __cplusplus
extern "C"{
#endif

// Purpose: Create a recognizer
// lpbyModel(IN): The command model.
// nMaxTime(IN): The maximum buffer length in number of frames for keeping the status of commands.
// lpbyMemPool(IN/OUT): Memory buffer for the recognizer.
// nMemSize(IN): Size in bytes of the memory buffer lpbyMemPool.
// lpbyState(IN/OUT): State buffer for recognizer.
// nStateSize(IN): Size in bytes of the state buffer lpbyState.
// pnErr(OUT): CSPOTTER_SUCCESS indicates success, else error code. It can be NULL.
// Return: a recognizer handle or NULL
CSPDLL_API HANDLE CSpotter_Init(UINT8 *lpbyModel, UINT8 *lpbyMemPool, int32_t nMemSize, UINT8 *lpbyState, int32_t nStateSize, int32_t *pnErr);

// lpbyBaseModel(IN): The background model, contents of CYBase.mod.
// lppbyModel(IN): The command model.
// nMaxTime(IN): The maximum buffer length in number of frames for keeping the status of commands.
// lpbyMemPool(IN/OUT): Memory buffer for the recognizer.
// nMemSize(IN): Size in bytes of the memory buffer lpbyMemPool.
// lpbyState(IN/OUT): State buffer for recognizer.
// nStateSize(IN): Size in bytes of the state buffer lpbyState.
// pnErr(OUT): CSPOTTER_SUCCESS indicates success, else error code. It can be NULL.
// Return: a recognizer handle or NULL
CSPDLL_API HANDLE CSpotter_Init_Sep(UINT8 *lpbyBaseModel, UINT8 *lpbyModel, int32_t nMaxTime,UINT8 *lpbyMemPool, int32_t nMemSize, UINT8 *lpbyState, int32_t nStateSize, int32_t *pnErr);
CSPDLL_API HANDLE CSpotter_Init_Multi(UINT8 *lpbyBaseModel, UINT8 *lppbyModel[], int32_t nNumModel, int32_t nMaxTime, UINT8 *lpbyMemPool, int32_t nMemSize, UINT8 *lpbyState, int32_t nStateSize, int32_t *pnErr);

// Purpose: Destroy a recognizer (free resources)
// hCSpotter(IN): a handle of the recognizer
// Return: Success or error code
CSPDLL_API int32_t CSpotter_Release(HANDLE hCSpotter);

// Purpose: Reset recognizer
// hCSpotter(IN): a handle of the recognizer
// Return: Success or error code
CSPDLL_API int32_t CSpotter_Reset(HANDLE hCSpotter);

// Purpose: Transfer voice samples to the recognizer for recognizing.
// hCSpotter(IN): a handle of the recognizer
// lpsSample(IN): the pointer of voice data buffer
// nNumSample(IN): the number of voice data (a unit is a short, we prefer to add 160 samples per call)
// Return: "CSPOTTER_ERR_NeedMoreSample" indicates call this function again, else call CSpotter_GetResult(...)
CSPDLL_API int32_t CSpotter_AddSample(HANDLE hCSpotter, INT16 *lpsSample, int32_t nNumSample);

// Purpose: Get recognition results.
// hCSpotter(IN): a handle of the recognizer
// Return: the command ID. It is 0 based
CSPDLL_API int32_t CSpotter_GetResult(HANDLE hCSpotter);
CSPDLL_API int32_t CSpotter_GetResultEPD(HANDLE hCSpotter, int32_t *pnWordDura, int32_t *pnEndDelay);
CSPDLL_API int32_t CSpotter_GetResultScore(HANDLE hCSpotter);

CSPDLL_API int32_t CSpotter_GetNumWord(UINT8 *lpbyModel);

CSPDLL_API int32_t CSpotter_GetMemoryUsage(UINT8 *lpbyModel, int32_t nMaxTime);
CSPDLL_API int32_t CSpotter_GetMemoryUsage_Sep(UINT8 *lpbyBaseModel, UINT8 *lpbyModel, int32_t nMaxTime);
CSPDLL_API int32_t CSpotter_GetMemoryUsage_Multi(UINT8 *lpbyBaseModel, UINT8 *lppbyModel[], int32_t nNumModel, int32_t nMaxTime);

CSPDLL_API int32_t CSpotter_GetStateSize(UINT8 *lpbyModel);

/************************************************************************/
//  Threshold Adjust API                                                                   
/************************************************************************/
// Purpose: Set model rejection level
// hCSpotter(IN): a handle of the recognizer
// nRejectionLevel(IN): rejection level
// Return: Success or error code
CSPDLL_API int32_t CSpotter_SetRejectionLevel(HANDLE hCSpotter, int32_t nRejectionLevel);

// Purpose: Set engine response time
// hCSpotter(IN): a handle of the recognizer
// nResponseTime(IN): response time
// Return: Success or error code
CSPDLL_API int32_t CSpotter_SetResponseTime(HANDLE hCSpotter, int32_t nResponseTime);

// Purpose: Set Cmd reward
// hCSpotter(IN): a handle of the recognizer
// nCmdIdx(IN): the command ID. It is 0 based
// nReward(IN): the reward
// Return: Success or error code
CSPDLL_API int32_t CSpotter_SetCmdReward(HANDLE hCSpotter, int32_t nCmdIdx, int32_t nReward);

// Purpose: Set Cmd response reward
// hCSpotter(IN): a handle of the recognizer
// nCmdIdx(IN): the command ID. It is 0 based
// nReward(IN): the reward
// Return: Success or error code
CSPDLL_API int32_t CSpotter_SetCmdResponseReward(HANDLE hCSpotter, int32_t nCmdIdx, int32_t nReward);

CSPDLL_API int32_t CSpotter_SetEndStateRange(HANDLE hCSpotter, int32_t nEndStateRange);

CSPDLL_API int32_t CSpotter_SetFreqWarpFactor(HANDLE hCSpotter, int32_t nFactor256);

CSPDLL_API int32_t CSpotter_SetMinEnergyThreshd(HANDLE hCSpotter, int32_t nThreshold);

CSPDLL_API int32_t CSpotter_SetEnableRejectedResult(HANDLE hCSpotter, int32_t bEnable);
CSPDLL_API int32_t CSpotter_SetRejectedResultEnergyThreshd(HANDLE hCSpotter, int32_t nThreshold);

#ifdef __cplusplus
}
#endif

#endif // __CSPOTTERSDK_API_H
