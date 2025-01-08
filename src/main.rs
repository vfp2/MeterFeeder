// main.rs: Entry point for MeterFeeder in Rust

mod driver;
mod generator;
mod constants;

use std::{env, process, time::Instant};
use driver::Driver;
use generator::Generator;

fn main() {
    let args: Vec<String> = env::args().collect();

    // Create a Driver instance and initialize devices
    let mut driver = match Driver::new() {
        Ok(d) => d,
        Err(e) => {
            eprintln!("Failed to initialize driver: {}", e);
            process::exit(1);
        }
    };

    if let Err(e) = driver.initialize() {
        eprintln!("Driver initialization failed: {}", e);
        process::exit(1);
    }

    // Command-line argument handling
    if args.len() >= 2 {
        let serial_number = &args[1];
        let length: usize = args.get(2).and_then(|s| s.parse().ok()).unwrap_or(1);
        let continuous = args.get(3).map(|s| s == "1").unwrap_or(false);

        match driver.find_generator(serial_number) {
            Some(generator) => {
                if let Err(e) = generator.start_streaming() {
                    eprintln!("Failed to start streaming: {}", e);
                    process::exit(1);
                }

                let mut buffer = vec![0u8; length];
                loop {
                    let start_time = Instant::now();

                    if let Err(e) = generator.get_bytes(&mut buffer) {
                        eprintln!("Failed to get bytes: {}", e);
                        process::exit(1);
                    }

                    let elapsed_time = start_time.elapsed();
                    println!("Retrieved bytes: {:?} (Time taken: {:.2?})", buffer, elapsed_time);

                    if !continuous {
                        break;
                    }
                }

                if let Err(e) = generator.stop_streaming() {
                    eprintln!("Failed to stop streaming: {}", e);
                    process::exit(1);
                }
            }
            None => {
                eprintln!("Device with serial number {} not found", serial_number);
                process::exit(1);
            }
        }
    } else {
        eprintln!("Usage: meterfeeder <serial_number> [length] [continuous]");
    }
}