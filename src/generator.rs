
use libusb::{Device, DeviceHandle, DeviceDescriptor};
use std::error::Error;

pub struct Generator {
    device: Device,
    handle: DeviceHandle,
    serial_number: String,
}

impl Generator {
    pub fn new(
        device: Device,
        handle: DeviceHandle,
        descriptor: DeviceDescriptor,
    ) -> Result<Self, Box<dyn Error>> {
        let serial_number = match handle.read_serial_number_string_ascii(&descriptor) {
            Ok(sn) => sn,
            Err(_) => "Unknown".to_string(),
        };

        Ok(Self {
            device,
            handle,
            serial_number,
        })
    }

    pub fn serial_number(&self) -> &str {
        &self.serial_number
    }

    pub fn get_bytes(&self, buffer: &mut [u8]) -> Result<(), Box<dyn Error>> {
        self.handle.read_bulk(0x81, buffer, std::time::Duration::from_secs(1))?;
        Ok(())
    }
}
