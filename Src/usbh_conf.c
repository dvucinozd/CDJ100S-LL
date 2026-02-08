/**
 ******************************************************************************
 * @file    usbh_conf.c
 * @brief   USB Host Configuration file
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"
#include "usbh_core.h"


/* Private variables ---------------------------------------------------------*/
HCD_HandleTypeDef hhcd_USB_OTG_FS;

/* External functions --------------------------------------------------------*/
void Error_Handler(void);

/*******************************************************************************
                       LL Driver Callbacks (HCD -> USB Host Library)
*******************************************************************************/

/**
 * @brief  SOF callback.
 * @param  hhcd: HCD handle
 * @retval None
 */
void HAL_HCD_SOF_Callback(HCD_HandleTypeDef *hhcd) {
  USBH_LL_IncTimer(hhcd->pData);
}

/**
 * @brief  Connect callback.
 * @param  hhcd: HCD handle
 * @retval None
 */
void HAL_HCD_Connect_Callback(HCD_HandleTypeDef *hhcd) {
  USBH_LL_Connect(hhcd->pData);
}

/**
 * @brief  Disconnect callback.
 * @param  hhcd: HCD handle
 * @retval None
 */
void HAL_HCD_Disconnect_Callback(HCD_HandleTypeDef *hhcd) {
  USBH_LL_Disconnect(hhcd->pData);
}

/**
 * @brief  Port Enabled callback.
 * @param  hhcd: HCD handle
 * @retval None
 */
void HAL_HCD_PortEnabled_Callback(HCD_HandleTypeDef *hhcd) {
  USBH_LL_PortEnabled(hhcd->pData);
}

/**
 * @brief  Port Disabled callback.
 * @param  hhcd: HCD handle
 * @retval None
 */
void HAL_HCD_PortDisabled_Callback(HCD_HandleTypeDef *hhcd) {
  USBH_LL_PortDisabled(hhcd->pData);
}

/**
 * @brief  Notify URB state change callback.
 * @param  hhcd: HCD handle
 * @param  chnum: Channel number
 * @param  urb_state: URB State
 * @retval None
 */
void HAL_HCD_HC_NotifyURBChange_Callback(HCD_HandleTypeDef *hhcd, uint8_t chnum,
                                         HCD_URBStateTypeDef urb_state) {
  /* To be used with OS to sync URB state with the global state machine */
}

/*******************************************************************************
                       LL Driver Interface (USB Host Library --> HCD)
*******************************************************************************/

/**
 * @brief  Initialize the Low Level portion of the Host driver.
 * @param  phost: Host handle
 * @retval USBH Status
 */
USBH_StatusTypeDef USBH_LL_Init(USBH_HandleTypeDef *phost) {
  /* Set the LL driver parameters */
  hhcd_USB_OTG_FS.Instance = USB_OTG_FS;
  hhcd_USB_OTG_FS.Init.Host_channels = 12;
  hhcd_USB_OTG_FS.Init.speed = HCD_SPEED_FULL;
  hhcd_USB_OTG_FS.Init.dma_enable = DISABLE;
  hhcd_USB_OTG_FS.Init.phy_itface = HCD_PHY_EMBEDDED;
  hhcd_USB_OTG_FS.Init.Sof_enable = DISABLE;
  hhcd_USB_OTG_FS.Init.low_power_enable = DISABLE;
  hhcd_USB_OTG_FS.Init.vbus_sensing_enable = DISABLE;
  hhcd_USB_OTG_FS.Init.use_external_vbus = DISABLE;

  hhcd_USB_OTG_FS.pData = phost;
  phost->pData = &hhcd_USB_OTG_FS;

  if (HAL_HCD_Init(&hhcd_USB_OTG_FS) != HAL_OK) {
    Error_Handler();
  }

  USBH_LL_SetTimer(phost, HAL_HCD_GetCurrentFrame(&hhcd_USB_OTG_FS));

  return USBH_OK;
}

/**
 * @brief  De-Initializes the Low Level portion of the Host driver.
 * @param  phost: Host handle
 * @retval USBH Status
 */
USBH_StatusTypeDef USBH_LL_DeInit(USBH_HandleTypeDef *phost) {
  HAL_HCD_DeInit(phost->pData);
  return USBH_OK;
}

/**
 * @brief  Starts the Low Level portion of the Host driver.
 * @param  phost: Host handle
 * @retval USBH Status
 */
USBH_StatusTypeDef USBH_LL_Start(USBH_HandleTypeDef *phost) {
  HAL_HCD_Start(phost->pData);
  return USBH_OK;
}

/**
 * @brief  Stops the Low Level portion of the Host driver.
 * @param  phost: Host handle
 * @retval USBH Status
 */
USBH_StatusTypeDef USBH_LL_Stop(USBH_HandleTypeDef *phost) {
  HAL_HCD_Stop(phost->pData);
  return USBH_OK;
}

/**
 * @brief  Returns the USB Host Speed from the low level driver.
 * @param  phost: Host handle
 * @retval USBH Speeds
 */
USBH_SpeedTypeDef USBH_LL_GetSpeed(USBH_HandleTypeDef *phost) {
  USBH_SpeedTypeDef speed = USBH_SPEED_FULL;

  switch (HAL_HCD_GetCurrentSpeed(phost->pData)) {
  case 0:
    speed = USBH_SPEED_HIGH;
    break;

  case 1:
    speed = USBH_SPEED_FULL;
    break;

  case 2:
    speed = USBH_SPEED_LOW;
    break;

  default:
    speed = USBH_SPEED_FULL;
    break;
  }
  return speed;
}

/**
 * @brief  Resets the Host Port of the Low Level Driver.
 * @param  phost: Host handle
 * @retval USBH Status
 */
USBH_StatusTypeDef USBH_LL_ResetPort(USBH_HandleTypeDef *phost) {
  HAL_HCD_ResetPort(phost->pData);
  return USBH_OK;
}

/**
 * @brief  Returns the last transferred packet size.
 * @param  phost: Host handle
 * @param  pipe: Pipe index
 * @retval Packet Size
 */
uint32_t USBH_LL_GetLastXferSize(USBH_HandleTypeDef *phost, uint8_t pipe) {
  return HAL_HCD_HC_GetXferCount(phost->pData, pipe);
}

/**
 * @brief  Opens a pipe of the Low Level Driver.
 * @param  phost: Host handle
 * @param  pipe: Pipe index
 * @param  epnum: Endpoint Number
 * @param  dev_address: Device USB address
 * @param  speed: Device Speed
 * @param  ep_type: Endpoint Type
 * @param  mps: Endpoint Max Packet Size
 * @retval USBH Status
 */
USBH_StatusTypeDef USBH_LL_OpenPipe(USBH_HandleTypeDef *phost, uint8_t pipe,
                                    uint8_t epnum, uint8_t dev_address,
                                    uint8_t speed, uint8_t ep_type,
                                    uint16_t mps) {
  HAL_HCD_HC_Init(phost->pData, pipe, epnum, dev_address, speed, ep_type, mps);
  return USBH_OK;
}

/**
 * @brief  Closes a pipe of the Low Level Driver.
 * @param  phost: Host handle
 * @param  pipe: Pipe index
 * @retval USBH Status
 */
USBH_StatusTypeDef USBH_LL_ClosePipe(USBH_HandleTypeDef *phost, uint8_t pipe) {
  HAL_HCD_HC_Halt(phost->pData, pipe);
  return USBH_OK;
}

/**
 * @brief  Submits a new URB to the low level driver.
 * @param  phost: Host handle
 * @param  pipe: Pipe index
 * @param  direction: Channel number
 * @param  ep_type: Endpoint Type
 * @param  token: Endpoint Type
 * @param  pbuff: pointer to URB data
 * @param  length: Length of URB data
 * @param  do_ping: activate do ping protocol (for high speed only)
 * @retval USBH Status
 */
USBH_StatusTypeDef USBH_LL_SubmitURB(USBH_HandleTypeDef *phost, uint8_t pipe,
                                     uint8_t direction, uint8_t ep_type,
                                     uint8_t token, uint8_t *pbuff,
                                     uint16_t length, uint8_t do_ping) {
  HAL_HCD_HC_SubmitRequest(phost->pData, pipe, direction, ep_type, token, pbuff,
                           length, do_ping);
  return USBH_OK;
}

/**
 * @brief  Gets a URB state from the low level driver.
 * @param  phost: Host handle
 * @param  pipe: Pipe index
 * @retval URB state
 */
USBH_URBStateTypeDef USBH_LL_GetURBState(USBH_HandleTypeDef *phost,
                                         uint8_t pipe) {
  return (USBH_URBStateTypeDef)HAL_HCD_HC_GetURBState(phost->pData, pipe);
}

/**
 * @brief  Drives VBUS.
 * @param  phost: Host handle
 * @param  state: VBUS state (0 = off, 1 = on)
 * @retval USBH Status
 */
USBH_StatusTypeDef USBH_LL_DriverVBUS(USBH_HandleTypeDef *phost,
                                      uint8_t state) {
  /* On STM32F746-Discovery, VBUS is controlled by the USB OTG controller itself
   */
  /* If external VBUS control is needed, implement GPIO control here */
  if (state == 0) {
    /* Drive low (VBUS off) */
  } else {
    /* Drive high (VBUS on) */
  }
  HAL_Delay(200);
  return USBH_OK;
}

/**
 * @brief  Sets toggle for a pipe.
 * @param  phost: Host handle
 * @param  pipe: Pipe index
 * @param  toggle: Toggle value (0/1)
 * @retval USBH Status
 */
USBH_StatusTypeDef USBH_LL_SetToggle(USBH_HandleTypeDef *phost, uint8_t pipe,
                                     uint8_t toggle) {
  if (hhcd_USB_OTG_FS.hc[pipe].ep_is_in) {
    hhcd_USB_OTG_FS.hc[pipe].toggle_in = toggle;
  } else {
    hhcd_USB_OTG_FS.hc[pipe].toggle_out = toggle;
  }
  return USBH_OK;
}

/**
 * @brief  Returns the current toggle of a pipe.
 * @param  phost: Host handle
 * @param  pipe: Pipe index
 * @retval Toggle value (0/1)
 */
uint8_t USBH_LL_GetToggle(USBH_HandleTypeDef *phost, uint8_t pipe) {
  uint8_t toggle = 0;

  if (hhcd_USB_OTG_FS.hc[pipe].ep_is_in) {
    toggle = hhcd_USB_OTG_FS.hc[pipe].toggle_in;
  } else {
    toggle = hhcd_USB_OTG_FS.hc[pipe].toggle_out;
  }
  return toggle;
}

/**
 * @brief  Delay routine for the USB Host Library
 * @param  Delay: Delay in ms
 * @retval None
 */
void USBH_Delay(uint32_t Delay) { HAL_Delay(Delay); }

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
