/**
 * MeterFeed Library
 * 
 * by fp2.dev
 */

#include "generator.h"

MeterFeeder::Generator::Generator(char* serialNumber, char* description, FT_HANDLE handle) {
    serialNumber_ = serialNumber;
    description_ = description;
    ftHandle_ = handle;
};

char* MeterFeeder::Generator::GetSerialNumber() {
    return serialNumber_;
};

char* MeterFeeder::Generator::GetDescription() {
    return description_;
};

FT_HANDLE MeterFeeder::Generator::GetHandle() {
    return ftHandle_;
};

int MeterFeeder::Generator::Stream() {
    UCHAR initCommand = 0x96; // Streaming command
    DWORD bytesTxd = 0;

	// Purge before writing
	FT_STATUS ftdiStatus = FT_Purge(ftHandle_, FT_PURGE_RX | FT_PURGE_TX);
    if (ftdiStatus != FT_OK) {
		return MF_DEVICE_ERROR;
	}

    // WRITE TO DEVICE
	ftdiStatus = FT_Write(ftHandle_, &initCommand, 1, &bytesTxd);
	if (ftdiStatus != FT_OK || bytesTxd != 1) {
		return MF_DEVICE_ERROR;
	}

    return MF_OK;
}

int MeterFeeder::Generator::Read(UCHAR* dxData) {
	DWORD bytesRxd = 0;

    // READ FROM DEVICE
	FT_STATUS ftdiStatus = FT_Read(ftHandle_, dxData, 1, &bytesRxd);
	if (ftdiStatus != FT_OK || bytesRxd != 1) {
		return MF_DEVICE_ERROR;
	}

    return MF_OK;
}

void MeterFeeder::Generator::Close() {
    FT_Close(ftHandle_);
}