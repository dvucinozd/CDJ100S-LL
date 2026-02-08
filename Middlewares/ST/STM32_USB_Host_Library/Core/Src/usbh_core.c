/**
 ******************************************************************************
 * @file    usbh_core.c
 * @author  MCD Application Team
 * @brief   USB Host core state machine implementation
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "usbh_core.h"

/* Private variables ---------------------------------------------------------*/
static uint8_t USBH_Pipes[15];

/* Private function prototypes -----------------------------------------------*/
static USBH_StatusTypeDef USBH_HandleEnum(USBH_HandleTypeDef *phost);
static void USBH_HandleSof(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef DeInitStateMachine(USBH_HandleTypeDef *phost);

/**
 * @brief  USBH_Init - Initialize the USB Host library
 * @param  phost: Host Handle
 * @param  pUsrFunc: User Callback
 * @param  id: Host id
 * @retval USBH Status
 */
USBH_StatusTypeDef
USBH_Init(USBH_HandleTypeDef *phost,
          void (*pUsrFunc)(USBH_HandleTypeDef *phost, uint8_t id), uint8_t id) {
  /* Host de-initializations */
  DeInitStateMachine(phost);

  /* Assign User process */
  if (pUsrFunc != NULL) {
    phost->pUser = pUsrFunc;
  }

  /* Set Host ID */
  phost->id = id;

  /* Initialize low level driver */
  USBH_LL_Init(phost);

  return USBH_OK;
}

/**
 * @brief  USBH_DeInit - De-Initialize the USB Host library
 * @param  phost: Host Handle
 * @retval USBH Status
 */
USBH_StatusTypeDef USBH_DeInit(USBH_HandleTypeDef *phost) {
  DeInitStateMachine(phost);

  if (phost->pData != NULL) {
    phost->pActiveClass->pData = NULL;
    USBH_LL_Stop(phost);
  }

  return USBH_OK;
}

/**
 * @brief  DeInitStateMachine - De-Initialize USB Host state machine
 * @param  phost: Host Handle
 * @retval USBH Status
 */
static USBH_StatusTypeDef DeInitStateMachine(USBH_HandleTypeDef *phost) {
  uint32_t i;

  /* Clear Pipes flags */
  for (i = 0U; i < 15U; i++) {
    USBH_Pipes[i] = 0U;
  }

  for (i = 0U; i < USBH_MAX_DATA_BUFFER; i++) {
    phost->device.Data[i] = 0U;
  }

  phost->gState = HOST_IDLE;
  phost->EnumState = ENUM_IDLE;
  phost->RequestState = USBH_OK;
  phost->Timer = 0U;
  phost->device.address = USBH_DEVICE_ADDRESS_DEFAULT;
  phost->device.speed = USBH_SPEED_FULL;
  phost->device.is_connected = 0U;

  return USBH_OK;
}

/**
 * @brief  USBH_RegisterClass - Link class driver to Host Core
 * @param  phost: Host Handle
 * @param  pclass: Class handle
 * @retval USBH Status
 */
USBH_StatusTypeDef USBH_RegisterClass(USBH_HandleTypeDef *phost,
                                      USBH_ClassTypeDef *pclass) {
  USBH_StatusTypeDef status = USBH_OK;

  if (pclass != NULL) {
    if (phost->ClassNumber < USBH_MAX_NUM_SUPPORTED_CLASS) {
      /* link the class to the USB Host handle */
      phost->pClass[phost->ClassNumber++] = pclass;
      status = USBH_OK;
    } else {
      USBH_ErrLog("Max Class Number reached");
      status = USBH_FAIL;
    }
  } else {
    USBH_ErrLog("Invalid Class handle");
    status = USBH_FAIL;
  }

  return status;
}

/**
 * @brief  USBH_SelectInterface - Select USB interface
 * @param  phost: Host Handle
 * @param  interface: Interface number
 * @retval USBH Status
 */
USBH_StatusTypeDef USBH_SelectInterface(USBH_HandleTypeDef *phost,
                                        uint8_t interface) {
  USBH_StatusTypeDef status = USBH_OK;

  if (interface < phost->device.CfgDesc.bNumInterfaces) {
    phost->device.current_interface = interface;
    USBH_UsrLog("Switching to Interface (#%d)", interface);
    USBH_UsrLog("Class    : %xh",
                phost->device.CfgDesc.Itf_Desc[interface].bInterfaceClass);
    USBH_UsrLog("SubClass : %xh",
                phost->device.CfgDesc.Itf_Desc[interface].bInterfaceSubClass);
    USBH_UsrLog("Protocol : %xh",
                phost->device.CfgDesc.Itf_Desc[interface].bInterfaceProtocol);
  } else {
    USBH_ErrLog("Cannot select interface");
    status = USBH_FAIL;
  }

  return status;
}

/**
 * @brief  USBH_GetActiveClass - Return Device Class
 * @param  phost: Host Handle
 * @retval Class Code
 */
uint8_t USBH_GetActiveClass(USBH_HandleTypeDef *phost) {
  return phost->device.CfgDesc.Itf_Desc[0].bInterfaceClass;
}

/**
 * @brief  USBH_FindInterface - Find interface index
 * @param  phost: Host Handle
 * @param  Class: Class code
 * @param  SubClass: SubClass code
 * @param  Protocol: Protocol code
 * @retval Interface index or 0xFF if not found
 */
uint8_t USBH_FindInterface(USBH_HandleTypeDef *phost, uint8_t Class,
                           uint8_t SubClass, uint8_t Protocol) {
  USBH_InterfaceDescTypeDef *pif;
  USBH_CfgDescTypeDef *pcfg;
  uint8_t if_ix = 0U;

  pcfg = &phost->device.CfgDesc;

  while (if_ix < USBH_MAX_NUM_INTERFACES) {
    pif = &pcfg->Itf_Desc[if_ix];
    if (((pif->bInterfaceClass == Class) || (Class == 0xFFU)) &&
        ((pif->bInterfaceSubClass == SubClass) || (SubClass == 0xFFU)) &&
        ((pif->bInterfaceProtocol == Protocol) || (Protocol == 0xFFU))) {
      return if_ix;
    }
    if_ix++;
  }

  return 0xFFU;
}

/**
 * @brief  USBH_FindInterfaceIndex - Find interface descriptor index
 * @param  phost: Host Handle
 * @param  interface_number: Interface number
 * @param  alt_settings: Alternate setting
 * @retval Interface index or 0xFF if not found
 */
uint8_t USBH_FindInterfaceIndex(USBH_HandleTypeDef *phost,
                                uint8_t interface_number,
                                uint8_t alt_settings) {
  USBH_InterfaceDescTypeDef *pif;
  USBH_CfgDescTypeDef *pcfg;
  uint8_t if_ix = 0U;

  pcfg = &phost->device.CfgDesc;

  while (if_ix < USBH_MAX_NUM_INTERFACES) {
    pif = &pcfg->Itf_Desc[if_ix];
    if ((pif->bInterfaceNumber == interface_number) &&
        (pif->bAlternateSetting == alt_settings)) {
      return if_ix;
    }
    if_ix++;
  }

  return 0xFFU;
}

/**
 * @brief  USBH_Start - Start the USB Host Core
 * @param  phost: Host Handle
 * @retval USBH Status
 */
USBH_StatusTypeDef USBH_Start(USBH_HandleTypeDef *phost) {
  /* Start the low level driver */
  USBH_LL_Start(phost);

  /* Activate VBUS on the port */
  USBH_LL_DriverVBUS(phost, TRUE);

  return USBH_OK;
}

/**
 * @brief  USBH_Stop - Stop USB Host Core
 * @param  phost: Host Handle
 * @retval USBH Status
 */
USBH_StatusTypeDef USBH_Stop(USBH_HandleTypeDef *phost) {
  /* Stop and cleanup the low level driver */
  USBH_LL_Stop(phost);

  /* De-activate VBUS on the port */
  USBH_LL_DriverVBUS(phost, FALSE);

  /* Free control pipes */
  USBH_FreePipe(phost, phost->Control.pipe_in);
  USBH_FreePipe(phost, phost->Control.pipe_out);

  return USBH_OK;
}

/**
 * @brief  USBH_ReEnumerate - Re-enumerate the device
 * @param  phost: Host Handle
 * @retval USBH Status
 */
USBH_StatusTypeDef USBH_ReEnumerate(USBH_HandleTypeDef *phost) {
  /* De-init state machine */
  if (phost->pActiveClass != NULL) {
    phost->pActiveClass->DeInit(phost);
    phost->pActiveClass = NULL;
  }

  /* Re-initialize host for new enumeration */
  phost->device.is_connected = 0U;
  phost->device.address = USBH_DEVICE_ADDRESS_DEFAULT;
  phost->device.speed = USBH_SPEED_FULL;

  phost->gState = HOST_IDLE;
  phost->EnumState = ENUM_IDLE;

  return USBH_OK;
}

/**
 * @brief  USBH_Process - USB Host Core main state machine process
 * @param  phost: Host Handle
 * @retval USBH Status
 */
USBH_StatusTypeDef USBH_Process(USBH_HandleTypeDef *phost) {
  USBH_StatusTypeDef status = USBH_FAIL;
  uint8_t idx;

  switch (phost->gState) {
  case HOST_IDLE:
    if (phost->device.is_connected == 1U) {
      /* Wait for 200ms and then reset port */
      USBH_Delay(200U);
      phost->gState = HOST_DEV_WAIT_FOR_ATTACHMENT;
      USBH_LL_ResetPort(phost);

      if (phost->pUser != NULL) {
        phost->pUser(phost, HOST_USER_CONNECTION);
      }
    }
    break;

  case HOST_DEV_WAIT_FOR_ATTACHMENT:
    break;

  case HOST_DEV_ATTACHED:
    /* Wait for 100ms after reset */
    USBH_Delay(100U);

    phost->device.speed = USBH_LL_GetSpeed(phost);

    phost->gState = HOST_ENUMERATION;

    phost->Control.pipe_out = USBH_AllocPipe(phost, 0x00U);
    phost->Control.pipe_in = USBH_AllocPipe(phost, 0x80U);

    /* Open control pipe */
    USBH_OpenPipe(phost, phost->Control.pipe_in, 0x80U, phost->device.address,
                  phost->device.speed, EP_TYPE_CTRL, phost->Control.pipe_size);

    USBH_OpenPipe(phost, phost->Control.pipe_out, 0x00U, phost->device.address,
                  phost->device.speed, EP_TYPE_CTRL, phost->Control.pipe_size);
    break;

  case HOST_ENUMERATION:
    /* Handle enumeration phase */
    if (USBH_HandleEnum(phost) == USBH_OK) {
      if (phost->pUser != NULL) {
        phost->pUser(phost, HOST_USER_SELECT_CONFIGURATION);
      }

      /* if enumeration complete set configuration */
      phost->gState = HOST_SET_CONFIGURATION;
    }
    break;

  case HOST_SET_CONFIGURATION:
    if (USBH_SetCfg(phost, phost->device.CfgDesc.bConfigurationValue) ==
        USBH_OK) {
      phost->gState = HOST_SET_WAKEUP_FEATURE;
      USBH_UsrLog("Default configuration set.");
    }
    break;

  case HOST_SET_WAKEUP_FEATURE:
    if ((phost->device.CfgDesc.bmAttributes & 0x20U) != 0U) {
      if (USBH_SetFeature(phost, FEATURE_SELECTOR_DEVICE) == USBH_OK) {
        phost->gState = HOST_CHECK_CLASS;
      }
    } else {
      phost->gState = HOST_CHECK_CLASS;
    }
    break;

  case HOST_CHECK_CLASS:
    if (phost->ClassNumber == 0U) {
      USBH_ErrLog("No Class has been registered.");
    } else {
      phost->pActiveClass = NULL;

      for (idx = 0U; idx < phost->ClassNumber; idx++) {
        if (phost->pClass[idx]->ClassCode == USBH_GetActiveClass(phost)) {
          phost->pActiveClass = phost->pClass[idx];
          break;
        }
      }

      if (phost->pActiveClass != NULL) {
        if (phost->pActiveClass->Init(phost) == USBH_OK) {
          phost->gState = HOST_CLASS_REQUEST;
          USBH_UsrLog("%s class started.", phost->pActiveClass->Name);

          if (phost->pUser != NULL) {
            phost->pUser(phost, HOST_USER_CLASS_SELECTED);
          }
        } else {
          phost->gState = HOST_ABORT_STATE;
          USBH_ErrLog("Device not supporting %s class.",
                      phost->pActiveClass->Name);
        }
      } else {
        phost->gState = HOST_ABORT_STATE;
        USBH_ErrLog("No registered class for this device.");
      }
    }
    break;

  case HOST_CLASS_REQUEST:
    /* process class standard control requests state machine */
    if (phost->pActiveClass != NULL) {
      status = phost->pActiveClass->Requests(phost);

      if (status == USBH_OK) {
        phost->gState = HOST_CLASS;

        if (phost->pUser != NULL) {
          phost->pUser(phost, HOST_USER_CLASS_ACTIVE);
        }
      } else if (status == USBH_FAIL) {
        phost->gState = HOST_ABORT_STATE;
        USBH_ErrLog("Device not responding.");
      }
    } else {
      phost->gState = HOST_ABORT_STATE;
      USBH_ErrLog("Invalid Class Driver.");
    }
    break;

  case HOST_CLASS:
    /* process class state machine */
    if (phost->pActiveClass != NULL) {
      phost->pActiveClass->BgndProcess(phost);
    }
    break;

  case HOST_DEV_DISCONNECTED:
    phost->device.is_connected = 0U;

    DeInitStateMachine(phost);

    /* Re-init host */
    if (phost->pActiveClass != NULL) {
      phost->pActiveClass->DeInit(phost);
      phost->pActiveClass = NULL;
    }

    if (phost->pUser != NULL) {
      phost->pUser(phost, HOST_USER_DISCONNECTION);
    }

    USBH_UsrLog("USB Device disconnected");

    /* Start the low level driver */
    USBH_LL_Start(phost);

    phost->gState = HOST_IDLE;
    break;

  case HOST_ABORT_STATE:
  default:
    break;
  }

  return USBH_OK;
}

/**
 * @brief  USBH_HandleEnum - Handle device enumeration
 * @param  phost: Host Handle
 * @retval USBH Status
 */
static USBH_StatusTypeDef USBH_HandleEnum(USBH_HandleTypeDef *phost) {
  USBH_StatusTypeDef status = USBH_BUSY;

  switch (phost->EnumState) {
  case ENUM_IDLE:
    /* Get device descriptor (first 8 bytes) */
    if (USBH_GetDevDesc(phost, 8U) == USBH_OK) {
      phost->Control.pipe_size = phost->device.DevDesc.bMaxPacketSize;

      phost->EnumState = ENUM_GET_FULL_DEV_DESC;

      /* Modify control channel config for new max packet size */
      USBH_OpenPipe(phost, phost->Control.pipe_in, 0x80U, phost->device.address,
                    phost->device.speed, EP_TYPE_CTRL,
                    phost->Control.pipe_size);

      USBH_OpenPipe(phost, phost->Control.pipe_out, 0x00U,
                    phost->device.address, phost->device.speed, EP_TYPE_CTRL,
                    phost->Control.pipe_size);
    }
    break;

  case ENUM_GET_FULL_DEV_DESC:
    /* Get full device descriptor */
    if (USBH_GetDevDesc(phost, USB_DEVICE_DESC_SIZE) == USBH_OK) {
      USBH_UsrLog("VID: %04Xh", phost->device.DevDesc.idVendor);
      USBH_UsrLog("PID: %04Xh", phost->device.DevDesc.idProduct);

      phost->EnumState = ENUM_SET_ADDR;
    }
    break;

  case ENUM_SET_ADDR:
    /* Set device address */
    if (USBH_SetAddress(phost, USBH_DEVICE_ADDRESS) == USBH_OK) {
      USBH_Delay(2U);

      phost->device.address = USBH_DEVICE_ADDRESS;

      USBH_UsrLog("Address (#%d) assigned.", phost->device.address);

      phost->EnumState = ENUM_GET_CFG_DESC;

      /* Modify control channel config for new device address */
      USBH_OpenPipe(phost, phost->Control.pipe_in, 0x80U, phost->device.address,
                    phost->device.speed, EP_TYPE_CTRL,
                    phost->Control.pipe_size);

      USBH_OpenPipe(phost, phost->Control.pipe_out, 0x00U,
                    phost->device.address, phost->device.speed, EP_TYPE_CTRL,
                    phost->Control.pipe_size);
    }
    break;

  case ENUM_GET_CFG_DESC:
    /* Get configuration descriptor (first 9 bytes) */
    if (USBH_GetCfgDesc(phost, USB_CONFIGURATION_DESC_SIZE) == USBH_OK) {
      phost->EnumState = ENUM_GET_FULL_CFG_DESC;
    }
    break;

  case ENUM_GET_FULL_CFG_DESC:
    /* Get full configuration descriptor */
    if (USBH_GetCfgDesc(phost, phost->device.CfgDesc.wTotalLength) == USBH_OK) {
      phost->EnumState = ENUM_GET_MFC_STRING_DESC;
    }
    break;

  case ENUM_GET_MFC_STRING_DESC:
    /* Get manufacturer string */
    if (phost->device.DevDesc.iManufacturer != 0U) {
      if (USBH_GetString(phost, phost->device.DevDesc.iManufacturer,
                         phost->device.Data, 0xFFU) == USBH_OK) {
        USBH_UsrLog("Manufacturer : %s", (char *)phost->device.Data);
        phost->EnumState = ENUM_GET_PRODUCT_STRING_DESC;
      }
    } else {
      USBH_UsrLog("Manufacturer : N/A");
      phost->EnumState = ENUM_GET_PRODUCT_STRING_DESC;
    }
    break;

  case ENUM_GET_PRODUCT_STRING_DESC:
    /* Get product string */
    if (phost->device.DevDesc.iProduct != 0U) {
      if (USBH_GetString(phost, phost->device.DevDesc.iProduct,
                         phost->device.Data, 0xFFU) == USBH_OK) {
        USBH_UsrLog("Product : %s", (char *)phost->device.Data);
        phost->EnumState = ENUM_GET_SERIALNUM_STRING_DESC;
      }
    } else {
      USBH_UsrLog("Product : N/A");
      phost->EnumState = ENUM_GET_SERIALNUM_STRING_DESC;
    }
    break;

  case ENUM_GET_SERIALNUM_STRING_DESC:
    /* Get serial number string */
    if (phost->device.DevDesc.iSerialNumber != 0U) {
      if (USBH_GetString(phost, phost->device.DevDesc.iSerialNumber,
                         phost->device.Data, 0xFFU) == USBH_OK) {
        USBH_UsrLog("Serial Number : %s", (char *)phost->device.Data);
        status = USBH_OK;
      }
    } else {
      USBH_UsrLog("Serial Number : N/A");
      status = USBH_OK;
    }
    break;

  default:
    break;
  }

  return status;
}

/**
 * @brief  USBH_LL_SetTimer - Set the initial frame number
 * @param  phost: Host Handle
 * @param  time: Frame number
 * @retval None
 */
void USBH_LL_SetTimer(USBH_HandleTypeDef *phost, uint32_t time) {
  phost->Timer = time;
}

/**
 * @brief  USBH_LL_IncTimer - Increment Host timer counter
 * @param  phost: Host Handle
 * @retval None
 */
void USBH_LL_IncTimer(USBH_HandleTypeDef *phost) {
  phost->Timer++;
  USBH_HandleSof(phost);
}

/**
 * @brief  USBH_HandleSof - Call SOF process for each class
 * @param  phost: Host Handle
 * @retval None
 */
static void USBH_HandleSof(USBH_HandleTypeDef *phost) {
  if ((phost->gState == HOST_CLASS) && (phost->pActiveClass != NULL)) {
    phost->pActiveClass->SOFProcess(phost);
  }
}

/**
 * @brief  USBH_LL_Connect - Handle USB device connection
 * @param  phost: Host Handle
 * @retval None
 */
void USBH_LL_Connect(USBH_HandleTypeDef *phost) {
  if (phost->device.is_connected == 0U) {
    phost->device.is_connected = 1U;
    phost->device.speed = USBH_SPEED_FULL;

    if (phost->gState == HOST_IDLE) {
      phost->gState = HOST_IDLE;
    }
  }
}

/**
 * @brief  USBH_LL_Disconnect - Handle USB device disconnection
 * @param  phost: Host Handle
 * @retval None
 */
void USBH_LL_Disconnect(USBH_HandleTypeDef *phost) {
  /* Stop host */
  USBH_LL_Stop(phost);

  /* Free control pipes */
  USBH_FreePipe(phost, phost->Control.pipe_in);
  USBH_FreePipe(phost, phost->Control.pipe_out);

  phost->device.is_connected = 0U;
  phost->gState = HOST_DEV_DISCONNECTED;
}

/**
 * @brief  USBH_LL_PortEnabled - Handle Port Enabled event
 * @param  phost: Host Handle
 * @retval None
 */
void USBH_LL_PortEnabled(USBH_HandleTypeDef *phost) {
  phost->gState = HOST_DEV_ATTACHED;
}

/**
 * @brief  USBH_LL_PortDisabled - Handle Port Disabled event
 * @param  phost: Host Handle
 * @retval None
 */
void USBH_LL_PortDisabled(USBH_HandleTypeDef *phost) {
  phost->gState = HOST_DEV_DISCONNECTED;
}

/* USB Device Descriptor Size */
#ifndef USB_DEVICE_DESC_SIZE
#define USB_DEVICE_DESC_SIZE 18U
#endif

/* USB Configuration Descriptor Size */
#ifndef USB_CONFIGURATION_DESC_SIZE
#define USB_CONFIGURATION_DESC_SIZE 9U
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
