/**
 * MeterFeeder Library
 * 
 * by fp2.dev
 */

#pragma once

#include "ftd2xx/ftd2xx.h"
#include <bitset>

#define MF_ERROR_STR_MAX_LEN 256

#define MF_FT_READ_MAX_BYTES 512

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

//
// https://www.techiedelight.com/count-set-bits-using-lookup-table/
//

// macros to generate the lookup table (at compile-time)
#define B2(n) n, n + 1, n + 1, n + 2
#define B4(n) B2(n), B2(n + 1), B2(n + 1), B2(n + 2)
#define B6(n) B4(n), B4(n + 1), B4(n + 1), B4(n + 2)
#define COUNT_BITS B6(0), B6(1), B6(1), B6(2)
#define NUM_SET_BITS(n) bLookup[*n & 0xff] +  bLookup[(*(n+1) >> 8) & 0xff] + bLookup[(*(n+2) >> 16) & 0xff] + bLookup[(*(n+3) >> 24) & 0xff]

// lookup-table to store the number of bits set for each index
// in the table. The macro COUNT_BITS generates the table.
inline unsigned int bLookup[256] = { COUNT_BITS };
 
// Function to count number of set bits in n
inline int numOfSetBits(UCHAR n)
{
    // print lookup table (number of bits set for integer i)
    // for (int i = 0; i < 256; i++)
    //    cout << i << " has " << lookup[i] << " bits\n";
 
    // assuming 32-bit(4 byte) integer, break the integer into 8-bit chunks
    // Note mask used 0xff is 11111111 in binary
 
    int numSetBits = bLookup[n & 0xff];
	int numNotSetBits = 8 - numSetBits;
	
    return numSetBits - numNotSetBits;
}