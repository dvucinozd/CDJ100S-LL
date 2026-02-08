/**
  ******************************************************************************
  * @file    usbh_diskio.h
  * @brief   Header for USB Host Disk I/O driver
  ******************************************************************************
  */

#ifndef __USBH_DISKIO_H
#define __USBH_DISKIO_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ff_gen_drv.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
extern const Diskio_drvTypeDef  USBH_Driver;

#ifdef __cplusplus
}
#endif

#endif /* __USBH_DISKIO_H */
