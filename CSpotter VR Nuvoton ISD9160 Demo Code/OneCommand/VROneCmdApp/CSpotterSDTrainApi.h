
#ifndef __CSPOTTERSDTRAIN_API_H
#define __CSPOTTERSDTRAIN_API_H

#include "CSpotterSDKApi.h"

#ifdef __cplusplus
extern "C"{
#endif

// Purpose: Create a trainer
// lpbyCYBase(IN): a base model for trainer
// lpbyMemPool(OUT): memory buffer for trainer
// nMemSize(IN): memory buffer size
// pnErr(OUT): CSPOTTER_SUCCESS indicates success, else error code. It can be NULL.
// Return: a trainer handle or NULL
CSPDLL_API HANDLE CSpotterSD_Init(BYTE *lpbyCYBase, BYTE *lpbyMemPool, INT nMemSize, INT *pnErr);

// Purpose: Destroy a trainer (free resources)
// hCSpotter(IN): a handle of the trainer
// Return: Success or error code
CSPDLL_API INT CSpotterSD_Release(HANDLE hCSpotter);

CSPDLL_API INT CSpotterSD_GetMemoryUsage(BYTE *lpbyCYBase, INT bEnableSpeakerMode);

// Purpose: Reset trainer
// hCSpotter(IN): a handle of the trainer
// Return: Success or error code
CSPDLL_API INT CSpotterSD_Reset(HANDLE hCSpotter);

// Purpose: Prepare to add a new utterance for training
// hCSpotter(IN): a handle of the trainer
// nUttrID(IN): identifier, the range is [0, 2], we prefer to use 3 utterances to train a voice tag
// lpszDataBuf(IN/OUT): the pointer of data buffer, it must be a DATA FALSH pointer, the buffer is used to store the voice data
// nBufSize(IN): the size of data buffer
// Return: Success or error code
CSPDLL_API INT CSpotterSD_AddUttrStart(HANDLE hCSpotter, INT nUttrID, char *lpszDataBuf, INT nBufSize);

CSPDLL_API INT CSpotterSD_AddUttrStartForSpeaker(HANDLE hCSpotter);

// Purpose: Transfer voice samples to the trainer for training
// hCSpotter(IN): a handle of the trainer
// lpsSample(IN): the pointer of voice data buffer
// nNumSample(IN): the number of voice data (a unit is a short, we prefer to add 160 samples per call)
// Return: "CSPOTTER_ERR_NeedMoreSample" indicates call this function again, otherwise Success or error code
CSPDLL_API INT CSpotterSD_AddSample(HANDLE hCSpotter, SHORT *lpsSample, INT nNumSample);

// Purpose: Finish the adding process
// hCSpotter(IN): a handle of the trainer
// Return: Success or error code
CSPDLL_API INT CSpotterSD_AddUttrEnd(HANDLE hCSpotter);

// Purpose: Get the utterance boundary
// hCSpotter(IN): a handle of the trainer
// pnStart(OUT): starting point (samples)
// pnEnd(OUT): ending point (samples)
// Return: Success or error code
CSPDLL_API INT CSpotterSD_GetUttrEPD(HANDLE hCSpotter, INT *pnStart, INT *pnEnd);

CSPDLL_API INT CSpotterSD_DifferenceCheck(HANDLE hCSpotter, INT nDiffChkLevel);

// Purpose: Train a voice tag
// hCSpotter(IN): a handle of the trainer
// lpszModelAddr(IN/OUT): the pointer of model buffer, it must be a DATA FALSH pointer
// nBufSize(IN): the size of model buffer
// nTagID(IN): the user specific ID, in the range of [0, 32767]
// nRejLevel(IN): rejection level, in the range of [0, 100]. The voice tag will be rejected if the quality is not good enough.
// pnUsedSize(OUT): the model size
// Return: Success or error code
CSPDLL_API INT CSpotterSD_TrainWord(HANDLE hCSpotter, char *lpszModelAddr, INT nBufSize, INT nTagID, INT nRejLevel, INT *pnUsedSize);
CSPDLL_API INT CSpotterSD_TrainWordForSpeaker(HANDLE hCSpotter, char *lpszModelAddr, INT nBufSize, INT nTagID, INT *pnUsedSize);

CSPDLL_API INT CSpotterSD_DeleteWord(HANDLE hCSpotter, char *lpszModelAddr, INT nBufSize, INT nTagID, INT *pnUsedSize);

CSPDLL_API INT CSpotterSD_GetCmdID(char *lpszModelAddr, INT nTagID);
CSPDLL_API INT CSpotterSD_GetTagID(char *lpszModelAddr, INT nCmdID);

// Purpose: Check whether the added utterance exists or not
// hCSpotter(IN): a handle of the trainer
// lpszModelAddr(IN): the pointer of model buffer
// nBufSize(IN): the size of model buffer
// nSimChkLevel(IN): the similarity check level
// Return: Voice tag ID or error code
CSPDLL_API INT CSpotterSD_SimilarityCheck(HANDLE hCSpotter, char *lpszModelAddr, INT nSimChkLevel);

CSPDLL_API INT CSpotterSD_GetUttrResult(HANDLE hCSpotter, char *lpszModelAddr, INT nRejectionLevel);

#ifdef __cplusplus
}
#endif

#endif // __CSPOTTERSDTRAIN_API_H
