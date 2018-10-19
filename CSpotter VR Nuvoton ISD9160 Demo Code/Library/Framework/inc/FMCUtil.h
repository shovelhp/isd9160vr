/**************************************************************************//**
 * @file     FMCUtil.c
 * @version  V1.00
 * $Revision: 2 $
 * $Date: 15/03/03 02:22p $
 * @brief    FMC utility file header.
 *
 * @note
 * Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#ifndef _FMCUTIL_H_
#define _FMCUTIL_H_

#include "Platform.h"
#include "BufCtrl.h"
#include "AudioCommon.h"

// Initiate FMC utility process.
#if(__CHIP_SERIES__ == __N572F072_SERIES__ || __CHIP_SERIES__ == __N572F065_SERIES__)
BOOL FMCUtil_Init(UINT32 u32StartAddr, UINT32 u32RecMaxSize, UINT32 u32EraseTime, UINT32 u32ProgramTime);
#else
BOOL FMCUtil_Init(UINT32 u32StartAddr, UINT32 u32RecMaxSize);
#endif
// Erase APROM storage.
void FMCUtil_Erase(void);
// Write word(4 bytes) into APROM
UINT32 FMCUtil_Write( UINT32 u32Addr, UINT8* pu8Buf, UINT32 u32ByteLen);
// Read word(4 bytes) from APROM
UINT32 FMCUtil_Read( UINT32 u32Addr, UINT8* pu8Buf, UINT32 u32ByteLen);

/* Provide to recording encode data based on NuVoice framwork */  
// Start to write encode data into APROM
void FMCUtil_StartWriteEncodeData(S_AUDIOCHUNK_HEADER *psAudioChunkHeader);
// End to write encode data.
void FMCUtil_EndWriteEncodeData(void);
// Write encode data into APROM
BOOL FMCUtil_WriteEncodeData(S_BUF_CTRL *psEncodeBufCtrl);

#endif //_FMCUTIL_H_
