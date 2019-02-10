#include "NUC505Series.h"

#include "usbd_audio_play.h"
#include "usbd_audio_20.h"

void UAC_ClassOUT_20(void)
{
    uint32_t volatile u32timeout = 0x100000;
    /* To make sure that no DMA is reading the Endpoint Buffer (4-8 & 4-5)*/
    while(1) {
        if (!(USBD->DMACTL & USBD_DMACTL_DMAEN_Msk))
            break;
        
        if (!USBD_IS_ATTACHED())
            break;

        if(u32timeout == 0)				
        {
            printf("EPA\t%x\n", USBD->EP[EPA].EPDATCNT);
            printf("EPB\t%x\n", USBD->EP[EPB].EPDATCNT);
            printf("EPC\t%x\n", USBD->EP[EPC].EPDATCNT);					
            printf("DMACTL\t%X\n", USBD->DMACTL);			
            printf("DMACNT\t%X\n", USBD->DMACNT);						
            u32timeout = 0x100000;
        }					
        else
            u32timeout--;
    }
    
    /* Host to device */
    if(CLOCK_SOURCE_ID == ((gUsbCmd.wIndex >> 8) & 0xff))
    {
        if(gUsbCmd.bRequest== FREQ_CONTROL)
        {
            USBD_CtrlOut((uint8_t *)&PlaySampleRate, gUsbCmd.wLength);//电脑发来要求的采样率，保存到了PlaySampleRate
        }
        USBD_SET_CEP_STATE(USB_CEPCTL_NAKCLR);
       
        EVAL_AUDIO_Samplerate();//更新采样率
    }
    else
        switch (gUsbCmd.bRequest)
        {
            case UAC_SET_CUR:
            {
                switch ((gUsbCmd.wValue & 0xff00) >> 8)
                {
                    case MUTE_CONTROL:
                        if (PLAY_FEATURE_UNITID == ((gUsbCmd.wIndex >> 8) & 0xff))
                        {
                            USBD_CtrlOut((uint8_t *)&m_u8PlayMute, gUsbCmd.wLength);//电脑发来静音设置，保存到了m_u8PlayMute
                        }
                        /* Status stage */
                        USBD_SET_CEP_STATE(USB_CEPCTL_NAKCLR);
                        EVAL_AUDIO_Mute();//更新静音设置
                        break;
                    
                    case VOLUME_CONTROL:
                        if (PLAY_FEATURE_UNITID == ((gUsbCmd.wIndex >> 8) & 0xff))
                        {
                            if (((gUsbCmd.wValue) & 0xff) == 1)
                            {
                                USBD_CtrlOut((uint8_t *)&m_i16PlayVolumeL, gUsbCmd.wLength);//电脑发来音量设置，保存到了m_i16PlayVolumeL
                                /* Status stage */
                                USBD_SET_CEP_STATE(USB_CEPCTL_NAKCLR);
								EVAL_AUDIO_VolumeCtl();//更新音量
                            }
                            else
                            {
                                USBD_CtrlOut((uint8_t *)&m_i16PlayVolumeR, gUsbCmd.wLength);//电脑发来音量设置，保存到了m_i16PlayVolumeR
                                /* Status stage */
                                USBD_SET_CEP_STATE(USB_CEPCTL_NAKCLR);
								EVAL_AUDIO_VolumeCtl();//更新音量
                            }
                        }
                        break;
                    default:
                        /* STALL control pipe */
                        USBD_SET_CEP_STATE(USBD_CEPCTL_STALLEN_Msk);
                        break;
                }
                break;
            }
            default:
            {
                USBD->CEPCTL = USBD_CEPCTL_FLUSH_Msk;
                /* Setup error, stall the device */
                USBD_SET_CEP_STATE(USBD_CEPCTL_STALLEN_Msk);
                break;
            }
        }
}
