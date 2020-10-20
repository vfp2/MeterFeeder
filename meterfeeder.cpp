
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
		UCHAR byte;
		driver->GetByte(generator->GetHandle(), &byte, &errorReason);
		if (errorReason.length() != 0) {
			cout << errorReason << endl;
			errorReason = ""; // reset error for next device
			continue;
		}
		cout << generator->GetSerialNumber() << " : " << (int)byte << endl;
	}
	driver->Shutdown();
}
