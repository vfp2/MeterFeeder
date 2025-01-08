// generator.rs: USB-based quantum random number generator

use ftdi::Device;
use std::error::Error;
use crate::constants::{FTDI_DEVICE_START_STREAMING_COMMAND, FTDI_DEVICE_STOP_STREAMING_COMMAND};

pub struct Generator {
    device: Device,
    serial_number: String,
}

impl Generator {
    pub fn new(device: Device, serial_number: String) -> Result<Self, Box<dyn Error>> {
        Ok(Self {
            device,
            serial_number,
        })
    }

    pub fn serial_number(&self) -> &str {
        &self.serial_number
    }

    pub fn start_streaming(&self) -> Result<(), Box<dyn Error>> {
        self.device.write_data(&[FTDI_DEVICE_START_STREAMING_COMMAND])?;
        Ok(())
    }

    pub fn stop_streaming(&self) -> Result<(), Box<dyn Error>> {
        self.device.write_data(&[FTDI_DEVICE_STOP_STREAMING_COMMAND])?;
        Ok(())
    }

    pub fn get_bytes(&self, buffer: &mut [u8]) -> Result<(), Box<dyn Error>> {
        self.device.read_data(buffer)?;
        Ok(())
    }

    pub fn description(&self) -> Result<String, Box<dyn Error>> {
        Ok(self.device.description()?)
    }
}