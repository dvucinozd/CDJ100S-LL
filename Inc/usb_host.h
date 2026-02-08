/**
 ******************************************************************************
 * @file    usb_host.h
 * @brief   Header for USB Host module
 ******************************************************************************
 */

#ifndef __USB_HOST_H
#define __USB_HOST_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"

/* Exported types ------------------------------------------------------------*/
typedef enum {
  APPLICATION_IDLE = 0,
  APPLICATION_START,
  APPLICATION_READY,
  APPLICATION_DISCONNECT
} ApplicationTypeDef;

/* Exported constants --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
void MX_USB_HOST_Init(void);
void MX_USB_HOST_Process(void);

#ifdef __cplusplus
}
#endif

#endif /* __USB_HOST_H */
