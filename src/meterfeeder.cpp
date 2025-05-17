/**
 * MeterFeeder Library
 * 
 * by fp2.dev
 */

#include "driver.h"

#include  <iomanip>
#include  <chrono>
#include  <climits>

int main(int argc, char *argv[]) {
    using namespace MeterFeeder;
    Driver* driver = new Driver();
    string errorReason = "";
    if (!driver->Initialize(&errorReason)) {
        cout << errorReason << endl;
        delete driver;
        return -1;
    }

    // If invoked with command line arguments to specify the device serial number
    // and length of entropy (in bytes) to read only read from that device
    // args: <serial number> [length to read in bytes] [1 to run in infinite loop]
    if (argc >= 2) {
        Generator *generator = driver->FindGeneratorBySerial(argv[1]);
        if (!generator) {
            cout << "Generator not found: " << argv[1] << endl;
            delete driver;
            return -1;
        }
        
        int len = 1;  // default to 1 byte
        if (argc >= 3) {
            // Validate length parameter
            char* endptr;
            long long len_long = strtoll(argv[2], &endptr, 10);
            
            // Check for conversion errors
            if (*endptr != '\0') {
                cout << "Invalid length parameter: " << argv[2] << endl;
                delete driver;
                return -1;
            }
            
            // Check for negative values
            if (len_long < 0) {
                cout << "Length parameter must be positive" << endl;
                delete driver;
                return -1;
            }
            
            // Check for overflow
            if (len_long > INT_MAX) {
                cout << "Length parameter too large (max: " << INT_MAX << ")" << endl;
                delete driver;
                return -1;
            }
            
            len = static_cast<int>(len_long);
        }

        // Validate allocation size
        if (len > 1024 * 1024) {  // Limit to 1MB to prevent excessive memory allocation
            cout << "Length parameter too large (max: 1MB)" << endl;
            delete driver;
            return -1;
        }

        UCHAR* bytes = (UCHAR*)malloc(len * sizeof(UCHAR));
        if (!bytes) {
            cout << "Failed to allocate memory" << endl;
            delete driver;
            return -1;
        }
        
        bool cont = true;
        if (argc == 4 && atoi(argv[3]) == 1)
            cont = true;
        else
            cont = false;

        do {
            using namespace std::chrono;
            auto start = high_resolution_clock::now();

            driver->GetBytes(generator->GetHandle(), len, bytes, &errorReason);

            if (errorReason.length() != 0) {
                cout << errorReason << endl;
            } else {
                for (int i = 0; i < len; i++) {
                    cout << setfill('0') << setw(2) << right << hex << (unsigned int)bytes[i];
                }
            }

            if (cont)
                cout << endl << "\t====> " << std::dec << duration_cast<milliseconds>(high_resolution_clock::now() - start).count() << " ms" << endl << endl; 
        } while (cont);
            
        free(bytes);
        delete driver;
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
    delete driver;
}