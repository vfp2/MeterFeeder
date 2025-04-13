// driver.rs: USB device management for MeterFeeder

use rusb::{self, Context, Device, DeviceHandle, UsbContext};
use std::io::{Read, Write, Error as IoError};
use std::collections::HashMap;
use std::sync::Arc;
use std::sync::Mutex;
use std::time::Duration;

use crate::constants::{self, ftdi as ftdi_consts};
use crate::generator::{Generator, MeterFeederError};

const FTDI_VENDOR_ID: u16 = 0x0403;
const FTDI_PRODUCT_ID: u16 = 0x6015;
const FTDI_INTERFACE: u8 = 0;
const FTDI_ENDPOINT_IN: u8 = 0x81;
const FTDI_ENDPOINT_OUT: u8 = 0x02;

/// Driver for MeterFeeder Library.
/// 
/// Provides functionality to initialize connected USB MED MMI generators and get entropy from them.
pub struct Driver {
    context: Context,
    generators: HashMap<String, Arc<Mutex<Generator>>>,
}

impl Driver {
    /// Create a new Driver instance
    pub fn new() -> Result<Self, MeterFeederError> {
        Ok(Self {
            context: Context::new()?,
            generators: HashMap::new(),
        })
    }

    /// Initialize all the connected generators
    pub fn initialize(&mut self) -> Result<(), MeterFeederError> {
        // Get list of devices
        let devices = self.context.devices()?;
        let mut found_devices = Vec::new();

        // Find FTDI devices
        for device in devices.iter() {
            let device_desc = device.device_descriptor()?;

            // Check if it's an FTDI device
            if device_desc.vendor_id() != FTDI_VENDOR_ID || device_desc.product_id() != FTDI_PRODUCT_ID {
                continue;
            }

            // Get device handle
            let mut handle = device.open()?;

            // Get serial number
            let languages = handle.read_languages(Duration::from_secs(1))?;
            if languages.is_empty() {
                continue;
            }

            let serial_number = match handle.read_serial_number_string(languages[0], &device_desc, Duration::from_secs(1)) {
                Ok(s) => s,
                Err(_) => continue,
            };

            // Check if it's a MED device
            if !serial_number.starts_with("QWR") {
                continue;
            }

            // Get product description
            let description = match handle.read_product_string(languages[0], &device_desc, Duration::from_secs(1)) {
                Ok(s) => s,
                Err(_) => continue,
            };

            // Configure device
            handle.set_active_configuration(1)?;
            handle.claim_interface(FTDI_INTERFACE)?;

            // Create and store the generator
            let generator = Generator::new(
                serial_number.clone(),
                description,
                handle,
            );

            found_devices.push((serial_number, generator));
        }

        // Clear existing generators
        self.generators.clear();

        // Store found devices
        for (serial, generator) in found_devices {
            self.generators.insert(
                serial,
                Arc::new(Mutex::new(generator)),
            );
        }

        if self.generators.is_empty() {
            return Err(MeterFeederError::DeviceNotFound);
        }

        Ok(())
    }

    /// Shutdown and de-initialize all generators
    pub fn shutdown(&mut self) {
        for (_, generator) in self.generators.drain() {
            let mut gen = generator.lock().unwrap();
            if let Ok(mut handle) = gen.get_device() {
                let _ = handle.release_interface(FTDI_INTERFACE);
            }
        }
    }

    /// Stop streaming on the specified generator
    pub fn clear(&self, serial_number: &str) -> Result<(), MeterFeederError> {
        if let Some(generator) = self.generators.get(serial_number) {
            let mut gen = generator.lock().unwrap();
            gen.stop_streaming()?;
        }
        Ok(())
    }

    /// Get the number of connected and successfully initialized generators
    pub fn get_number_generators(&self) -> usize {
        self.generators.len()
    }

    /// Get a list of all generator serial numbers
    pub fn get_serial_list(&self) -> Vec<String> {
        self.generators.keys().cloned().collect()
    }

    /// Get a list of all generators with their descriptions
    pub fn get_generators_list(&self) -> Vec<(String, String)> {
        self.generators.iter()
            .map(|(serial, gen)| {
                let gen = gen.lock().unwrap();
                (serial.clone(), gen.get_description().to_string())
            })
            .collect()
    }

    /// Get bytes of randomness from a specific generator
    pub fn get_bytes(&self, serial_number: &str, length: usize) -> Result<Vec<u8>, MeterFeederError> {
        if let Some(generator) = self.generators.get(serial_number) {
            let mut gen = generator.lock().unwrap();
            
            // Start streaming
            gen.start_streaming()?;
            
            // Read the data
            let data = gen.read(length)?;
            
            // Stop streaming
            gen.stop_streaming()?;
            
            Ok(data)
        } else {
            Err(MeterFeederError::DeviceNotFound)
        }
    }

    /// Get a single byte of randomness from a specific generator
    pub fn get_byte(&self, serial_number: &str) -> Result<u8, MeterFeederError> {
        let mut bytes = self.get_bytes(serial_number, 1)?;
        Ok(bytes[0])
    }

    /// Get a random 32-bit integer from a specific generator
    pub fn get_int32(&self, serial_number: &str) -> Result<i32, MeterFeederError> {
        let bytes = self.get_bytes(serial_number, 4)?;
        Ok(i32::from_le_bytes([bytes[0], bytes[1], bytes[2], bytes[3]]))
    }

    /// Get a random uniform number between 0 and 1 from a specific generator
    pub fn get_uniform(&self, serial_number: &str) -> Result<f64, MeterFeederError> {
        let bytes = self.get_bytes(serial_number, 8)?;
        let value = u64::from_le_bytes([
            bytes[0], bytes[1], bytes[2], bytes[3],
            bytes[4], bytes[5], bytes[6], bytes[7]
        ]);
        Ok((value as f64) / (u64::MAX as f64))
    }

    /// Get a random normal (Gaussian) number from a specific generator
    pub fn get_normal(&self, serial_number: &str) -> Result<f64, MeterFeederError> {
        // Use Box-Muller transform to convert uniform to normal distribution
        let u1 = self.get_uniform(serial_number)?;
        let u2 = self.get_uniform(serial_number)?;
        
        let z0 = (-2.0 * u1.ln()).sqrt() * (ftdi_consts::TWO_PI * u2).cos();
        Ok(z0)
    }
}