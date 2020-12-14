
/**
 * MeterFeeder Library
 * 
 * by fp2.dev
 */

#include "driver.h"

int main() {
	using namespace MeterFeeder;
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

		int fc = 0, j = 0;
		for (j = 0; j < 1695; j++ ) {
			fc += numOfSetBits(byte[j]);
		}
		fc += numOfSetBits(byte[++j]>>1);

		cout << generator->GetSerialNumber() << " (" << generator->GetDescription() << "): " << fc << "/13568" << endl;
	}
	driver->Shutdown();
}
