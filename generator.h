/**
 * MeterFeeder Library
 * 
 * by fp2.dev
 */

#pragma once

#include <string>

#include "ftd2xx/ftd2xx.h"

#include "constants.h"

namespace MeterFeeder {
    /**
     * A Mind-Enabled Device MMI (mind-matter interaction) generator.
     * It's a USB device that is a quantum random number generator.
     * The measurement of entropy (randomness) is based on the quantum
     * tunneling effect in the transistors on the onboard FTDI chip.
     * Onboard or computer-side post-processing methods like majority voting
     * and bias amplification can help boost th effect size of the
     * postulated idea that mental thought (intention) can have a
     * measurable effect on the the output of the random numbers.
     */
    class Generator {
        public:
            Generator(char* serialNumber, char* description, FT_HANDLE handle);

            /**
             * Get the generator's serial number. E.g. "QWR4A003"
             * 
             * @return The serial number of the generator device.
             */
            char* GetSerialNumber();

            /**
             * Get the generator's description. E.g. "MED100K 100 kHz v1.0"
             * 
             * @return The description of the generator device.
             */
            char* GetDescription();

            /**
             * Get the handle for interacting with the generator.
             * 
             * @return The FT_HANDLE for specifying this device.
             */
            FT_HANDLE GetHandle();

            /**
             * Send command to start streaming.
             * 
             * @return MF_DEVICE_ERROR on error communicating with the generator.
             */
            int Stream();

            /**
             * Read in the streamed entropy.
             * 
             * @param Pointer to where to store the streamed data (the random number).
             * 
             * @return MF_DEVICE_ERROR on error communicating with the generator.
             */
            int Read(UCHAR* dxData);

            /**
             * Close the generator.
             */
            void Close();

        private:
            char* serialNumber_;
            char* description_;
            FT_HANDLE ftHandle_;
    };
}