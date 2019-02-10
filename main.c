/**************************************************************************//**
 * @file        main.c
 * @version     V1.00
 * $Revision:   1$
 * $Date:       17/05/09 5:00p$
 * @brief       NUC505 Series USB Audio Class Sample Code mod by rush
 *
 * @note        The main() function cannot be debugged until C startup has completed.
 *              It is because C startup will be responsible for copying main() from ROM to RAM.
 *              So don't check the Run to main debug option or set BP before C startup is completed.
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include "NUC505Series.h"
#include "Hardware.h"
#include "USBLib.h"
#include "usbd_audio_play.h"

int main(void)
{
    /* Init Hardware */
    Hardware_Init();
	
	GPIO_SetMode(PC, BIT3, GPIO_MODE_OUTPUT);//板载绿色led灯
    
    /* Start AudioLib */
	EVAL_AUDIO_Init();
    
    /* Start USBLib */
    USBLib_Start();
    
    while ( 1 )
    {
        
        USBLib_Process();
    }
    //return 0;
}
