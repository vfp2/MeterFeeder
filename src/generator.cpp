/**
 * MeterFeeder Library
 * 
 * by fp2.dev
 */

#include "generator.h"

MeterFeeder::Generator::Generator(char* serialNumber, char* description, FT_HANDLE handle) {
    serialNumber_ = serialNumber;
    description_ = description;
    ftHandle_ = handle;
};

std::string MeterFeeder::Generator::GetSerialNumber() {
    return serialNumber_;
};

std::string MeterFeeder::Generator::GetDescription() {
    return description_;
};

FT_HANDLE MeterFeeder::Generator::GetHandle() {
    return ftHandle_;
};

int MeterFeeder::Generator::StartStreaming() {
    unsigned char startCommand = FTDI_DEVICE_START_STREAMING_COMMAND;
    unsigned int bytesTxd = 0;

	// Purge before writing
	FT_STATUS ftdiStatus = FT_Purge(ftHandle_, FT_PURGE_RX | FT_PURGE_TX);
    if (ftdiStatus != FT_OK) {
		return ftdiStatus;
	}

    // WRITE TO DEVICE
	ftdiStatus = FT_Write(ftHandle_, &startCommand, 1, &bytesTxd);
	if (ftdiStatus != FT_OK || bytesTxd != 1) {
		return ftdiStatus;
	}

    return MF_OK;
}

int MeterFeeder::Generator::StopStreaming() {
    unsigned char stopCommand = FTDI_DEVICE_STOP_STREAMING_COMMAND;
    unsigned int bytesTxd = 0;

	// Purge before writing
	FT_STATUS ftdiStatus = FT_Purge(ftHandle_, FT_PURGE_RX | FT_PURGE_TX);
    if (ftdiStatus != FT_OK) {
		return ftdiStatus;
	}

    // WRITE TO DEVICE
	ftdiStatus = FT_Write(ftHandle_, &stopCommand, 1, &bytesTxd);
	if (ftdiStatus != FT_OK || bytesTxd != 1) {
		return ftdiStatus;
	}

    return MF_OK;
}

int MeterFeeder::Generator::Read(unsigned int length, unsigned char* dxData) {
	unsigned int bytesRxd = 0;

    // READ FROM DEVICE
	FT_STATUS ftdiStatus = FT_Read(ftHandle_, dxData, length, &bytesRxd);
    if (bytesRxd != length) {
        return MF_RXD_BYTES_LENGTH_WRONG;
    }
    if (ftdiStatus != FT_OK) {
		return ftdiStatus;
    }

    return MF_OK;
}

void MeterFeeder::Generator::Close() {
    FT_Close(ftHandle_);
}