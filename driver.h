/**
 * MeterFeed Library
 * 
 * by fp2.dev
 */

#pragma once

#include <iostream>
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
        bool Initialize(char* errorReason);

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
         * Get a byte of randomness.
         * 
         * @param The handle of the generator.
         * @param Pointer where to store the byte.
         * @param Error reason upon failure to retrieve data.
         */
        void GetByte(FT_HANDLE* handle, unsigned char *entropyByte, char* errorReason);

        private:
            vector<Generator> _generators;
            Generator* findGenerator(FT_HANDLE* handle);
    };
}