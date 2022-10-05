/**
 * MeterFeeder Library
 * 
 * by fp2.dev
 */

#pragma once

#include "../ftd2xx/ftd2xx.h"

#define MF_ERROR_STR_MAX_LEN 256

// FTDI transport parameters
enum {
	// Latency timer (milliseconds)
	// https://www.ftdichip.com/Support/Knowledgebase/index.html?ft_setlatencytimer.htm
	FTDI_DEVICE_LATENCY_MS = 2,

	// USB packet size for both in and out transfers
	// https://www.ftdichip.com/Support/Knowledgebase/index.html?ft_setusbparameters.htm
	// * Must be a mutiple of 64
	FTDI_DEVICE_PACKET_USB_SIZE_BYTES = 64,

	// Read/write timeout (milliseconds)
	// https://www.ftdichip.com/Support/Knowledgebase/index.html?ft_settimeouts.htm
	FTDI_DEVICE_TX_TIMEOUT_MS = 5000
};

// Meter Feed status // MF_STATUS
enum {
	MF_OK,
	MF_RXD_BYTES_LENGTH_WRONG = 1000,
};

#define FTDI_DEVICE_HALF_OF_UNIFORM_LSB		1.7763568394002505e-15
#define FTDI_DEVICE_2_PI					6.283185307179586

#define FTDI_DEVICE_START_STREAMING_COMMAND           0x96U
#define FTDI_DEVICE_STOP_STREAMING_COMMAND            0xe0U