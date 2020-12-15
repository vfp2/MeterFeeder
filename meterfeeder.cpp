
/**
 * MeterFeeder Library
 * 
 * by fp2.dev
 */

#include "driver.h"

int main() {
	using namespace MeterFeeder;
	static int counter = 0;
	Driver* driver = new Driver();
	string errorReason = "";
	if (!driver->Initialize(&errorReason)) {
		cout << errorReason << endl;
		return -1;
	}
	vector<Generator>* generators = driver->GetListGenerators();
	if (generators->size() == 0) {
		cout << "No generators" << endl;
		return -1;
	}
	for (int i = 0; i < generators->size(); i++) {
		Generator *generator = &generators->at(i);
		UCHAR byte[MF_FT_READ_MAX_BYTES]; 
		driver->GetByte(generator->GetHandle(), byte, &errorReason);
		if (errorReason.length() != 0) {
			cout << errorReason << endl;
			errorReason = ""; // reset error for next device
			continue;
		}

		int j = 0;
		int ampFactor = 100;
		for (j = 0; j < 512; j++ ) {
			counter += numOfSetBits(byte[j]);
			if (counter > (ampFactor - 1))
			{
				cout << 1 << endl;
				counter = 0;
			}
			else if (counter < (1 - ampFactor)) {
				cout << -1 << endl;
				counter = 0;
			}
		}

		// cout << generator->GetSerialNumber() << " (" << generator->GetDescription() << "): " << fc << "/13568" << endl;
	}
	driver->Shutdown();
}
