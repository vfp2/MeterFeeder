/**
 * MeterFeeder Library
 * 
 * by fp2.dev
 */

#pragma once

#include <cstring>
#include <iostream>
#include <stdarg.h>
#include <string>
#include <vector>
#include <math.h>

#include "../../ftd2xx/ftd2xx.h"
// #include "ftd2xx.h" //DJ 05.02.23

#include "constants.h"
#include "generator.h"

using namespace std;

namespace MeterFeeder {
    /**
     * Driver for MeterFeeder Library.
     * 
     * Provides functionality to initialize connected USB MED MMI generators and get entropy from them.
     */
    class Driver {
        public:
        /**
         * Initialize all the connected generators.
         * 
         * @param errorReason: Contains error reason string there was an error.
         * 
         * @return true on successful initialization, false on failure
         */
        bool Initialize(string* errorReason);

        /**
         * Shutdown and de-initialize all the generators.
         */
        void Shutdown();

         /**
         *  Stop streaming on the specified generator.
         * 
         * @param Handle of the generator.
         * @param Serial number identifying the device.
         * @param Error reason upon failure to retrieve data.
         */
        void Clear(FT_HANDLE handle, string* errorReason);

        /**
         * Get the number of connected and successfully initialized generators.
         * 
         * @return The number of initialized devices
         */
        int GetNumberGenerators();

        /**
         * Get the list of connected and successfully initialized generators.
         *
         * @return The list of Generators.
         */
        vector<Generator>* GetListGenerators();

        /**
         * Find generator specified by FT_HANDLE.
         * 
         * @param FT_HANDLE determined when the device was opened.
         * 
         * @return The Generator object if found, else null.
         */
        Generator* FindGeneratorByHandle(FT_HANDLE handle);

        /**
         * Find generator specified by serial number.
         * 
         * @param Serial number identifying the device.
         * 
         * @return The Generator object if found, else null.
         */
        Generator* FindGeneratorBySerial(string serialNumber);

        /**
         * Get a byte of randomness.
         * 
         * @param Handle of the generator.
         * @param Pointer where to store the byte.
         * @param Error reason upon failure to retrieve data.
         */
        void GetByte(FT_HANDLE handle, unsigned char *entropyByte, string* errorReason) {
            GetBytes(handle, 1, entropyByte, errorReason);
        }

        /**
         * Get bytes of randomness.
         * 
         * @param Handle of the generator.
         * @param Length in bytes to read.
         * @param Pointer where to store the bytes.
         * @param Error reason upon failure to retrieve data.
         */
        void GetBytes(FT_HANDLE handle, int length, unsigned char *entropyBytes, string* errorReason);

        private:
            vector<Generator> _generators;
            void makeErrorStr(string* errorReason, const char* format, ...);
    };
}
