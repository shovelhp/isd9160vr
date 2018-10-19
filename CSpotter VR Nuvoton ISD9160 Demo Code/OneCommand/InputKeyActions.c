/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/	 

// ---------------------------------------------------------------------------------------------------------
//	Functions:
//		- Direct trigger key and matrix key falling, rising, long pressing handling functions.
//			* Direct trigger key handling functions are referenced in a lookup table "g_asTriggerKeyHandler" (in "ConfigIO.h").
//			* Matrix key handling functions are reference in a lookup talbe "g_asMatrixKeyHandler" (in "ConfigIO.h").
//			* Both lookup tables are used by keypad library to call at key action being identified.
//		- Default trigger key and matrix key handler is "Default_KeyHandler()"
//
//	Reference "Readme.txt" for more information.
// ---------------------------------------------------------------------------------------------------------
#include "App.h"
//#include "VRModelsID.h"
#include "Keypad.h"

extern volatile UINT8 g_u8AppCtrl;
extern S_APP g_sApp;

void PowerDown_KeypadHandler(UINT32 u32Param)
{
	App_PowerDown();
}

void Default_KeyHandler(UINT32 u32Param)
{
	// Within this function, key state can be checked with these keywords:
	//	TGnF: direct trigger key n is in falling state(pressing)
	//	TGnR: direct trigger key n is in rising state(releasing)
	//	TGnP: direct trigger key n is in long pressing state
	//	KEYmF: matrix key m is in falling state(pressing)
	//	KEYmR: matrix key m is in rising state(releasing)
	//	KEYmP: matrix key m is in long pressing state
	// the maxium value of n is defined by "TRIGGER_KEY_COUNT" in "ConfigIO.h"
	// the maxium value of m is defined by "MATRIX_KEY_COUNT"  in "ConfigIO.h"
	switch(u32Param)
	{
		case TG1F:
			if( g_u8AppCtrl&APPCTRL_PLAY )
				return;
			App_StopRecognize();
//			g_sApp.u8VRModelID = ISD9160C_Demo_Group1_pack;
			// Dark (PA13) led for change VR model group1 from group2.
			OUT2(1);
			if (PreLoad_VRBin((UINT32)VRCMD_START_ADDR, VRCMD_MAX_SIZE, g_sApp.u32VRModelStartAddr, g_sApp.u8VRModelID) == FALSE)
				while(1);
	
			//??VR bin ???		//Must adjust parameter with model amount			  
			if (VROneCmdApp_UnpackBin(&g_sApp.sVRapp, (uint32_t)VRCMD_START_ADDR) == 0)		   
				while(1);
			// Light (PA12) led for load VR model group1.
			OUT1(0);
			App_StartRecognize();
			break;
		case TG2F:
		if( g_u8AppCtrl&APPCTRL_PLAY )
				return;	
			App_StopRecognize();
//			g_sApp.u8VRModelID = ISD9160C_Demo_Group2_pack;
			// Dark (PA12) led for change VR model group2 from group1.
			OUT1(1);
			if (PreLoad_VRBin((UINT32)VRCMD_START_ADDR, VRCMD_MAX_SIZE, g_sApp.u32VRModelStartAddr, g_sApp.u8VRModelID) == FALSE)
				while(1);
	
			//??VR bin ???		//Must adjust parameter with model amount			  
			if (VROneCmdApp_UnpackBin(&g_sApp.sVRapp, (uint32_t)VRCMD_START_ADDR) == 0)		   
				while(1);
			// Light (PA13) led for load VR model group2.
			OUT2(0);
			App_StartRecognize();
			break;
		case TG3F:
			break;	
		case TG4P:
			PowerDown_KeypadHandler(0);	
			break;	
		case TG5F:
			break;	
		case TG6F:
			break;	
	}
}
