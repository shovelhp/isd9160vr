/*---------------------------------------------------------------------------------------------------------*/
/*																										   */
/* Copyright (c) Nuvoton Technology	Corp. All rights reserved.											   */
/*																										   */
/*---------------------------------------------------------------------------------------------------------*/
#include "PreLoad.h"
#include "SPIFlash.h"

extern S_SPIFLASH_HANDLER g_sSpiFlash;


	// $$----------------------------------------------------------------$$
	//  Read merged file header information
	// --------------------------------------------------------------------
	//  TOTAL BIN CNT | Reserved 4 bytes |BIN0001 ADDR  |  BIN0001 DATALEN | 
	//  BIN0002 ADDR  | BIN0002 DATALEN |  BIN0003 ADDR |  BIN0003 DATALEN | 
	//  .........
	// --------------------------------------------------------------------

BOOL PreLoad_VRBin(UINT32 u32RomBuff, UINT32 u32RomBuffSize, UINT32 u32SPIFlashAddr, UINT32 u32ID)
{
	UINT32 au32Info[2]; //au32Info[0]: Offset Address, au32Info[1]: length
	UINT32 u32BinNum, u32Data;
	UINT32 u32WriteCnt, u32WriteRemain;
	UINT8 u8LockState;
	
	SPIFlash_Read(&g_sSpiFlash, u32SPIFlashAddr, (PUINT8) &u32BinNum, 4);
		
	if (u32ID >= u32BinNum)
		return FALSE;
	
	SPIFlash_Read(&g_sSpiFlash, (u32SPIFlashAddr + 8 + (8*u32ID)), (PUINT8) &au32Info, 8);
	
	if (au32Info[1] > u32RomBuffSize)
		return FALSE;
	
	u32WriteCnt = au32Info[1]/4;
	u32WriteRemain = au32Info[1]&3;
	
	u32SPIFlashAddr = u32SPIFlashAddr + au32Info[0];
	u8LockState = SYS_Unlock();
	
	FMC_Open();
	
	while(u32WriteCnt)
	{
		
		if ((u32RomBuff&0x000003FF) == 0) // Page erase; 1k bytes unit
			FMC_Erase(u32RomBuff);
		
		SPIFlash_Read(&g_sSpiFlash, u32SPIFlashAddr, (PUINT8) &u32Data, 4);
		FMC_Write(u32RomBuff, u32Data);
		u32SPIFlashAddr+=4;
		u32RomBuff+=4;
		u32WriteCnt--;
	}
	
	if (u32WriteRemain)
	{
		u32Data = 0xffffffff;
		if ((u32RomBuff&0x000003FF) == 0) // Page erase; 1k bytes unit
			FMC_Erase(u32RomBuff);
		SPIFlash_Read(&g_sSpiFlash, u32SPIFlashAddr, (PUINT8) &u32Data, u32WriteRemain);
		FMC_Write(u32RomBuff, u32Data);
	}
	
	FMC_Close();
	SYS_Lock(u8LockState);
	
	return TRUE;
	
}

