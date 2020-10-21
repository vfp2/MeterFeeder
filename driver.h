/**
 * MeterFeeder Library
 * 
 * by fp2.dev
 */

#pragma once

#include <iostream>
#include <stdarg.h>
#include <string>
#include <vector>

#include "ftd2xx/ftd2xx.h"

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
        Generator* FindGenerator(FT_HANDLE handle);

        /**
         * Find generator specified by serial number.
         * 
         * @param Serial number identifying the device.
         * 
         * @return The Generator object if found, else null.
         */
        
        Generator* FindGenerator(string serialNumber);

        /**
         * Get a byte of randomness.
         * 
         * @param The handle of the generator.
         * @param Pointer where to store the byte.
         * @param Error reason upon failure to retrieve data.
         */
        void GetByte(FT_HANDLE handle, unsigned char *entropyByte, string* errorReason);

        private:
            vector<Generator> _generators;
            void makeErrorStr(string* errorReason, const char* format, ...);
    };
}
