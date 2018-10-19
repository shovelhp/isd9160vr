#ifndef __VRONECMDAPP_H__
#define __VRONECMDAPP_H__

#include "Platform.h"
#include "CSpotterSDKApi.h"

#ifdef __cplusplus
extern "C"{
#endif

// -------------------------------------------------------------------------------------------------------------------------------
// Configuration: Voice recognition required resource
// -------------------------------------------------------------------------------------------------------------------------------
#define VRONECMDAPP_IN_FRAME_NUM					(2)
#define VRONECMDAPP_IN_SAMPLES_PER_FRAME			(160)
//#define VRONECMDAPP_WORK_BUF_SIZE					(0x00001da0)			//This definition is decided by CSpotter_GetMemoryUsage_xxx API
#define VRONECMDAPP_WORK_BUF_SIZE					(0x00002400)			//This definition is decided by CSpotter_GetMemoryUsage_xxx API
//#define VRONECMDAPP_WORK_BUF_SIZE					(0x000024A0)			//This definition is decided by CSpotter_GetMemoryUsage_xxx API
//#define VRONECMDAPP_WORK_BUF_SIZE					(0x00002500)			//This definition is decided by CSpotter_GetMemoryUsage_xxx API	
// -------------------------------------------------------------------------------------------------------------------------------
// Configuration: Library can process maximum time from ADC input samples 
// -------------------------------------------------------------------------------------------------------------------------------
#define VRONECMDAPP_MAX_CMD_TIME		(200)		   // 2sec, 200 frames * 160 samples/frame / 16kHz sample rate. This definition will affect VR SRAM usage

//#define VRONECMDAPP_MAX_CMD_GROUPS		(4)			   // This definition is decided by command model
#define VRONECMDAPP_MAX_CMD_GROUPS		(4)			   // This definition is decided by command model
	
#define VRONECMDAPP_SAMPLE_RATE			(16000)		   // Don't change!

#define VRONECMDAPP_STATE_BUF_SIZE		(64)		   // Don't change!
	
	
typedef struct
{
	// Work buffer for VR library to keep private data during recognition.
	// (VRAPP_WORK_BUF_SIZE+3)/4 : Force to do 4 byte alignment
	UINT32 au32VRWorkBuf[(VRONECMDAPP_WORK_BUF_SIZE+3)/4];
	
	UINT32 au32VRStateBuf[(VRONECMDAPP_STATE_BUF_SIZE+3)/4];
	
	// Save VR model address 0:CYBase, 1: group1, 2: group2, 3: group3, ....
	UINT32 au32VRModel[VRONECMDAPP_MAX_CMD_GROUPS]; 
	
	// handle of a recognizer
	void *pVRHandle;


	// Record keyword to command stage;
	UINT8 u8Stage;
	
} S_VRONECMD_APP;																

// Unpack VR model groups in an bin file
UINT32 VROneCmdApp_UnpackBin(S_VRONECMD_APP *psVRApp, UINT32 u32BinStartAddr);

//Create a recognizer for recognizing a single group of commands
INT32 VROneCmdApp_Initiate(S_VRONECMD_APP *psVRApp);

//Get the recognition result from the recognizer.
INT32 VROneCmdApp_GetCMDID(S_VRONECMD_APP *psVRApp, INT16 *pi16PcmBuf, UINT16 u16SampleNum);

//Release a recognizer
__STATIC_INLINE
void VROneCmdApp_UnInitiate(S_VRONECMD_APP *psVRApp)
{
	CSpotter_Release(psVRApp->pVRHandle);
}

#ifdef __cplusplus
}
#endif

#endif // __VRAPP_H__
