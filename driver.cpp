/**
 * MeterFeed Library
 * 
 * by fp2.dev
 */

#include "driver.h"

bool MeterFeeder::Driver::Initialize(char* errorReason) {
	DWORD numDevices;
	FT_STATUS ftdiStatus = FT_CreateDeviceInfoList(&numDevices);
	if (ftdiStatus != FT_OK || numDevices < 1) {
		errorReason = (char*) "Error creating device info list. Check if generators are connected.";
		return false;
	}

	vector<FT_DEVICE_LIST_INFO_NODE> devInfoList(numDevices);
	ftdiStatus = FT_GetDeviceInfoList(&devInfoList[0], &numDevices);
	if (ftdiStatus != FT_OK) {
		errorReason = (char*) "Error getting the device info list.";
		return false;
	}

	// open devices by deviceId
	for (int i = 0; i < numDevices; i++) {
		string serialNumber = devInfoList[i].SerialNumber;
		serialNumber.resize(sizeof(devInfoList[i].SerialNumber));
		FT_HANDLE ftHandle = devInfoList[i].ftHandle;

		if (serialNumber.find("QWR4") != 0) {
			// Skip any other but MED1K or MED100K devices
			continue;
		}

		// Open the current device
		ftdiStatus = FT_OpenEx(&devInfoList[i].SerialNumber, FT_OPEN_BY_SERIAL_NUMBER, &ftHandle);
		if (ftdiStatus != FT_OK) {
			return false;
		}

		// Configure FTDI transport parameters
		FT_SetLatencyTimer(ftHandle, FTDI_DEVICE_LATENCY_MS);
		if (ftdiStatus != FT_OK) {
			sprintf(errorReason, "Failed to set latency time for %s", devInfoList[i].SerialNumber);
			return false;
		}
		FT_SetUSBParameters(ftHandle, FTDI_DEVICE_PACKET_USB_SIZE, FTDI_DEVICE_PACKET_USB_SIZE);
		if (ftdiStatus != FT_OK) {
			sprintf(errorReason, "Failed to set in/out packset size for %s", devInfoList[i].SerialNumber);
			return false;
		}
		FT_SetTimeouts(ftHandle, FTDI_DEVICE_TX_TIMEOUT_MS, FTDI_DEVICE_TX_TIMEOUT_MS);
		if (ftdiStatus != FT_OK) {
			sprintf(errorReason, "Failed to set timeout time for %s", devInfoList[i].SerialNumber);
			return false;
		}

		// Device is successfully initialized. Add it to the list of generators the driver will control.
		Generator generator = Generator(&devInfoList[i].SerialNumber[0], &devInfoList[i].Description[0], ftHandle);
		_generators.push_back(generator);
	}

	return true;
};

void MeterFeeder::Driver::Shutdown() {
	// Shutdown all generators
	for (int i = 0; i < _generators.size(); i++) {
		_generators[i].Close();
	}
};

int MeterFeeder::Driver::GetNumberGenerators() {
	return _generators.size();	
};

vector<MeterFeeder::Generator>* MeterFeeder::Driver::GetListGenerators() {
	return &_generators;
};

void MeterFeeder::Driver::GetByte(FT_HANDLE handle, unsigned char* entropyByte, char* errorReason) {
	// Find the specified generator
	Generator *generator = findGenerator(handle);
	if (!generator) {
		errorReason = (char*) "Could not find generator by that handle";
		return;
	}

	// Get the device to start measuring randomness
	if (MF_DEVICE_ERROR == generator->Stream()) {
		errorReason = (char*) "Error instructing the generator to start streaming entropy";
		return;
	}

	// Read in the entropy
	UCHAR dxData;
	if (MF_DEVICE_ERROR == generator->Read(entropyByte)) {
		errorReason = (char*) "Error reading in entropy from the generator";
		return;
	}
};

MeterFeeder::Generator* MeterFeeder::Driver::findGenerator(FT_HANDLE handle) {
	for (int i = 0; i < _generators.size(); i++) {
		if (_generators[i].GetHandle() == handle) {
			return &_generators[i];
		}
	}

	return nullptr;
};