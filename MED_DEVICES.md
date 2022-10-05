## About the various models of MED Devices

There are slow but bias amplified devices that usually come in a black or a translucent turqoise enclosure about the size of a thumb drive. They are all designed to output near 100KHz for easy comparison, except the 4MHz random generators I sent out originally (without labels) and MED1Kx3.

The faster devices come in a black metal enclosure with a USB cable.

|   Name    |   S/N    | Internal Rate | Bias Amplification | Output Rate | Notes        |
|-----------|----------|---------------|--------------------|-------------|--------------|
| MED100K   | QWR4Axxx | 128 MHz       | 36x                | 98.765 KHz  |              |
| MED100Kx3 | QWR4Bxxx | 384 MHz       | 62x                | 99.896 KHz  |              |
| MED100Kx4 | QWR4Dxxx | 512 MHz       | 72x                | 98.765 KHz  |              |
| MED100Kx8 | QWR4Exxx | 1.024 GHz     | 101x               | 100.382 KHz |              |
| MED100KP  | QWR4Pxxx | Pseudorandom  | -                  | 100KHz      | baseline     |
| MED100KR  | QWR4Rxxx | 128 MHz       | -                  | 100 KHz     | raw entropy  |
| MED100KX  | QWR4Xxxx | 384 MHz       | XOR processing     | 100 KHz     | experimental |
| PQ4000KM  | QWR4Mxxx | 128 MHz       | -                  | 4 MHz.      |              |
| PQ32MU    | QWR7xxxx | 128 MHz       | -                  | 32 Mhz.     | high speed   |
| PQ128MU   | QWR7xxxx | 128 MHz       | -                  | 128 Mhz.    | high speed   |
| MED1Kx3   | QWR4Cxxx | 384 MHz       | 320x               | 999.0 Hz.   | GCP test*    |

* The serial numbers can be read by a program since they are used to register the device to be read by the USB interface. 
* The last 3 digits of the S/N are just sequential numbers from 1 to 999 to make each one unique. 
* QWR is part of the S/N that is recognized by the MeterFeeder interface program so it doesn’t connect to some other device that uses FTDI USB interface chips.

*This device is intended to compare with other generators used in the Global Consciousness Project (GCP), which output 1 Kbps. So far only one person has this generator tasked with the comparison for future GCP installations.