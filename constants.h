/**
 * MeterFeeder Library
 * 
 * by fp2.dev
 */

#pragma once

#include "ftd2xx/ftd2xx.h"

#define MF_ERROR_STR_MAX_LEN 256

// FTDI transport parameters
enum {
	// Latency timer (milliseconds)
	// https://www.ftdichip.com/Support/Knowledgebase/index.html?ft_setlatencytimer.htm
	FTDI_DEVICE_LATENCY_MS = 2,

	// USB packet size for both in and out tranfers
	// https://www.ftdichip.com/Support/Knowledgebase/index.html?ft_setusbparameters.htm
	FTDI_DEVICE_PACKET_USB_SIZE = 8,

	// Read/write timeout (milliseconds)
	// https://www.ftdichip.com/Support/Knowledgebase/index.html?ft_settimeouts.htm
	FTDI_DEVICE_TX_TIMEOUT_MS = 5000
};

// Meter Feed constants
enum {
	MF_DEVICE_ERROR	= -1,
	MF_OK,
};