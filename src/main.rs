// main.rs: Entry point for MeterFeeder in Rust

mod driver;
mod generator;
mod constants;

use std::{env, process, time::Instant};
use driver::Driver;
use generator::MeterFeederError;

fn main() {
    let args: Vec<String> = env::args().collect();

    // Create a Driver instance and initialize devices
    let mut driver = match Driver::new() {
        Ok(d) => d,
        Err(e) => {
            eprintln!("Failed to initialize driver: {:?}", e);
            process::exit(1);
        }
    };

    if let Err(e) = driver.initialize() {
        eprintln!("Driver initialization failed: {:?}", e);
        process::exit(1);
    }

    // Command-line argument handling
    if args.len() >= 2 {
        let serial_number = &args[1];
        let length: usize = args.get(2).and_then(|s| s.parse().ok()).unwrap_or(1);
        let continuous = args.get(3).map(|s| s == "1").unwrap_or(false);

        loop {
            let start_time = Instant::now();

            match driver.get_bytes(serial_number, length) {
                Ok(bytes) => {
                    let elapsed_time = start_time.elapsed();
                    println!("Retrieved bytes: {:?} (Time taken: {:.2?})", bytes, elapsed_time);
                }
                Err(e) => {
                    eprintln!("Failed to get bytes: {:?}", e);
                    process::exit(1);
                }
            }

            if !continuous {
                break;
            }
        }
    } else {
        eprintln!("Usage: meterfeeder <serial_number> [length] [continuous]");
        eprintln!("\nAvailable devices:");
        for (serial, desc) in driver.get_generators_list() {
            eprintln!("  {} - {}", serial, desc);
        }
    }
}