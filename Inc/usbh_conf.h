/**
 ******************************************************************************
 * @file    usbh_conf.h
 * @brief   USB Host Configuration header
 ******************************************************************************
 */

#ifndef __USBH_CONF_H
#define __USBH_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Exported constants --------------------------------------------------------*/

/* Debug options - set to 1 to enable debug output */
#define USBH_DEBUG_LEVEL 0

/* Memory management macros */
#define USBH_MAX_NUM_ENDPOINTS 3
#define USBH_MAX_NUM_INTERFACES 2
#define USBH_MAX_NUM_CONFIGURATION 1
#define USBH_KEEP_CFG_DESCRIPTOR 1
#define USBH_MAX_NUM_SUPPORTED_CLASS 1
#define USBH_MAX_SIZE_CONFIGURATION 256
#define USBH_MAX_DATA_BUFFER 512

/* USB Host ID */
#define HOST_FS 0

/* Memory management - use standard malloc/free */
#define USBH_malloc malloc
#define USBH_free free
#define USBH_memset memset
#define USBH_memcpy memcpy

/* DEBUG macros */
#if (USBH_DEBUG_LEVEL > 0)
#define USBH_UsrLog(...)                                                       \
  printf(__VA_ARGS__);                                                         \
  printf("\n");
#else
#define USBH_UsrLog(...)
#endif

#if (USBH_DEBUG_LEVEL > 1)
#define USBH_ErrLog(...)                                                       \
  printf("ERROR: ");                                                           \
  printf(__VA_ARGS__);                                                         \
  printf("\n");
#else
#define USBH_ErrLog(...)
#endif

#if (USBH_DEBUG_LEVEL > 2)
#define USBH_DbgLog(...)                                                       \
  printf("DEBUG: ");                                                           \
  printf(__VA_ARGS__);                                                         \
  printf("\n");
#else
#define USBH_DbgLog(...)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __USBH_CONF_H */
