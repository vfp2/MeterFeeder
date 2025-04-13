// generator.rs: USB-based quantum random number generator

use rusb::{DeviceHandle, Error as UsbError};
use std::io::{Read, Write, Error as IoError};
use std::time::Duration;
use crate::constants::ftdi::{START_STREAMING_COMMAND, STOP_STREAMING_COMMAND};

/// Error type for MeterFeeder operations
#[derive(Debug)]
pub enum MeterFeederError {
    UsbError(UsbError),
    IoError(IoError),
    ReadLengthMismatch,
    DeviceNotFound,
}

impl From<UsbError> for MeterFeederError {
    fn from(err: UsbError) -> Self {
        MeterFeederError::UsbError(err)
    }
}

impl From<IoError> for MeterFeederError {
    fn from(err: IoError) -> Self {
        MeterFeederError::IoError(err)
    }
}

/// Generator struct that handles communication with the FTDI device
pub struct Generator {
    serial_number: String,
    description: String,
    device: DeviceHandle<rusb::Context>,
}

impl Generator {
    /// Create a new Generator instance
    pub fn new(serial_number: String, description: String, device: DeviceHandle<rusb::Context>) -> Self {
        Self {
            serial_number,
            description,
            device,
        }
    }

    /// Get the device's serial number
    pub fn get_serial_number(&self) -> &str {
        &self.serial_number
    }

    /// Get the device's description
    pub fn get_description(&self) -> &str {
        &self.description
    }

    /// Get the device handle
    pub fn get_device(&mut self) -> Result<&mut DeviceHandle<rusb::Context>, MeterFeederError> {
        Ok(&mut self.device)
    }

    /// Start streaming data from the device
    pub fn start_streaming(&mut self) -> Result<(), MeterFeederError> {
        // Reset the device
        self.device.reset()?;
        
        // Send start command
        let start_command = [START_STREAMING_COMMAND];
        self.device.write_bulk(0x02, &start_command, Duration::from_secs(1))?;
        
        Ok(())
    }

    /// Stop streaming data from the device
    pub fn stop_streaming(&mut self) -> Result<(), MeterFeederError> {
        // Reset the device
        self.device.reset()?;
        
        // Send stop command
        let stop_command = [STOP_STREAMING_COMMAND];
        self.device.write_bulk(0x02, &stop_command, Duration::from_secs(1))?;
        
        Ok(())
    }

    /// Read data from the device
    pub fn read(&mut self, length: usize) -> Result<Vec<u8>, MeterFeederError> {
        let mut buffer = vec![0u8; length];
        let bytes_read = self.device.read_bulk(0x81, &mut buffer, Duration::from_secs(1))?;
        
        if bytes_read != length {
            return Err(MeterFeederError::ReadLengthMismatch);
        }
        
        Ok(buffer)
    }

    /// Close the device connection
    pub fn close(self) {
        // Device is automatically closed when dropped
    }
}