// driver.rs: USB device management for MeterFeeder

use ftdi::{Context, Device};
use std::error::Error;
use crate::generator::Generator;

pub struct Driver {
    context: Context,
    devices: Vec<Generator>,
}

impl Driver {
    pub fn new() -> Result<Self, Box<dyn Error>> {
        let context = Context::new()?;
        Ok(Self {
            context,
            devices: Vec::new(),
        })
    }

    pub fn initialize(&mut self) -> Result<(), Box<dyn Error>> {
        let devices = self.context.enumerate()?;

        for device_info in devices {
            let device = self.context.open_device(device_info)?;

            let generator = Generator::new(device, device_info.serial_number.clone())?;
            self.devices.push(generator);
        }

        Ok(())
    }

    pub fn find_generator(&self, serial_number: &str) -> Option<&Generator> {
        self.devices.iter().find(|g| g.serial_number() == serial_number)
    }
}