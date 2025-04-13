/// MeterFeeder Library
/// 
/// by fp2.dev

/// Maximum length for error strings
pub const MF_ERROR_STR_MAX_LEN: usize = 256;

/// FTDI transport parameters
pub mod ftdi {
    /// Latency timer (milliseconds)
    /// https://www.ftdichip.com/Support/Knowledgebase/index.html?ft_setlatencytimer.htm
    pub const DEVICE_LATENCY_MS: u32 = 2;

    /// USB packet size for both in and out transfers
    /// https://www.ftdichip.com/Support/Knowledgebase/index.html?ft_setusbparameters.htm
    /// * Must be a multiple of 64
    pub const DEVICE_PACKET_USB_SIZE_BYTES: u32 = 64;

    /// Read/write timeout (milliseconds)
    /// https://www.ftdichip.com/Support/Knowledgebase/index.html?ft_settimeouts.htm
    pub const DEVICE_TX_TIMEOUT_MS: u32 = 5000;

    /// Device commands
    pub const START_STREAMING_COMMAND: u8 = 0x96;
    pub const STOP_STREAMING_COMMAND: u8 = 0xe0;

    /// Device constants
    pub const HALF_OF_UNIFORM_LSB: f64 = 1.7763568394002505e-15;
    pub const TWO_PI: f64 = 6.283185307179586;
}

/// Meter Feed status codes
#[derive(Debug, PartialEq)]
pub enum Status {
    Ok,
    RxdBytesLengthWrong = 1000,
}

impl Status {
    pub fn is_ok(&self) -> bool {
        matches!(self, Status::Ok)
    }
}