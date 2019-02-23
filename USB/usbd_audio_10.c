/******************************************************************************
 * @file     usbd_audio.c
 * @brief    NuMicro series USBD driver Sample file
 * @date     2017/04/26 09:30 a.m.
 *
 * @note
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include "NUC505Series.h"

#include "demo.h"
#include "usbd_audio_10.h"
#include "usbd_audio_play.h"

u8 fb_buf[3];
void fb10(u32 freq)
{
u32 fbvalue, nInt,nFrac;

	// example freq:44100
	// 10.10fb=  44 << 14 + 100 << 4
	
	nInt=freq/1000;
	nFrac=(freq - nInt*1000);
		
	fbvalue=(nInt <<14) | (nFrac<<4);	
	
	fb_buf[0]=fbvalue & 0xff;
	fb_buf[1]=(fbvalue>>8) & 0xff;
	fb_buf[2]=(fbvalue>>16) & 0xff;

//{fb_buf[0]=0x66;fb_buf[1]=0x06;fb_buf[2]=0x0b;}
}

void EPA_Handler10(void)
{
if(alt_setting==0) return;//播放状态下才需要发送反馈，停止状态不应该发送

switch (PlaySampleRate)
	{
case 44100:

	if (play_speed==0)		fb10(44100);
	else if (play_speed>0)	fb10(45100);
	else					fb10(43100);
	break;
	
default:
	return;
	}
	
    while(1) {
        if (!(USBD->DMACTL & USBD_DMACTL_DMAEN_Msk))
            break;
        
        if (!USBD_IS_ATTACHED())
            break;
    }
    USBD_SET_DMA_READ(ISO_IN_EP_NUM);
    USBD_SET_DMA_ADDR((uint32_t)&fb_buf);
    USBD_SET_DMA_LEN(3);
    g_usbd_ShortPacket = 1;
    USBD_ENABLE_DMA();
    
    USBD->EP[EPA].EPRSPCTL = USB_EP_RSPCTL_SHORTTXEN;	

}


#if defined __HID20__ || defined __HID__
extern int32_t g_hid_type;
extern uint8_t g_u8EPCReady;

#ifdef __ICCARM__
#pragma data_alignment=4
           static volatile uint8_t buf[8];
#pragma data_alignment=4
           volatile uint32_t g_hid_count = 0;
#else   // __CC_ARM
__align(4) static volatile uint8_t buf[8];
__align(4) volatile uint32_t g_hid_count = 0;
#endif

void HID_UpdateKbData(void)
{
    /* executed in main loop */
    
    int32_t n;
    int32_t volatile i;
    uint32_t key = 0xF;
    static uint32_t preKey;
    n = 8;
    if(g_u8EPCReady)
    {
        if(USBD->EP[EPC].EPDATCNT & 0xFFFF)
        {
            g_hid_count++;
            g_u8EPCReady = 0;
            if(g_hid_count >2 || (((USBD->EP[EPC].EPDATCNT & 0xFFFF) % 8) != 0))
            {
                printf("HID %d %d\n",g_hid_count,USBD->EP[EPC].EPDATCNT & 0xFFFF );
                USBD->EP[EPC].EPRSPCTL |= USBD_EPCRSPCTL_FLUSH_Msk;
                g_hid_count = 0;
            }
            return;
        }
        
        key = !PC0_PIN | (!PC1_PIN << 1) | (!PC2_PIN << 1) | (!PC3_PIN << 1) | (!PC4_PIN << 1);
        
        if(key == 0)
        {
            for(i = 0; i < n; i++)
                buf[i] = 0;
            
            if(key != preKey)
            {
                preKey = key;
            }
            else
                return;
        }
        else
        {
            if(g_hid_type == HID_TYPE_MEDIAKEY)
            {
                if(preKey == key)
                    return;
                
                preKey = key;
                buf[0] = 0;
                buf[1] = 0;
                if(!PC0_PIN)
                    buf[1] |= HID_CTRL_PAUSE;
                else if(!PC1_PIN)
                    buf[1] |= HID_CTRL_NEXT;
                else if(!PC2_PIN)
                    buf[1] |= HID_CTRL_PREVIOUS;
                else if(!PC3_PIN)
                    buf[0] |= HID_CTRL_VOLUME_INC;
                else if(!PC4_PIN)
                    buf[0] |= HID_CTRL_VOLUME_DEC;
            }
            else
            {
                if(preKey == key)
                    return;
                
                preKey = key;
                if(!PC0_PIN)
                    buf[2] = 0x04;/* Key A */
                else if(!PC1_PIN)
                    buf[2] = 0x05;/* Key B */
                else if(!PC2_PIN)
                    buf[2] = 0x06;/* Key C */
                else if(!PC3_PIN)
                    buf[2] = 0x07;/* Key D */
                else if(!PC4_PIN)
                    buf[2] = 0x08;/* Key E */
                else if(!PC5_PIN)
                    buf[2] = 0x09;/* Key F */
            }
        }
    
        NVIC_DisableIRQ(USBD_IRQn);
        /* Set transfer length and trigger IN transfer */
        while (1)
        {
            if (!(USBD->DMACTL & USBD_DMACTL_DMAEN_Msk))
                break;
            
            if (!USBD_IS_ATTACHED())
                break;
        }
        if(USBD->EP[EPC].EPDATCNT & 0xFFFF)
            USBD->EP[EPC].EPRSPCTL |= USBD_EPCRSPCTL_FLUSH_Msk;
        USBD_SET_DMA_READ(HID_IN_EP_NUM);
        USBD_SET_DMA_ADDR((uint32_t)&buf[0]);
        USBD_SET_DMA_LEN(8);
        USBD_ENABLE_DMA();
        NVIC_EnableIRQ(USBD_IRQn);
        while (1)
        {
            if ((USBD->EP[EPC].EPDATCNT & 0xFFFF)>=8)
                break;
            if (!USBD_IS_ATTACHED())
                break;
        }
        
        if ((USBD->EP[EPC].EPDATCNT & 0xFFFF)>=8)
            USBD->EP[EPC].EPTXCNT = 8;
        g_u8EPCReady = 0;
        g_hid_count = 0;
        return;
    }
}

void EPC_Handler(void)  /* Interrupt IN handler */
{
    g_u8EPCReady = 1;
}
#endif
