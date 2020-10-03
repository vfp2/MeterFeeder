#include <iostream>
#include "ftd2xx/ftd2xx.h"
#include <string>
#include <vector>

using namespace std;

enum
{
	FTDI_DEVICE_LATENCY_MS = 2,
	FTDI_DEVICE_PACKET_USB_SIZE = 8,
	FTDI_DEVICE_TX_TIMEOUT = 5000
};

// Defines
vector<FT_HANDLE> ftHandles;
vector<string> deviceIds;
DWORD numDevices;

bool DevicesCount();
bool DevicesFind();
bool DevicesStartup();
void DevicesShutdown();

int main() {
	// open FTDI devices
	if (!DevicesStartup()) {
		cout << "Press any key to exit." << endl;
		cin.get();
		return -1;
	}

	unsigned char init_comm;
	DWORD bytesToTx;
	DWORD bytesTxd;

	cout << endl << "Sampling..." << endl << endl;

	for (int s = 0; s < 10; s++) {
		for (int i = 0; i < numDevices; i++) {
			// start streaming device
			// Send streaming command = 0x96
			init_comm = 0x96;
			bytesToTx = 1;
			bytesTxd = 0;

			// WRITE TO DEVICE
			FT_Purge(ftHandles[i], FT_PURGE_RX | FT_PURGE_TX);

			FT_STATUS ftdiStatus;
			ftdiStatus = FT_Write(ftHandles[i], &init_comm, bytesToTx, &bytesTxd);
			if (ftdiStatus != FT_OK || bytesTxd != bytesToTx) {
				cout << "%%%% Write Failed!" << endl;
				return -1;
			}

			// READ FROM DEVICE
			DWORD bytesRxd = 0;
			int bytesToRx = 8;
			unsigned char dxData[8];

			ftdiStatus = FT_Read(ftHandles[i], &dxData, bytesToRx, &bytesRxd);
			if (ftdiStatus != FT_OK || bytesRxd != bytesToRx) {
				cout << "%%%% Read Failed!" << endl;
				return -1;
			}

			cout << deviceIds[i] << ": ";
			for (int i = 0; i < (int)bytesRxd; i++) {
				printf("%X ", dxData[i] & 0xff);
			}
			cout << endl;
		}

		cout << endl;
	}

	return 0;
}

// function to find how many devices
bool DevicesCount() {
	FT_STATUS ftResult = FT_CreateDeviceInfoList(&numDevices);
	if (numDevices<1 || ftResult != FT_OK)
		return false;

	return true;
}

// function to find devices
bool DevicesFind() {
	vector<FT_DEVICE_LIST_INFO_NODE> devInfoList(numDevices);

	FT_STATUS ftResult = FT_GetDeviceInfoList(&devInfoList[0], &numDevices);
	if (ftResult != FT_OK)
		return false;

	for (unsigned i = 0; i<numDevices; i++) {
		string deviceId = devInfoList[i].SerialNumber;
		deviceId.resize(sizeof(devInfoList[i].SerialNumber));
		
		// do we have a MED1K or MED100K device?
		if (deviceId.find("QWR4") == 0) {
			// soliax: this was only returning one deviceId, even with 2 plugged in.
			//return deviceId;
			cout << "Found " << deviceId << ": " << devInfoList[i].Description << endl;
			deviceIds.push_back(deviceId);
		} else if (deviceId.compare("") == 0) {
			cout << "%%%% device not found!" << endl;
			return false;
		}
	}

	return true;
}

// function to start devices
bool DevicesStartup() {
	if (!DevicesCount()) {
		cout << "Failed to get number of devices" << endl;
		return false;
	} else {
		cout << numDevices << " devices connected" << endl;
	}

	if (!DevicesFind()) {
		cout << "Failed to find any compatible devices" << endl;
		return false;
	}


	// open devices by deviceId
	for (int i = 0; i < numDevices; i++) {
		FT_HANDLE ftHandle;
		string deviceId = deviceIds[i];

		FT_STATUS ftStatus = FT_OpenEx((PVOID)deviceId.c_str(), FT_OPEN_BY_SERIAL_NUMBER, &ftHandle);
		if (ftStatus != FT_OK) {
			cout << deviceId << " failed to open!" << endl;
			return false;
		}
		ftHandles.push_back(ftHandle);

		// configure FTDI transport parameters
		FT_SetLatencyTimer(ftHandles[i], FTDI_DEVICE_LATENCY_MS);
		FT_SetUSBParameters(ftHandles[i], FTDI_DEVICE_PACKET_USB_SIZE, FTDI_DEVICE_PACKET_USB_SIZE);
		FT_SetTimeouts(ftHandles[i], FTDI_DEVICE_TX_TIMEOUT, FTDI_DEVICE_TX_TIMEOUT);

		cout << deviceId << " initialized" << endl;
	}

	return true;
};

// function to shut down device
void DevicesShutdown() {
	// FT_Close(ftHandles);
}