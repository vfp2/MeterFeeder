## About the various models of MED Devices

They may be provided in a black or a translucent blue enclosure about the size of a thumb drive. The are all designed to output near 100KHz for easy comparison, except the 4MHz random generators I sent out originally (without labels) and MED1Kx3.

* MED100K S/N QWR4Axxx Internal 128 MHz, bias amplified 36x, output rate 98.765 KHz

* MED100Kx3 S/N QWR4Bxxx Internal 384 MHz, bias amplified 62x, output rate 99.896 KHz

* MED100Kx4 S/N QWR4Dxxx Internal 512 MHz, bias amplified 72x, output rate 98.765 KHz

* MED100Kx8 S/N QWR4Exxx Internal 1.024 GHz, bias amplified 101x, output rate 100.382 KHz

* MED100KP S/N QWR4Pxxx Purely Pseudorandom 100KHz output. Baseline generator – expected to be least responsive.

* MED100KR S/N QWR4Rxxx Internal 128 MHz, no bias amplification, just random output with no deterministic postprocessing, output rate 100 KHz

* MED100KX S/N QWR4Xxxx Internal 384 MHz, no bias amplification, special XOr processing – experimental, output rate 100 KHz

* PQ4000KM S/N QWR4Mxxx Internal 128 MHz, no bias amplification, a random generator with no deterministic postprocessing, output rate 4 MHz.

The distinguishing feature is the 5th digit of the serial number, which is a unique alpha character for each model type. The serial numbers can be read by a program since they are used to register the device to be read by the USB interface. The last 3 digits of the S/N are just sequential numbers from 1 to 999 to make each one unique. QWR4 is part of the S/N that is recognized by the interface program so it doesn’t connect to some other device that uses FTDI USB interface chips.

To complete this list, there is a Model MED1Kx3. I believe the S/N is QWR4Cxxx. Internal rate 384 MHz, bias amplified 320x, output rate 999.0 Hz. This is intended to compare with other generators used in the Global Consciousness Project (GCP), which output 1 Kbps. So far only one person has this generator tasked with the comparison for future GCP installations.