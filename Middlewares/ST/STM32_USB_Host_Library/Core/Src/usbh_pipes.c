/**
 ******************************************************************************
 * @file    usbh_pipes.c
 * @brief   USB Host pipe handling
 ******************************************************************************
 */

#include "usbh_pipes.h"

static uint16_t USBH_PipeStates[15];

uint8_t USBH_AllocPipe(USBH_HandleTypeDef *phost, uint8_t ep_addr) {
  uint16_t pipe;
  for (pipe = 0U; pipe < 11U; pipe++) {
    if ((USBH_PipeStates[pipe] & 0x8000U) == 0U) {
      USBH_PipeStates[pipe] = 0x8000U | (uint16_t)ep_addr;
      return (uint8_t)pipe;
    }
  }
  return 0xFFU;
}

USBH_StatusTypeDef USBH_FreePipe(USBH_HandleTypeDef *phost, uint8_t idx) {
  if (idx < 11U) {
    USBH_PipeStates[idx] = 0U;
  }
  return USBH_OK;
}

USBH_StatusTypeDef USBH_OpenPipe(USBH_HandleTypeDef *phost, uint8_t pipe_num,
                                 uint8_t epnum, uint8_t dev_address,
                                 uint8_t speed, uint8_t ep_type, uint16_t mps) {
  return USBH_LL_OpenPipe(phost, pipe_num, epnum, dev_address, speed, ep_type,
                          mps);
}

USBH_StatusTypeDef USBH_ClosePipe(USBH_HandleTypeDef *phost, uint8_t pipe_num) {
  return USBH_LL_ClosePipe(phost, pipe_num);
}
