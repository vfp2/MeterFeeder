// ftd2xx_to_ftdi.h
#ifndef FTD2XX_TO_FTDI_H
#define FTD2XX_TO_FTDI_H

#include <libftdi1/ftdi.h>
#include <libusb-1.0/libusb.h>
#include <string.h>

// Type Definitions
typedef struct ftdi_context FT_HANDLE;
typedef int FT_STATUS;

// Status Codes
#define FT_OK 0
#define FT_INVALID_HANDLE -1
#define FT_DEVICE_NOT_FOUND -2
#define FT_DEVICE_NOT_OPENED -3
#define FT_IO_ERROR -4
#define FT_INVALID_PARAMETER -5
#define FT_DEVICE_LIST_NOT_READY -6

// Macros
#define FT_SUCCESS(status) ((status) == FT_OK)

// Device Information Node
typedef struct {
    char SerialNumber[16];
    char Description[64];
    int Flags;
    unsigned short VendorId;
    unsigned short ProductId;
} FT_DEVICE_LIST_INFO_NODE;

// Function Mappings
#define FT_Open(deviceNumber, pHandle) ftdi_new_and_open((pHandle))
#define FT_OpenEx(pArg1, Flags, pHandle) ftdi_new_and_open((pHandle))
#define FT_Close(ftHandle) ftdi_free(ftHandle)
#define FT_SetBaudRate(ftHandle, BaudRate) ftdi_set_baudrate(ftHandle, BaudRate)
#define FT_SetDataCharacteristics(ftHandle, WordLength, StopBits, Parity) \
    ftdi_set_line_property(ftHandle, (Parity), (StopBits), (WordLength))
#define FT_SetFlowControl(ftHandle, FlowControl, XonChar, XoffChar) \
    ftdi_setflowctrl(ftHandle, FlowControl)
#define FT_SetTimeouts(ftHandle, ReadTimeout, WriteTimeout) \
    ftdi_set_timeouts(ftHandle, ReadTimeout, WriteTimeout)
#define FT_Purge(ftHandle, Mask) ftdi_usb_purge_buffers(ftHandle)
#define FT_Read(ftHandle, lpBuffer, dwBytesToRead, lpBytesReturned) \
    ftdi_read_data(ftHandle, (unsigned char *)(lpBuffer), (int)(dwBytesToRead))
#define FT_Write(ftHandle, lpBuffer, dwBytesToWrite, lpBytesWritten) \
    ftdi_write_data(ftHandle, (unsigned char *)(lpBuffer), (int)(dwBytesToWrite))
#define FT_ResetDevice(ftHandle) ftdi_usb_reset(ftHandle)
#define FT_SetDtr(ftHandle) ftdi_setdtr(ftHandle, 1)
#define FT_ClrDtr(ftHandle) ftdi_setdtr(ftHandle, 0)
#define FT_SetRts(ftHandle) ftdi_setrts(ftHandle, 1)
#define FT_ClrRts(ftHandle) ftdi_setrts(ftHandle, 0)
#define FT_GetModemStatus(ftHandle, pModemStatus) ftdi_poll_modem_status(ftHandle, pModemStatus)
#define FT_SetChars(ftHandle, EventChar, EventCharEnabled, ErrorChar, ErrorCharEnabled) \
    ftdi_set_event_char(ftHandle, EventChar, EventCharEnabled)
#define FT_CreateDeviceInfoList(pNumDevs) ftdi_get_device_count(pNumDevs)
#define FT_ListDevices(pArg1, pArg2, Flags) ftdi_list_devices((pArg1), (pArg2), (Flags))
#define FT_GetQueueStatus(ftHandle, dwRxBytes) ftdi_get_queue_status(ftHandle, dwRxBytes)
#define FT_SetBitMode(ftHandle, Mask, Mode) ftdi_set_bitmode(ftHandle, Mask, Mode)
#define FT_GetBitMode(ftHandle, pMode) ftdi_get_bitmode(ftHandle, pMode)
#define FT_SetLatencyTimer(ftHandle, Latency) ftdi_set_latency_timer(ftHandle, Latency)
#define FT_GetLatencyTimer(ftHandle, pLatency) ftdi_get_latency_timer(ftHandle, pLatency)
#define FT_ReadEE(ftHandle, dwWordOffset, lpwValue) ftdi_read_eeprom(ftHandle, dwWordOffset, lpwValue)
#define FT_WriteEE(ftHandle, dwWordOffset, wValue) ftdi_write_eeprom(ftHandle, dwWordOffset, wValue)
#define FT_EraseEE(ftHandle) ftdi_erase_eeprom(ftHandle)
#define FT_GetDeviceInfoList(pDest, pNumDevs) ftdi_get_device_info_list((pDest), (pNumDevs))

// Helper Function for Open
static inline FT_STATUS ftdi_new_and_open(FT_HANDLE **ftHandle) {
    *ftHandle = ftdi_new();
    if (!*ftHandle) return FT_DEVICE_NOT_FOUND;
    return FT_OK;
}

// Helper Function for Device Info Count
static inline FT_STATUS ftdi_get_device_count(unsigned int *pNumDevs) {
    struct ftdi_context *ftdi = ftdi_new();
    if (!ftdi) return FT_IO_ERROR;
    struct ftdi_device_list *dev_list = NULL;
    int count = ftdi_usb_find_all(ftdi, &dev_list, 0, 0);
    ftdi_list_free(&dev_list);
    ftdi_free(ftdi);
    if (count < 0) return FT_IO_ERROR;
    *pNumDevs = count;
    return FT_OK;
}

// Helper Function for Device List
static inline FT_STATUS ftdi_list_devices(FT_DEVICE_LIST_INFO_NODE *pArg1, void *pArg2, unsigned int Flags) {
    struct ftdi_context *ftdi = ftdi_new();
    if (!ftdi) return FT_IO_ERROR;
    struct ftdi_device_list *dev_list = NULL;
    int count = ftdi_usb_find_all(ftdi, &dev_list, 0, 0);
    if (count < 0) {
        ftdi_list_free(&dev_list);
        ftdi_free(ftdi);
        return FT_IO_ERROR;
    }

    struct ftdi_device_list *current = dev_list;
    for (int i = 0; i < count; i++) {
        struct libusb_device_descriptor desc;
        libusb_get_device_descriptor(current->dev, &desc);
        snprintf(pArg1[i].SerialNumber, sizeof(pArg1[i].SerialNumber), "UNKNOWN");
        snprintf(pArg1[i].Description, sizeof(pArg1[i].Description), "FTDI Device");
        pArg1[i].Flags = 0; // Update flags if needed
        pArg1[i].VendorId = desc.idVendor;
        pArg1[i].ProductId = desc.idProduct;
        current = current->next;
    }

    ftdi_list_free(&dev_list);
    ftdi_free(ftdi);
    return FT_OK;
}

// Helper Function for Queue Status
static inline FT_STATUS ftdi_get_queue_status(FT_HANDLE *ftHandle, unsigned int *dwRxBytes) {
    int result = ftdi_read_data_get_chunksize(ftHandle, (unsigned int *)dwRxBytes);
    if (result < 0) return FT_IO_ERROR;
    return FT_OK;
}

// Helper Function for Device Info List
static inline FT_STATUS ftdi_get_device_info_list(FT_DEVICE_LIST_INFO_NODE *pDest, unsigned int *pNumDevs) {
    struct ftdi_context *ftdi = ftdi_new();
    if (!ftdi) return FT_IO_ERROR;
    struct ftdi_device_list *dev_list = NULL;
    int count = ftdi_usb_find_all(ftdi, &dev_list, 0, 0);
    if (count < 0) {
        ftdi_list_free(&dev_list);
        ftdi_free(ftdi);
        return FT_IO_ERROR;
    }

    struct ftdi_device_list *current = dev_list;
    for (int i = 0; i < count; i++) {
        struct libusb_device_descriptor desc;
        libusb_get_device_descriptor(current->dev, &desc);
        snprintf(pDest[i].SerialNumber, sizeof(pDest[i].SerialNumber), "UNKNOWN");
        snprintf(pDest[i].Description, sizeof(pDest[i].Description), "FTDI Device");
        pDest[i].Flags = 0;
        pDest[i].VendorId = desc.idVendor;
        pDest[i].ProductId = desc.idProduct;
        current = current->next;
    }

    *pNumDevs = count;
    ftdi_list_free(&dev_list);
    ftdi_free(ftdi);
    return FT_OK;
}

#endif // FTD2XX_TO_FTDI_H
