#include "usbd_audio_play.h"
#include "i2s.h"
//#include "led.h"
//////////////////////////////////////////////////////////////////////////////////	 

//USB声卡底层接口函数 驱动代码	   
//正点原子@ALIENTEK
//rush修改
////////////////////////////////////////////////////////////////////////////////// 	   

s16 m_i16PlayVolumeL=0;
s16 m_i16PlayVolumeR=0;
s16 m_u8PlayMute=0;
int16_t  m_i16PlayMaxVolume=0x0000;        //Playing volume control max
int16_t  m_i16PlayMinVolume=0x8100;        //Playing volume control min
int16_t  m_i16PlayResVolume=0x0100;        //Playing volume control res
 
u8 audiostatus=0;							//bit0:0,暂停播放;1,继续播放 
u8 alt_setting=0;	//1:32位，0:停止播放
  
u16 Play_ptr=0;							//即将播放的音频帧缓冲编号
u16 Write_ptr=0;							//当前保存到的音频缓冲编号 

s32 play_speed=0;			//0:反馈当前采样率；正数:叠加正值(要求电脑多发采样)；负数:叠加负值(少发采样)
u32 underrun_count=0;

u32 PlaySampleRate=44100;	//当前工作采样率

u32 const i2s_BUFSIZE=2000;								
u32 i2s_bufL[i2s_BUFSIZE+1];							
u32 i2s_bufR[i2s_BUFSIZE+1];//缓存分开放，拒绝错乱
u32 i2s_workingBUFSIZE=2000;//实际工作中缓存大小	

#define LEDON ;
#define LEDOFF ;

#define CPLD_divide8 	PA1_DOUT = 1;PA2_DOUT = 1;
#define CPLD_divide4 	PA1_DOUT = 1;PA2_DOUT = 0;
#define CPLD_divide2 	PA1_DOUT = 0;PA2_DOUT = 1;
#define CPLD_divide1 	PA1_DOUT = 0;PA2_DOUT = 0;

#define SEL_45 	PA6_DOUT = 0;PA7_DOUT = 1;
#define SEL_49 	PA6_DOUT = 1;PA7_DOUT = 0;

//开始播放
void EVAL_AUDIO_Play(void)
{  
    I2S_EnableInt(I2S,I2S_FIFO_TXTH_INT_MASK);
    I2S_ENABLE_TX(I2S);
	LEDON;
	audiostatus=1;
}
 
 
//停止播放
void EVAL_AUDIO_Stop(void)
{ 
    I2S_DisableInt(I2S,I2S_FIFO_TXTH_INT_MASK);
    I2S_DISABLE_TX(I2S);
    I2S_CLR_TX_FIFO(I2S);
	LEDOFF;
	audiostatus=0;
}

///设置是采样率 i2s set samplerate
static void I2S_Set_Samplerate(uint32_t u32I2sSampleRate)
{
	
	PA0_DOUT = 0;//reset CPLD


///晶振切换
//切换22.5792m/24.576m晶振

    switch ( u32I2sSampleRate )
    {
        case 44100:
		case 88200:	
        case 176400:
		case 352800:
			SEL_45;//22或45m
            break;	
			
        case 48000:
		case 96000:
        case 192000:
		case 384000:
			SEL_49;//24或49m
            break;
        default:
			printf("what fuck of this rate:%d",u32I2sSampleRate);			
			return;
    }


    
//分频切换
//切换1/2/4/8分频，
//22/24mhz 对应192k/96k/48k/24k四类速率
//45/49mhz，四档速率变为384k/192k/96k/48k，但是注意你的DAC支持1024fs情况	

    switch ( u32I2sSampleRate )
    {
        case 44100:
		case 48000:
			CPLD_divide4;//CPLD_divide8;				
            break;		
        case 88200:
		case 96000:
			CPLD_divide2;//CPLD_divide4;			
            break;		
        case 176400:
		case 192000:
			CPLD_divide1;//CPLD_divide2;		
            break;		
        case 352800:
		case 384000:
			CPLD_divide1;
            break;
        default:
			printf("what fuck of this rate:%d",u32I2sSampleRate);			
			return;
    }
	
	PA0_DOUT = 1;//CPLD play

}

void I2S_slaveOpen1(uint32_t u32WordWidth, uint32_t u32Channels, uint32_t u32DataFormat)
{
	SYS->IPRST1 |= SYS_IPRST1_I2SRST_Msk;
	SYS->IPRST1 &= ~SYS_IPRST1_I2SRST_Msk;

	I2S->CTL = BIT22 | I2S_MODE_SLAVE | u32WordWidth | u32Channels | u32DataFormat | I2S_CTL_CODECSEL_Msk | I2S_FIFO_TX_LEVEL_WORD_8 | I2S_FIFO_RX_LEVEL_WORD_9;

	I2S->CTL &= ~I2S_CTL_CODECRST_Msk;//内部DAC关闭

    I2S->CTL |= I2S_CTL_I2SEN_Msk;

}
void EVAL_AUDIO_Samplerate( void)
{
EVAL_AUDIO_Stop();

	I2S_Set_Samplerate(PlaySampleRate);
}

void I2S2_Init(void)
{
    
    /* Init I2S, IP clock and multi-function I/O */
    /* Enable I2S Module clock */
    CLK_EnableModuleClock(I2S_MODULE);
    
    /* Reset IP */
    SYS_ResetModule(I2S_RST);
    
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Configure multi-function pins for I2S */
    SYS->GPC_MFPH = (SYS->GPC_MFPH & (~SYS_GPC_MFPH_PC8MFP_Msk) )  | SYS_GPC_MFPH_PC8MFP_I2S_MCLK;
    SYS->GPC_MFPH = (SYS->GPC_MFPH & (~SYS_GPC_MFPH_PC9MFP_Msk) )  | SYS_GPC_MFPH_PC9MFP_I2S_DIN;
    SYS->GPC_MFPH = (SYS->GPC_MFPH & (~SYS_GPC_MFPH_PC10MFP_Msk) ) | SYS_GPC_MFPH_PC10MFP_I2S_DOUT;
    SYS->GPC_MFPH = (SYS->GPC_MFPH & (~SYS_GPC_MFPH_PC11MFP_Msk) ) | SYS_GPC_MFPH_PC11MFP_I2S_LRCLK;
    SYS->GPC_MFPH = (SYS->GPC_MFPH & (~SYS_GPC_MFPH_PC12MFP_Msk) ) | SYS_GPC_MFPH_PC12MFP_I2S_BCLK;
    
	
//控制CPLD和晶振的GPIO口	
	GPIO_SetMode(PA, BIT0, GPIO_MODE_OUTPUT);
	GPIO_SetMode(PA, BIT1, GPIO_MODE_OUTPUT);
	GPIO_SetMode(PA, BIT2, GPIO_MODE_OUTPUT);


	I2S_Set_Samplerate(PlaySampleRate);		//设置采样率(初始化外部晶振状态，开机默认是两个同时通电输出短路)
    I2S_slaveOpen1(I2S_DATABIT_32, I2S_STEREO, I2S_FORMAT_I2S);
	
}

//上电初始化
void EVAL_AUDIO_Init(void)
{ 
	I2S2_Init();
	I2S_SetFIFO(I2S,8,8);
	EVAL_AUDIO_Stop();
	NVIC_EnableIRQ(I2S_IRQn);
}

//更新音量
void EVAL_AUDIO_VolumeCtl(void)
{


}

//更新静音设置
void EVAL_AUDIO_Mute(void)
{


}

//喂食i2s（中断中间调用）
void i2s_feed(void)
{

u16 next_Playptr;
u16 fifo_remain;

//获取发送FIFO中现存数据容量
	if (I2S->STATUS & I2S_STATUS_TXFULL_Msk) fifo_remain=16;
	else fifo_remain=(I2S->STATUS & I2S_STATUS_TXCNT_Msk)>>I2S_STATUS_TXCNT_Pos;

feed_start:
	//fifo缓冲区一共16words 至少剩下两个word的空间，来存放一对的左右声道
	if (fifo_remain>14) return;

	//算下一个数据帧
	if (Play_ptr < i2s_BUFSIZE ) next_Playptr = Play_ptr + 1;

	else  next_Playptr = 0;              //循环回头部

	if( next_Playptr == Write_ptr)  //todo:这里应该判断缓冲具体剩余容量
	{ 
		EVAL_AUDIO_Stop();	//撞尾了
		underrun_count++;
		return;
	}else
	{
		Play_ptr = next_Playptr;//可以开开心心去下一个数据帧
	}
	
//塞进一对数据去FIFO
	I2S->TX = i2s_bufL[Play_ptr];//32bit 左声道
	I2S->TX = i2s_bufR[Play_ptr];//32bit 右声道

//塞进了一对数据
	fifo_remain=fifo_remain+2;

//回头，判断还有没有空间，有空间就继续塞
	goto feed_start;

}


//中断方式服务函数
void I2S_IRQHandler(void)
{
	__IO uint32_t status;

	status = I2S->STATUS;

	//tx 阈值中断
	if (status & I2S_STATUS_TXTHIF_Msk) i2s_feed();

}















