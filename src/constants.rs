// constants.rs: Constants for MeterFeeder

pub const FTDI_DEVICE_START_STREAMING_COMMAND: u8 = 0x01; // Start streaming command
pub const FTDI_DEVICE_STOP_STREAMING_COMMAND: u8 = 0x02; // Stop streaming command
pub const FTDI_PURGE_RX: u32 = 0x01; // Purge RX buffer
pub const FTDI_PURGE_TX: u32 = 0x02; // Purge TX buffer
pub const TIMEOUT_MILLISECONDS: u64 = 1000; // Default timeout for USB operations