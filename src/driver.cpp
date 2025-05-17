/**
 * MeterFeeder Library
 * 
 * by fp2.dev
 */

#include "driver.h"

bool MeterFeeder::Driver::Initialize(string* errorReason) {
    DWORD numDevices;
    FT_STATUS ftdiStatus = FT_CreateDeviceInfoList(&numDevices);
    if (ftdiStatus != FT_OK) {
        makeErrorStr(errorReason, "Error creating device info list. Check if generators are connected. [%d]", ftdiStatus);
        return false;
    }
    if (numDevices < 1) {
        makeErrorStr(errorReason, "No generators connected", ftdiStatus);
        return false;
    }

    vector<FT_DEVICE_LIST_INFO_NODE> devInfoList(numDevices);
    ftdiStatus = FT_GetDeviceInfoList(&devInfoList[0], &numDevices);
    if (ftdiStatus != FT_OK) {
        makeErrorStr(errorReason, "Error getting the device info list");
        return false;
    }

    _generators.clear();

    // Open devices by serialNumber
    for (DWORD i = 0; i < numDevices; i++) {
        string serialNumber = devInfoList[i].SerialNumber;
        serialNumber.resize(sizeof(devInfoList[i].SerialNumber));
        FT_HANDLE ftHandle = devInfoList[i].ftHandle;

        if (serialNumber.find("QWR") != 0) {
            // Skip other but MED1K or MED100K and PQ128MU devices
            continue;
        }

        // Open the current device
        ftdiStatus = FT_OpenEx(&devInfoList[i].SerialNumber, FT_OPEN_BY_SERIAL_NUMBER, &ftHandle);
        if (ftdiStatus != FT_OK) {
            makeErrorStr(errorReason, "Failed to connect to %s", &serialNumber);
            return false;
        }

        // Configure FTDI transport parameters
        ftdiStatus = FT_SetLatencyTimer(ftHandle, FTDI_DEVICE_LATENCY_MS);
        if (ftdiStatus != FT_OK) {
            makeErrorStr(errorReason, "Failed to set latency time for %s", &serialNumber);
            return false;
        }
        ftdiStatus = FT_SetUSBParameters(ftHandle, FTDI_DEVICE_PACKET_USB_SIZE_BYTES, FTDI_DEVICE_PACKET_USB_SIZE_BYTES);
        if (ftdiStatus != FT_OK) {
            makeErrorStr(errorReason, "Failed to set in/out packset size for %s", &serialNumber);
            return false;
        }
        ftdiStatus = FT_SetTimeouts(ftHandle, FTDI_DEVICE_TX_TIMEOUT_MS, FTDI_DEVICE_TX_TIMEOUT_MS);
        if (ftdiStatus != FT_OK) {
            makeErrorStr(errorReason, "Failed to set timeout time for %s", &serialNumber);
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
    for (size_t i = 0; i < _generators.size(); i++) {
        _generators[i].Close();
    }
};

void MeterFeeder::Driver::Clear(FT_HANDLE handle, string* errorReason) {
    // Find the specified generator
    Generator *generator = FindGeneratorByHandle(handle);
    if (!generator) {
        makeErrorStr(errorReason, "Could not find %s by the handle %x", generator->GetSerialNumber().c_str(), generator->GetHandle());
        return;
    }

    // Get the device to stop measuring randomness
    FT_STATUS streamStatus = generator->StopStreaming();
    if (streamStatus != FT_OK) {
        makeErrorStr(errorReason, "Error instructing %s to stop streaming entropy [%d]", generator->GetSerialNumber().c_str(), streamStatus);
        return;
    }
};

int MeterFeeder::Driver::GetNumberGenerators() {
    return _generators.size();    
};

vector<MeterFeeder::Generator>* MeterFeeder::Driver::GetListGenerators() {
    return &_generators;
};

void MeterFeeder::Driver::GetBytes(FT_HANDLE handle, int length, unsigned char* entropyBytes, string* errorReason) {
    // Find the specified generator
    Generator *generator = FindGeneratorByHandle(handle);
    if (!generator) {
        makeErrorStr(errorReason, "Could not find %s by the handle %x", generator->GetSerialNumber().c_str(), generator->GetHandle());
        return;
    }

    // Get the device to start measuring randomness
    FT_STATUS streamStatus = generator->StartStreaming();
    if (streamStatus != FT_OK) {
        makeErrorStr(errorReason, "Error instructing %s to start streaming entropy [%d]", generator->GetSerialNumber().c_str(), streamStatus);
        return;
    }

    // Read in the entropy
    FT_STATUS readStatus = generator->Read(length, entropyBytes);
    if (readStatus != FT_OK) {
        makeErrorStr(errorReason, "Error reading in entropy from %s [%d]", generator->GetSerialNumber().c_str(), readStatus);
        return;
    }
};

MeterFeeder::Generator* MeterFeeder::Driver::FindGeneratorByHandle(FT_HANDLE handle) {
    for (size_t i = 0; i < _generators.size(); i++) {
        if (_generators[i].GetHandle() == handle) {
            return &_generators[i];
        }
    }

    return nullptr;
};

MeterFeeder::Generator* MeterFeeder::Driver::FindGeneratorBySerial(string serialNumber) {
    for (size_t i = 0; i < _generators.size(); i++) {
        if (_generators[i].GetSerialNumber() == serialNumber) {
            return &_generators[i];
        }
    }

    return nullptr;
};

void MeterFeeder::Driver::makeErrorStr(string* errorReason, const char* format, ...) {
    char buffer[MF_ERROR_STR_MAX_LEN];
    va_list args;
    va_start (args, format);
    vsnprintf (buffer, MF_ERROR_STR_MAX_LEN-1, format, args);
    *errorReason = buffer;
    va_end (args);
};

/**
 * Code below for interfacing with MeterFeeder as a library
 */

#ifdef __APPLE__
    #define DllExport __attribute__((visibility("default")))
#elif defined __GNUC__
    #define DllExport __attribute__((visibility("default")))
#else
     #define DllExport __declspec(dllexport)
#endif

extern "C" {
    /**
     * Library functions exposed for consumption by Unity/C# or whomever
     */

    using namespace MeterFeeder;
    Driver driver = Driver();

    // Initialize the connected generators
    DllExport int MF_Initialize(char* pErrorReason) {
        string errorReason = "";
        int res = driver.Initialize(&errorReason);
        std::strcpy(pErrorReason, errorReason.c_str());
        return res;
    }

    // Shutdown and de-initialize all the generators.
    DllExport void MF_Shutdown() {
        driver.Shutdown();
    }

    // Shutdown and re-initialize all the generators.
    DllExport int MF_Reset(char* pErrorReason) {
        MF_Shutdown();
        return MF_Initialize(pErrorReason);
    }

    // Stop streaming on the specified generator.
    DllExport bool MF_Clear(char* generatorSerialNumber, char* pErrorReason) {
        string errorReason = "";
        Generator *generator = driver.FindGeneratorBySerial(generatorSerialNumber);
        if (!generator) {
            std::strcpy(pErrorReason, "Generator not found");
            return false;
        }
        driver.Clear(generator->GetHandle(), &errorReason);
        std::strcpy(pErrorReason, errorReason.c_str());
        if (*pErrorReason != '\0') {
            return false;
        }
        return true;
    }

    // Get the number of connected and successfully initialized generators.
    DllExport int MF_GetNumberGenerators() {
        return driver.GetNumberGenerators();
    }

      // Get the list of connected and successfully initialized generators with serial number and device description.
    // Array element format: <serial number>|<description>
    DllExport int MF_GetListGeneratorsWithSize(char** pGenerators, int arraySize) {
        vector<Generator>* generators = driver.GetListGenerators();
        int numGenerators = driver.GetNumberGenerators();
        
        if (arraySize < numGenerators) {
            return -1;  // Array too small
        }
        
        for (int i = 0; i < numGenerators; i++) {
            Generator generator = generators->at(i);
            string fullGenDesc = generator.GetSerialNumber() + "|" + generator.GetDescription();
            std::strcpy(pGenerators[i], fullGenDesc.c_str());
        }
        return numGenerators;
    }

    // Original function maintained for backward compatibility
    DllExport void MF_GetListGenerators(char** pGenerators) {
        MF_GetListGeneratorsWithSize(pGenerators, driver.GetNumberGenerators());
    }

    // Get the list of connected and successfully initialized generators.
    // Array element format: <serial number>
    DllExport int MF_GetSerialListGeneratorsWithSize(char** pGenerators, int arraySize) {
        vector<Generator>* generators = driver.GetListGenerators();
        int numGenerators = driver.GetNumberGenerators();
        
        if (arraySize < numGenerators) {
            return -1;  // Array too small
        }
        
        for (int i = 0; i < numGenerators; i++) {
            Generator generator = generators->at(i);
            std::strcpy(pGenerators[i], generator.GetSerialNumber().c_str());
        }
        return numGenerators;
    }

    // Original function maintained for backward compatibility
    DllExport void MF_GetSerialListGenerators(char** pGenerators) {
        MF_GetSerialListGeneratorsWithSize(pGenerators, driver.GetNumberGenerators());
    }

    // Get bytes of randomness.
    DllExport void MF_GetBytes(int length, unsigned char* buffer, char* generatorSerialNumber, char* pErrorReason) {
        string errorReason = "";
        Generator *generator = driver.FindGeneratorBySerial(generatorSerialNumber);
        if (!generator) {
            std::strcpy(pErrorReason, "Generator not found");
            return;
        }
        driver.GetBytes(generator->GetHandle(), length, buffer, &errorReason);
        std::strcpy(pErrorReason, errorReason.c_str());
    }

    // Get a byte of randomness.
    DllExport unsigned char MF_GetByte(char* generatorSerialNumber, char* pErrorReason) {
        unsigned char byte;
        MF_GetBytes(1, &byte, generatorSerialNumber, pErrorReason);
        return byte;
    }

    // Get a random 32 bit integer.
    DllExport int32_t MF_RandInt32(char* generatorSerialNumber, char* pErrorReason) {
        UCHAR *buffer = (UCHAR*)malloc(sizeof(int32_t));
        MF_GetBytes(sizeof(int32_t), buffer, generatorSerialNumber, pErrorReason);

        int32_t rc = 0;

        // (little endianess assumed)
        for (int i = 0; i < sizeof(int32_t); i++) {
            ((unsigned char*)&rc)[i] = buffer[i];
        }

        free(buffer);
        return rc;
    }

    // Get a random floating point number between [0,1)
    DllExport double MF_RandUniform(char* generatorSerialNumber, char* pErrorReason) {
        int sizeofUint48 = 6; // value for the mantissa part of double
        UCHAR *buffer = (UCHAR*)malloc(sizeof(sizeofUint48));
        MF_GetBytes(sizeofUint48, buffer, generatorSerialNumber, pErrorReason);

        uint64_t mantissa = 0;
        // (little endianess assumed)
        for (int i = 0; i < sizeofUint48; i++) {
            ((unsigned char*)&mantissa)[i] = buffer[i];
        }

        // copy 6 bytes into mantissa
        double uniform = (double)mantissa;
        uniform /= 281474976710656.0;  // 2^(6*8)

        free(buffer);
        return uniform;
    }

    // Get a random normal number with mean zero and standard deviation one
    DllExport double MF_RandNormal(char* generatorSerialNumber, char* pErrorReason) {
        // TODO: Not confident in the accuracy of the implementation here - should be checked one day

        // first half of calculation: create normU1
        double normU1 = MF_RandUniform(generatorSerialNumber, pErrorReason);
        if (*pErrorReason != '\0') {
            return 0;
        }
        normU1 += FTDI_DEVICE_HALF_OF_UNIFORM_LSB;

        // second half: create normU2
        double normU2 = MF_RandUniform(generatorSerialNumber, pErrorReason);
        if (*pErrorReason != '\0') {
            return 0;
        }
        normU2 += FTDI_DEVICE_HALF_OF_UNIFORM_LSB;

        // n1 = cos(2PI * u2) * sqrt(-2 * ln(u1)) 
        // n2 = sin(2PI * u2) * sqrt(-2 * ln(u1))
        double sqrtTerm = sqrt(-2.0 * log(normU1));
        double normal = cos(FTDI_DEVICE_2_PI * normU2) * sqrtTerm;
        //normalConjugate = sin(FTDI_DEVICE_2_PI * normU2) * sqrtTerm;
        return normal;
    }
}