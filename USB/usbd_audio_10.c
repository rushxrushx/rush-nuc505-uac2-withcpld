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

u32 overrun_counter=0;
u16 rx_bytes_count;
u16 rx_frames_count;

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
