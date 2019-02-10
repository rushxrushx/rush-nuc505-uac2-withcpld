#ifndef __STM324xG_USB_AUDIOCODEC_H
#define __STM324xG_USB_AUDIOCODEC_H

#include "NUC505Series.h"
 
extern s16 m_i16PlayVolumeL;
extern s16 m_i16PlayVolumeR;
extern s16 m_u8PlayMute;
extern int16_t  m_i16PlayMaxVolume;        //Playing volume control max
extern int16_t  m_i16PlayMinVolume;        //Playing volume control min
extern int16_t  m_i16PlayResVolume;        //Playing volume control res
#define PLAY_MAX_VOL    0x0000 // 0 dB
#define PLAY_MIN_VOL    0x8100 // -127 dB 补码
#define PLAY_RES_VOL    0x0100 // 精度 1 dB
																	 
extern u8 audiostatus;							//bit0:0,暂停播放;1,继续播放
extern u8 alt_setting;   
extern u16 Play_ptr;							//即将播放的音频帧缓冲编号
extern u16 Write_ptr;							//当前保存到的音频缓冲编号 
extern u32 PlaySampleRate;
extern s32 play_speed;
extern u32 underrun_count;
extern u32 const i2s_BUFSIZE;								
extern u32 i2s_bufL[];
extern u32 i2s_bufR[];


/* Exported functions ------------------------------------------------------- */
void EVAL_AUDIO_Init(void);
void EVAL_AUDIO_Samplerate(void);
void EVAL_AUDIO_Play(void);
void EVAL_AUDIO_Stop(void);
void EVAL_AUDIO_VolumeCtl(void);
void EVAL_AUDIO_Mute(void);

#endif 






















