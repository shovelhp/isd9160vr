#include "FMCUtil.h"
#include "BufCtrl.h"
#include "string.h"

UINT32 g_u32RecStartAddr;

UINT32 g_u32RecAddr;

UINT32 g_u32RecMaxSize;

//----------------------------------------------------------------------------------------------------------
#if(__CHIP_SERIES__ == __N572F072_SERIES__ || __CHIP_SERIES__ == __N572F065_SERIES__)
BOOL FMCUtil_Init(UINT32 u32StartAddr, UINT32 u32RecMaxSize, UINT32 u32EraseTime, UINT32 u32ProgramTime)
#else
BOOL FMCUtil_Init(UINT32 u32StartAddr, UINT32 u32RecMaxSize)
#endif
{
	UINT8 u8Lock = SYS_Unlock();
	
	// Initiate FMC process.
	FMC_Open();

	#if(__CHIP_SERIES__ == __N572F072_SERIES__ || __CHIP_SERIES__ == __N572F065_SERIES__)
	{
		// Set erase time per page.
		FMC_SetPageEraseTime(u32EraseTime);
		// Set program time per page.
		FMC_SetProgramTime(u32ProgramTime);
		// Disable user config update.
		FMC_DisableCfgUpdate();
		// Record recording space start address.
		g_u32RecStartAddr = u32StartAddr;
		// Record the max size.
		g_u32RecMaxSize = u32RecMaxSize;		
	}
	#else
	{
		uint32_t   	au32Config[4];
	
		if (FMC_ReadConfig(au32Config, 4) < 0) 
			return FALSE;

		if ((!(au32Config[0] & 0x1)) && (au32Config[1] == u32StartAddr))
		{
			// Record recording space start address.
			g_u32RecStartAddr = u32StartAddr;
			// Record the max size.
			g_u32RecMaxSize = u32RecMaxSize;
			
			SYS_Lock(u8Lock);
			
			return TRUE;
		}
		FMC_EnableConfigUpdate();

		au32Config[0] &= ~0x1;
		au32Config[1] = u32StartAddr;

		if (FMC_WriteConfig(au32Config, 4) < 0)
			return FALSE;

		SYS->IPRST0 = SYS_IPRST0_CHIPRST_Msk;
	}
	#endif
	
	SYS_Lock(u8Lock);
	
    return TRUE;
}

//----------------------------------------------------------------------------------------------------------
void FMCUtil_Erase(void)
{
	UINT32 u32Addr;
	
	UINT8  u8Lock = SYS_Unlock();
	
	for (u32Addr = 0 ; u32Addr <= (g_u32RecMaxSize-FMC_FLASH_PAGE_SIZE) ; u32Addr += FMC_FLASH_PAGE_SIZE)
		FMC_Erase(g_u32RecStartAddr+u32Addr);
	
	SYS_Lock(u8Lock);
}

//----------------------------------------------------------------------------------------------------------
UINT32 FMCUtil_Read( UINT32 u32Addr, UINT8* pu8Buf, UINT32 u32ByteLen)
{
	if( u32Addr > (g_u32RecMaxSize - u32ByteLen) )
		u32ByteLen = g_u32RecMaxSize - u32Addr;
	
	u32Addr += g_u32RecStartAddr;
	
	memcpy( pu8Buf, (const void*)u32Addr, u32ByteLen);
	
	return u32ByteLen;
}

//----------------------------------------------------------------------------------------------------------
UINT32 FMCUtil_Write( UINT32 u32Addr, UINT8* pu8Buf, UINT32 u32ByteLen)
{
	UINT32* pu32Buf = (UINT32*)pu8Buf;
	
	UINT8 u8Lock = SYS_Unlock();
	
	UINT32 u32ReByteLen = 0;
	
	if( u32Addr > (g_u32RecMaxSize - u32ByteLen) )
		u32ReByteLen = g_u32RecMaxSize - u32Addr;
	else
		u32ReByteLen = u32ByteLen;
	
	u32Addr += g_u32RecStartAddr;
	
	while( u32ByteLen > 0 )
	{
		FMC_Write( u32Addr, *pu32Buf );	
		pu32Buf += 1;		
		u32ByteLen -= 4;		
		u32Addr += 4;
	}
	SYS_Lock(u8Lock);

	return u32ReByteLen;
}

//----------------------------------------------------------------------------------------------------------
void FMCUtil_StartWriteEncodeData(S_AUDIOCHUNK_HEADER *psAudioChunkHeader)
{
	// Erase FMC storage depend storage variable.
	FMCUtil_Erase( );
	// Record audio codec header.
	FMCUtil_Write( 0, (UINT8*)psAudioChunkHeader, sizeof(S_AUDIOCHUNK_HEADER) );
	// Add header length.
	g_u32RecAddr = sizeof(S_AUDIOCHUNK_HEADER);
}

//----------------------------------------------------------------------------------------------------------
void FMCUtil_EndWriteEncodeData(void)
{
	S_AUDIOCHUNK_HEADER sAudioChunkHeader;
	// Read current chunk header(for writing size)
	FMCUtil_Read( 0, (UINT8*)&sAudioChunkHeader, sizeof(S_AUDIOCHUNK_HEADER));
	// Cal record audio chunk size.
	sAudioChunkHeader.u32TotalSize =  g_u32RecAddr - sizeof(S_AUDIOCHUNK_HEADER);
	// write chunk header into APROM.
	FMCUtil_Write( 0, (UINT8*)&sAudioChunkHeader, sizeof(S_AUDIOCHUNK_HEADER));
}

//----------------------------------------------------------------------------------------------------------
BOOL FMCUtil_WriteEncodeData(S_BUF_CTRL *psEncodeBufCtrl)
{		
	if ( psEncodeBufCtrl->u16BufReadIdx != psEncodeBufCtrl->u16BufWriteIdx )
	{
		// APROM flash size is not enough
		if( g_u32RecAddr >= (g_u32RecMaxSize - psEncodeBufCtrl->u16FrameSize) )
			return FALSE;
		// Write a encode frame data into FMC storage
		FMCUtil_Write( g_u32RecAddr, ((UINT8*)(psEncodeBufCtrl->pi16Buf))+psEncodeBufCtrl->u16BufReadIdx, psEncodeBufCtrl->u16FrameSize );
		// Add current record address.
		g_u32RecAddr += psEncodeBufCtrl->u16FrameSize;
		// Add encode buffer record index.
		if( (psEncodeBufCtrl->u16BufReadIdx += psEncodeBufCtrl->u16FrameSize) >= psEncodeBufCtrl->u16BufCount )
			psEncodeBufCtrl->u16BufReadIdx = 0;
	}
	return TRUE;
}
