/********************************************************************************************
 UAC mode Setting 
********************************************************************************************/
//#define __UAC10_ONLY__			/* Sample code is for UAC 1.0 Only */
#define __UAC20_ONLY__		/* Sample code is for UAC 2.0 Only */
//#define __UAC10_20__			/* Sample code is for UAC 2.0 & UAC 1.0 (Try UAC 2.0 First) */

/********************************************************************************************
 UAC 1.0 Setting 
********************************************************************************************/	
/* UAC 1.0 VID PID */
#define USBD_VID                0x20b1
#define USBD_PID        		0x301f	
//#define __HID__							/* Enable HID for UAC 1.0 */
#ifdef __HID__
    #define __MEDIAKEY__				/* Select Mediakey for HID */		
    //#define __KEYBOARD__				/* Select Keyboard for HID */	
#endif
/********************************************************************************************
 UAC 2.0 Setting
********************************************************************************************/	
/* UAC 2.0 VID PID */
#define USBD_VID20              0x20b1
#define USBD_PID20        		0x301f
//#define __HID20__						/* Enable HID for UAC 2.0 */
#ifdef __HID20__
    #define __MEDIAKEY20__			/* Select Mediakey for HID */		
    //#define __KEYBOARD20__			/* Select Keyboard for HID */	
#endif 

extern S_USBD_INFO_T gsInfo_10,gsInfo_20;

void USBD_IRQHandler_10(void);
void USBD_IRQHandler_20(void);
void UAC_ClassRequest_10(void);
void UAC_ClassRequest_20(void);
void UAC_ClassOUT_10(void);
void UAC_ClassOUT_20(void);
void UAC_SetInterface_10(uint32_t u32AltInterface);
void UAC_SetInterface_20(uint32_t u32AltInterface);
void UAC_Init_10(void);
void UAC_Init_20(void);
void SOF_Handler(void);
void EPA_Handler20(void);
void EPA_Handler10(void);
void EPB_Handler(void);
void EPC_Handler(void);
void EPE_Handler(void);
void EPF_Handler(void);
void HID_UpdateKbData(void);

#define HID_CTRL_MUTE        0x01
#define HID_CTRL_VOLUME_INC  0x02
#define HID_CTRL_VOLUME_DEC  0x04

#define HID_CTRL_EJECT       0x08
#define HID_CTRL_PLAY        0x01
#define HID_CTRL_STOP        0x02
#define HID_CTRL_PAUSE       0x04
#define HID_CTRL_NEXT        0x08
#define HID_CTRL_PREVIOUS    0x10
#define HID_CTRL_RECORD      0x20
#define HID_CTRL_REWIND      0x40
#define HID_CTRL_FF          0x80

#define HID_TYPE_MEDIAKEY 	0
#define HID_TYPE_KEYBOARD 	1

/*-------------------------------------------------------------*/
/* Define the interrupt In EP number */
#define ISO_IN_EP_NUM          0x01
#define HID_IN_EP_NUM          0x03
#define ISO_OUT_EP_NUM         0x02
#define HID_INT_EP_NUM         HID_IN_EP_NUM
