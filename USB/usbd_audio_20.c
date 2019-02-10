/******************************************************************************
 * @file     usbd_audio.c
 * @brief    NuMicro series USBD driver Sample file
 * @date     2017/04/26 09:30 a.m.
 *
 * @note
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include "NUC505Series.h"

#include "usbd_audio_play.h"
#include "usbd_audio_20.h"

uint32_t PlayRate;


void SOF_Handler(void)
{

}


/* Feedback */
void EPA_Handler(void)
{

if(alt_setting==0) return;//播放状态下才需要发送反馈，停止状态不应该发送

switch (PlaySampleRate)
{
case 44100:

	if (play_speed==0)		PlayRate = (44 << 14) + (1 << 14)/10;
	else if (play_speed>0)	PlayRate = (45 << 14) + (1 << 14)/10;
	else					PlayRate = (43 << 14) + (1 << 14)/10;
	break;
	
case 88200:

	if (play_speed==0)		PlayRate = (88 << 14) + (2 << 14)/10;
	else if (play_speed>0)	PlayRate = (89 << 14) + (2 << 14)/10;
	else					PlayRate = (87 << 14) + (2 << 14)/10;
	break;
	
case 176400:

	if (play_speed==0)		PlayRate = (176 << 14) + (4 << 14)/10;
	else if (play_speed>0)	PlayRate = (177 << 14) + (4 << 14)/10;
	else					PlayRate = (175 << 14) + (4 << 14)/10;
	break;
	
case 48000:

	if (play_speed==0)		PlayRate = (48 << 14) + (0 << 14)/10;
	else if (play_speed>0)	PlayRate = (49 << 14) + (0 << 14)/10;
	else					PlayRate = (47 << 14) + (0 << 14)/10;
	break;
	
case 96000:

	if (play_speed==0)		PlayRate = (96 << 14) + (0 << 14)/10;
	else if (play_speed>0)	PlayRate = (97 << 14) + (0 << 14)/10;
	else					PlayRate = (95 << 14) + (0 << 14)/10;
	break;
	
case 192000:

	if (play_speed==0)		PlayRate = (192 << 14) + (0 << 14)/10;
	else if (play_speed>0)	PlayRate = (191 << 14) + (0 << 14)/10;
	else					PlayRate = (193 << 14) + (0 << 14)/10;
	break;
		

default:
	return;
	
}

play_speed=0;

PlayRate =	PlayRate /2;	//make 16.16 format

    while(1) {
        if (!(USBD->DMACTL & USBD_DMACTL_DMAEN_Msk))
            break;
        
        if (!USBD_IS_ATTACHED())
            break;
    }
    USBD_SET_DMA_READ(ISO_IN_EP_NUM);
    USBD_SET_DMA_ADDR((uint32_t)&PlayRate);
    USBD_SET_DMA_LEN(4);
    g_usbd_ShortPacket = 1;
    USBD_ENABLE_DMA();

    
    USBD->EP[EPA].EPRSPCTL = USB_EP_RSPCTL_SHORTTXEN;	

}

/*
void EPE_Handler(void)
{
    
	PlayRate = (44 << 14) + (1 << 14)/10;
	PlayRate = 0x000b0666;	
	
    #if 0
    USBD->EP[EPE].EPDAT_BYTE=( PlayRate&0xff);
    USBD->EP[EPE].EPDAT_BYTE=((PlayRate>>8)&0xff);
    USBD->EP[EPE].EPDAT_BYTE=((PlayRate>>16)&0xff);
    USBD->EP[EPE].EPDAT_BYTE=((PlayRate>>24)&0xff);
    #else
    while(1) {
        if (!(USBD->DMACTL & USBD_DMACTL_DMAEN_Msk))
            break;
        
        if (!USBD_IS_ATTACHED())
            break;
    }
    USBD_SET_DMA_READ(ISO_IN_EP_NUM);
    USBD_SET_DMA_ADDR((uint32_t)&PlayRate);
    USBD_SET_DMA_LEN(4);
    g_usbd_ShortPacket = 1;
    USBD_ENABLE_DMA();
    #endif
    
    USBD->EP[EPE].EPRSPCTL = USB_EP_RSPCTL_SHORTTXEN;	
	
	
}
*/

void EPF_Handler_20(void)
{

}

#ifdef __HID20__
extern uint8_t g_u8EPCReady;

void EPC_Handler_20(void)  /* Interrupt IN handler */
{
    g_u8EPCReady = 1;
}
#endif
