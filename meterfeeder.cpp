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
FT_HANDLE ftHandle;
FT_STATUS ftdiStatus;

string DeviceFind();
bool DeviceStartup();
void DeviceShutdown();

int main() {
	// open FTDI device
	if (!DeviceStartup()) {
		cout << "Press any key to exit." << endl;
		cin.get();
		return -1;
	}

	// start streaming device
	// Send streaming command = 0x96
	unsigned char init_comm = 0x96;
	DWORD bytesToTx = 1;
	DWORD bytesTxd = 0;

	// WRITE TO DEVICE
	FT_Purge(ftHandle, FT_PURGE_RX | FT_PURGE_TX);

	ftdiStatus = FT_Write(ftHandle, &init_comm, bytesToTx, &bytesTxd);
	if (ftdiStatus != FT_OK || bytesTxd != bytesToTx) {
		cout << "%%%% Write Failed!" << endl;
		return -1;
	}

	// READ FROM DEVICE
	DWORD bytesRxd = 0;
	int bytesToRx = 8;
	unsigned char dxData[8];

	ftdiStatus = FT_Read(ftHandle, &dxData, bytesToRx, &bytesRxd);
	if (ftdiStatus != FT_OK || bytesRxd != bytesToRx) {
		cout << "%%%% Read Failed!" << endl;
		return -1;
	}

	for (int i = 0; i < (int)bytesRxd; i++) {
		printf("%X ", dxData[i] & 0xff);
	}

	return 0;
}

// function to find device
string DeviceFind() {
	DWORD devCount;

	FT_STATUS ftResult = FT_CreateDeviceInfoList(&devCount);
	if (devCount<1 || ftResult != FT_OK)
		return "";

	vector<FT_DEVICE_LIST_INFO_NODE> devInfoList(devCount);

	ftResult = FT_GetDeviceInfoList(&devInfoList[0], &devCount);
	if (ftResult != FT_OK)
		return "";

	for (unsigned i = 0; i<devCount; i++) {
		string deviceId = devInfoList[i].SerialNumber;
		deviceId.resize(sizeof(devInfoList[i].SerialNumber));

		// do we have a MED1K or MED100K device?
		if (deviceId.find("QWR4") == 0)
			return deviceId;
	}

	return "";
};

// function to start device
bool DeviceStartup() {

	string deviceId = DeviceFind();

	if (deviceId.compare("") == 0) {
		cout << "%%%% device not found!" << endl;
		return false;
	}

	cout << "deviceId: " << deviceId << "\n\n";

	// open device by deviceId
	FT_STATUS ftStatus = FT_OpenEx((PVOID)deviceId.c_str(), FT_OPEN_BY_SERIAL_NUMBER, &ftHandle);
	if (ftStatus != FT_OK) {
		cout << "%%%% device failed to open!" << endl;
		return false;
	}

	// configure FTDI transport parameters
	FT_SetLatencyTimer(ftHandle, FTDI_DEVICE_LATENCY_MS);
	FT_SetUSBParameters(ftHandle, FTDI_DEVICE_PACKET_USB_SIZE, FTDI_DEVICE_PACKET_USB_SIZE);
	FT_SetTimeouts(ftHandle, FTDI_DEVICE_TX_TIMEOUT, FTDI_DEVICE_TX_TIMEOUT);

	return true;
};

// function to shut down device
void DeviceShutdown() {
	FT_Close(ftHandle);
};
