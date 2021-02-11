
/**
 * MeterFeeder Library
 * 
 * by fp2.dev
 */

#include "driver.h"

int main(int argc, char *argv[]) {
	using namespace MeterFeeder;
	Driver* driver = new Driver();
	string errorReason = "";
	if (!driver->Initialize(&errorReason)) {
		cout << errorReason << endl;
		return -1;
	}

	// If invoked with command line arguments to specify the device serial number
	// and length of entropy (in bytes) to read only read from that device
	// args: <serial number> [length to read in bytes]
	if (argc >= 2) {
		Generator *generator = driver->FindGeneratorBySerial(argv[1]);
		int len = 1;
		if (argc == 3) len = atoi(argv[2]);
		UCHAR* bytes = (UCHAR*)malloc(len * sizeof(UCHAR));
		driver->GetBytes(generator->GetHandle(), len, bytes, &errorReason);
		if (errorReason.length() != 0) {
			cout << errorReason << endl;
		} else {
			for (int i = 0; i < len; i++) {
				cout << hex << (unsigned int) bytes[i];
			}
		}
		delete bytes;
		return 0;
	}

	// Else, read entropy from all the connected devices
	vector<Generator>* generators = driver->GetListGenerators();
	if (generators->size() == 0) {
		cout << "No generators" << endl;
		return -1;
	}
	for (size_t i = 0; i < generators->size(); i++) {
		Generator *generator = &generators->at(i);
		int len = 1;
		UCHAR* bytes = (UCHAR*)malloc(len * sizeof(UCHAR));
		driver->GetBytes(generator->GetHandle(), len, bytes, &errorReason);
		if (errorReason.length() != 0) {
			cout << errorReason << endl;
			errorReason = ""; // reset error for next device
			continue;
		}
		cout << generator->GetSerialNumber() << " (" << generator->GetDescription() << "): ";
		for (int j = 0; j < len; j++) {
			cout << (int)*(bytes+j) << " ";
		}
		cout << endl;

		delete bytes;
	}
	driver->Shutdown();
}
