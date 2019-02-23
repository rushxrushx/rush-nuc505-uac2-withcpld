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

u32 overrun_counter=0;
u16 rx_bytes_count;
u16 rx_frames_count;

void SOF_Handler(void)
{

}


/* EP A : Feedback */
void EPA_Handler20(void)
{
uint32_t PlayRate;

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




/* EP B : Play */
void EPB_Handler(void)
{
    uint32_t volatile u32timeout = 0x100000;
u16 i;
u16 nextbuf;
u16 data_remain;
	
	
    while(1) {
        if (!(USBD->DMACTL & USBD_DMACTL_DMAEN_Msk))
            break;
        
        if((USBD->CEPINTSTS & USBD->CEPINTEN) & USBD_CEPINTSTS_SETUPPKIF_Msk)
            return;
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
    
    rx_bytes_count = USBD->EP[EPB].EPDATCNT & 0xffff;//实际收到了windows给的多少bytes


	if(rx_bytes_count % 8 != 0)	/// 左右声道数据量不配套？！ 
		{
         //砸了电脑！
		}

    rx_frames_count	=	rx_bytes_count / 8; //64位一帧，32位每声道 X 2声道-> 8 bytes


for (i=0; i<rx_frames_count; i++ )
{

		if (Write_ptr < i2s_BUFSIZE ) nextbuf=Write_ptr+1 ;
		else nextbuf=0;
	   
		if (nextbuf != Play_ptr ) //如果没有追尾，把这一帧数据写入到环形缓存
		{
			Write_ptr = nextbuf;
			i2s_bufL[Write_ptr] = USBD->EP[EPB].EPDAT;
			i2s_bufR[Write_ptr] = USBD->EP[EPB].EPDAT;

		}
		else //撞上了
		{
			USBD->EP[EPB].EPRSPCTL |= USBD_EPRSPCTL_FLUSH_Msk;//清楚剩余的数据
			overrun_counter++;//立OVER-RUN FLAG

			break;//吃吐了，剩下数据全部不要了
			
		}
}
      
	//计算环形数据容量
	if (Write_ptr > Play_ptr) data_remain = Write_ptr - Play_ptr;

	else data_remain = i2s_BUFSIZE - Play_ptr + Write_ptr;
	
//反馈条件
	
	if (data_remain > i2s_BUFSIZE/3*2 ) play_speed=-1;

	else if (data_remain > i2s_BUFSIZE/3*1 ) play_speed=0;

	else play_speed=1;
	
	

//如果目前是停止状态，有一半容量开启播放
	if ( (audiostatus==0) && (data_remain > i2s_BUFSIZE/2 ) ) EVAL_AUDIO_Play();

	
	
}

//EP E: demo feedback,obsolote
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


/*
#ifdef __HID20__
extern uint8_t g_u8EPCReady;

void EPC_Handler_20(void)  // Interrupt IN handler
{
    g_u8EPCReady = 1;
}
#endif
*/
