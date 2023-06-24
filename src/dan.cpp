/**

MeterFeeder Library
by fp2.dev
*/

#include "driver.h"

#include  <iomanip>
#include  <chrono>

#define MAX_ERROR_REASONS 16
#define MAX_ERROR_REASON_LENGTH 256
#define MAX_SERIAL_NUMBER_LENGTH 32
#define MAX_SERIAL_NUMBERS 2
#define MAX_TRIAL_BYTES 4096
#define TRIAL_BYTES 2501

extern char errorReasons[MAX_ERROR_REASONS][MAX_ERROR_REASON_LENGTH];
extern char serialNumbers[MAX_SERIAL_NUMBERS][MAX_SERIAL_NUMBER_LENGTH];
extern int rngBytes[MAX_SERIAL_NUMBERS][MAX_TRIAL_BYTES];
extern int rngNumBytes[MAX_SERIAL_NUMBERS];
extern int numRngs,numErrors,trialBytes;

int meterfeeder (void);

int meterfeeder() {
    using namespace MeterFeeder;
    Driver* driver = new Driver();
    string errorReason = "";
    numErrors=0;
    if (!driver->Initialize(&errorReason))
    {
        strcpy(errorReasons[0],errorReason.c_str());
        numErrors++;
        return -1;
    }
    // Else, read entropy from all the connected devices
    vector* generators = driver->GetListGenerators();
    if (generators->size() == 0)
    {
        strcpy(errorReasons[0],"No generators");rs.numErrors=1;return -1;

    }

    for (size_t i = 0; i < generators->size(); i++) {
        Generator *generator = &generators->at(i);
        int len = trialBytes; 
        UCHAR* bytes = (UCHAR*)malloc(len * sizeof(UCHAR));
        driver->GetBytes(generator->GetHandle(), len, bytes, &errorReason);
        if (errorReason.length() != 0) 
        {
            strcpy(errorReasons[i],errorReason.c_str());
            numErrors++;
            errorReason = ""; // reset error for next device
            continue;
        }
        strcpy(serialNumbers[i],generator->GetSerialNumber().c_str());
        for (int j = 0; j < len; j++) rngBytes[i][j]=(int)*(bytes+j);		    
        delete bytes;
        rngNumBytes[i]=len;
    }
    numRngs=generators->size(); 
    driver->Shutdown();	
    return(0); 
}