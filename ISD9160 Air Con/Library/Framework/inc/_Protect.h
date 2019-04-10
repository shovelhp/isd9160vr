#ifndef _PROTECT_H_
#define _PROTECT_H_

#include "Platform.h"

static __inline
BOOL ProtectCheck(void)
{
	int OrignalIsUnLocked;
	
	OrignalIsUnLocked = SYS->REGLCTL;
	
	// unlock register
    while(SYS->REGLCTL != SYS_REGLCTL_REGLCTL_Msk) 
	{
        SYS->REGLCTL = 0x59;
        SYS->REGLCTL = 0x16;
        SYS->REGLCTL = 0x88;
    }

	if ( SYS->REGLCTL == 1 )
	{
		SYS->REGLCTL = 0;
		
		if ( SYS->REGLCTL == 0 )
		{
			if ( OrignalIsUnLocked == 1 )
			{
				while(SYS->REGLCTL != SYS_REGLCTL_REGLCTL_Msk) 
				{
					SYS->REGLCTL = 0x59;
					SYS->REGLCTL = 0x16;
					SYS->REGLCTL = 0x88;
				}
			}
			if (((SYS->PDID)>>24)==0x1d||((SYS->PDID)>>24)==0x0b)
				return TRUE;
		}
	}

	return FALSE;
}
#endif
