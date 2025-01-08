
use libusb::{Context, Device, DeviceHandle};
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
        let devices = self.context.devices()?;

        for device in devices.iter() {
            let descriptor = device.device_descriptor()?;

            let handle = match device.open() {
                Ok(h) => h,
                Err(_) => continue,
            };

            let generator = Generator::new(device, handle, descriptor)?;
            self.devices.push(generator);
        }

        Ok(())
    }

    pub fn find_generator(&self, serial_number: &str) -> Option<&Generator> {
        self.devices.iter().find(|g| g.serial_number() == serial_number)
    }
}
