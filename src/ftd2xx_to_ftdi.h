// ftd2xx_to_ftdi.h
#ifndef FTD2XX_TO_FTDI_H
#define FTD2XX_TO_FTDI_H

#include <libftdi1/ftdi.h>

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

// Helper Function for Open
static inline FT_STATUS ftdi_new_and_open(FT_HANDLE **ftHandle) {
    *ftHandle = ftdi_new();
    if (!*ftHandle) return FT_DEVICE_NOT_FOUND;
    return FT_OK;
}

// Helper Function for Device Info Count
static inline FT_STATUS ftdi_get_device_count(int *pNumDevs) {
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

#endif // FTD2XX_TO_FTDI_H

