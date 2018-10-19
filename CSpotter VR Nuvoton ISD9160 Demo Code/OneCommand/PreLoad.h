/*---------------------------------------------------------------------------------------------------------*/
/*																										   */
/* Copyright (c) Nuvoton Technology	Corp. All rights reserved.											   */
/*																										   */
/*---------------------------------------------------------------------------------------------------------*/
#ifndef _PRELOAD_H_
#define _PRELOAD_H_	  	 

#include "Platform.h"

BOOL PreLoad_PicBin(UINT8 *u8SavedBuff, UINT32 u32SPIFlashAddr, UINT32 u32ID);

BOOL PreLoad_VRBin(UINT32 u32RomBuff, UINT32 u32RomBuffSize, UINT32 u32SPIFlashAddr, UINT32 u32ID);

#endif //_PRELOAD_H_ 

