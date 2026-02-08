/**
 ******************************************************************************
 * @file   fatfs.c
 * @brief  Code for fatfs applications - USB Host version
 ******************************************************************************
 */

#include "fatfs.h"

uint8_t retUSB;  /* Return value for USB */
char USBPath[4]; /* USB logical drive path */
FATFS USBFatFS;  /* File system object for USB logical drive */
FIL USBFile;     /* File object for USB */

/* USER CODE BEGIN Variables */
#include "diskio.h"
#include "ff.h"
#include "usbh_diskio.h"

/* USER CODE END Variables */

void MX_FATFS_Init(void) {
  /*## FatFS: Link the USB Host driver ###########################*/
  retUSB = FATFS_LinkDriver(&USBH_Driver, USBPath);

  /* USER CODE BEGIN Init */
  /* additional user code for init */
  /* USER CODE END Init */
}

/**
 * @brief  Gets Drive Path for USB
 * @retval Pointer to USB path string
 */
char *FATFS_GetUSBPath(void) { return USBPath; }

/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
