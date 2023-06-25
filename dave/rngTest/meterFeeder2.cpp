
/**
 * MeterFeeder Library
 * 
 * by fp2.dev
 */
#include "driver.h"

#include  <iomanip>
#include  <chrono>

/// Create a Timer, which will immediately begin counting
/// up from 0.0 seconds.
// class Timer {
//   public:
//     Timer() {
//       reset();
//     }
//     /// reset() makes the timer start over counting from 0.0 seconds.
//     void reset() {
//       unsigned __int64 pf;
//       QueryPerformanceFrequency( (LARGE_INTEGER *)&pf );
//       freq_ = 1.0 / (double)pf;
//       QueryPerformanceCounter( (LARGE_INTEGER *)&baseTime_ );
//     }
//     /// seconds() returns the number of seconds (to very high resolution)
//     /// elapsed since the timer was last created or reset().
//     double seconds() {
//       unsigned __int64 val;
//       QueryPerformanceCounter( (LARGE_INTEGER *)&val );
//       return (val - baseTime_) * freq_;
//     }
//     /// seconds() returns the number of milliseconds (to very high resolution)
//     /// elapsed since the timer was last created or reset().
//     double milliseconds() {
//       return seconds() * 1000.0;
//     }
//   private:
//     double freq_;
//     unsigned __int64 baseTime_;
// };

// Timer gt2;



int meterfeeder (void);

int meterfeeder() {
	using namespace MeterFeeder;
	Driver* driver = new Driver();
	string errorReason = "";
	double startTime;
	int i;
	char serialNumbers[2][16];
	if (!driver->Initialize(&errorReason)) 
	{
	    printf("\nError reason=%s",errorReason.c_str()); 
		return -1;
	}
	// Else, read entropy from all the connected devices
	vector<Generator>* generators = driver->GetListGenerators();
	if (generators->size() == 0) {printf("\nNo generators");return -1;}
	// startTime=gt2.seconds();
	for (i = 0; i < generators->size(); i++) {
		Generator *generator = &generators->at(i);
		int len = 256; //Can be any number of bytes 
		UCHAR* bytes = (UCHAR*)malloc(len * sizeof(UCHAR));
		driver->GetBytes(generator->GetHandle(), len, bytes, &errorReason);
		if (errorReason.length() != 0) 
		{
		    printf("\nError reason=%s",errorReason.c_str());
			errorReason = ""; // reset error for next device
			continue;
		}
		strcpy(serialNumbers[i],generator->GetSerialNumber().c_str());
		// cout << serialNumbers[i] << endl;
		//for (int j = 0; j < len; j++) printf("\nj=%d rngByte=%d",j,(int)*(bytes+j)); //Print serial bytes		    
		delete bytes;
	}
	printf("\nRNG generators:");for(i=0;i<generators->size();i++) printf("%s ",serialNumbers[i]);
	// printf("\nElapsed time = %f seconds",gt2.seconds()-startTime);
	driver->Shutdown();	
	return(0); 
}


int main()
{
  meterfeeder();

  return(0);
}
