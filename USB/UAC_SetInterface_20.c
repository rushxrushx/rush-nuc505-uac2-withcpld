#include "NUC505Series.h"

#include "usbd_audio_play.h"
#include "usbd_audio_20.h"


/**
 * @brief       Set Interface standard request
 *
 * @param[in]   u32AltInterface Interface
 *
 * @return      None
 *
 * @details     This function is used to set UAC Class relative setting
 */
void UAC_SetInterface_20(uint32_t u32AltInterface)
{
    
    if ((gUsbCmd.wIndex & 0xff) == 1)        /* Interface 1 Audio Iso OUT interface */
    { 
        if (u32AltInterface == 1)            /* Alternate 1 : 32bit play */ 	
        {	
			alt_setting=1;
			PC3_DOUT = 0;//led on
        }
        else                                 /* Alt0:stop */
        {
            //psAudioLib->m_pfnPlayStop( psAudioLib );
            //USBD->EP[EPB].EPRSPCTL |= USBD_EPRSPCTL_FLUSH_Msk;
			PC3_DOUT = 1;//led off
			alt_setting=0;
        }			
				
    }
}
