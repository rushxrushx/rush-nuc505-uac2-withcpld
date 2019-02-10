#include "NUC505Series.h"

#include "usbd_audio_play.h"
#include "usbd_audio_20.h"

/*--------------------------------------------------------------------------*/
/**
 * @brief       UAC Class Initial
 *
 * @param[in]   None
 *
 * @return      None
 *
 * @details     This function is used to configure endpoints for UAC class
 */
void UAC_Init_20(void)
{
    /* Configure USB controller */
    USBD->OPER = 2; /* High Speed */
    USBD_ENABLE_USB_INT(USBD_GINTEN_USBIEN_Msk|USBD_GINTEN_CEPIEN_Msk|USBD_GINTEN_EPAIEN_Msk|USBD_GINTEN_EPBIEN_Msk|USBD_GINTEN_EPCIEN_Msk|USBD_GINTEN_EPDIEN_Msk|USBD_GINTEN_EPEIEN_Msk|USBD_GINTEN_EPFIEN_Msk);
    
    /* Enable BUS interrupt */
    USBD_ENABLE_BUS_INT(USBD_BUSINTEN_DMADONEIEN_Msk|USBD_BUSINTEN_RESUMEIEN_Msk|USBD_BUSINTEN_RSTIEN_Msk|USBD_BUSINTEN_VBUSDETIEN_Msk);
    /* Reset Address to 0 */
    USBD_SET_ADDR(0);
    
    /********************/
    /* Control endpoint */
    USBD_SetEpBufAddr(CEP, CEP_BUF_BASE, CEP_BUF_LEN);
    USBD_ENABLE_CEP_INT(USBD_CEPINTEN_SETUPPKIEN_Msk|USBD_CEPINTEN_STSDONEIEN_Msk);
	
    /******************************************************************/
    /* EPA ==> ISO IN endpoint, address 0x01 (ISO_FEEDBACK_ENDPOINT) */
    USBD_SetEpBufAddr(EPA, EPA_BUF_BASE, EPA_BUF_LEN);
    USBD_SET_MAX_PAYLOAD(EPA, EPA_BUF_LEN);
    USBD_ConfigEp(EPA, ISO_IN_EP_NUM, USB_EP_CFG_TYPE_ISO, USB_EP_CFG_DIR_IN);
    USBD_ENABLE_EP_INT(EPA,USBD_EPINTEN_TXPKIEN_Msk );			
        
    /***********************************************************/
    /* EPB ==> ISO OUT endpoint, address 0x02 (ISO_OUT_EP_NUM) */
    USBD_SetEpBufAddr(EPB, EPB_BUF_BASE, EPB_BUF_LEN);
    USBD_SET_MAX_PAYLOAD(EPB, 200);
    USBD_ConfigEp(EPB, ISO_OUT_EP_NUM, USB_EP_CFG_TYPE_ISO, USB_EP_CFG_DIR_OUT);
    /* Enable EPB Data Received Interrupt for Audio Play (Read data from EPB Buffer) */
    USBD_ENABLE_EP_INT(EPB, USBD_EPINTEN_RXPKIEN_Msk);
		
    #ifdef __HID20__		
    /****************************************************************/
    /* EPC ==> Interrupt IN endpoint, address 0x03 (HID_INT_EP_NUM) */
    USBD_SetEpBufAddr(EPC, EPC_BUF_BASE, EPC_BUF_LEN);
    USBD_SET_MAX_PAYLOAD(EPC, EPC_MAX_PKT_SIZE);
    USBD_ConfigEp(EPC, HID_INT_EP_NUM, USB_EP_CFG_TYPE_INT, USB_EP_CFG_DIR_IN);
    /* Enable EPC IN Token Interrupt for HID (Write HID data to EPC) */		
    USBD_ENABLE_EP_INT(EPC, USBD_EPINTEN_INTKIEN_Msk);
    #endif
    

}
