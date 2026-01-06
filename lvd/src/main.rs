use gpiod::{Chip, Options, EdgeDetect, Edge};
use std::fs;
use std::process::Command;
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::Arc;

const TRIGGER_SCRIPT: &str = "/usr/sbin/lvd-callback.sh";
const CONFIG_INI: &str = "/etc/lvd/config.ini";

fn find_ini() -> Option<(String, u32)> {
    let content = fs::read_to_string(CONFIG_INI).ok()?;
    let mut chip_name = String::new();
    let mut line_offset = String::new();

    for line in content.lines() {
        let line = line.trim();
        if line.starts_with("gpiochip=") {
            chip_name = line.split('=').nth(1)?.trim().to_string();
        } else if line.starts_with("line=") {
            line_offset = line.split('=').nth(1)?.trim().to_string();
        }
    }

    if chip_name.is_empty() || line_offset.is_empty() {
        return None;
    }

    let chip_name = if chip_name.chars().all(|c| c.is_ascii_digit()) {
        format!("gpiochip{}", chip_name)
    } else {
        chip_name.strip_prefix("/dev/").unwrap_or(&chip_name).to_string()
    };

    println!("config.ini: {}-{}.", chip_name, line_offset);
    Some((chip_name, line_offset.parse().ok()?))
}

fn find_lvd() -> Option<(String, u32)> {
    println!("Auto find LVD...");
    
    for i in 0..8 {
        let chip_name = format!("gpiochip{}", i);
        if let Ok(chip) = Chip::new(&chip_name) {
            for offset in 0..chip.num_lines() {
                if let Ok(info) = chip.line_info(offset) {
                    if !info.name.is_empty() && info.name == "LVD" {
                        return Some((chip_name, offset));
                    }
                }
            }
        }
    }
    None
}

fn trigger_callback(event_type: &str) {
    let _ = Command::new("sh")
        .arg("-c")
        .arg(&format!("{} {}", TRIGGER_SCRIPT, event_type))
        .status();
}

fn main() -> std::io::Result<()> {
    let (chip_name, offset) = find_ini().or_else(find_lvd).ok_or_else(|| {
        std::io::Error::new(std::io::ErrorKind::NotFound, "Not found LVD pin.")
    })?;

    let chip = Chip::new(&chip_name)?;
    let opts = Options::input([offset])
        .edge(EdgeDetect::Both)
        .consumer("edge_detector");
    
    let mut inputs = chip.request_lines(opts)?;

    let running = Arc::new(AtomicBool::new(true));

    ctrlc::set_handler(move || {
        println!("\nexit");
        std::process::exit(0);
    }).unwrap();

    println!("Start monitoring LVD...");

    loop {
        if let Ok(event) = inputs.read_event() {
            if event.edge == Edge::Rising {
                println!("Detected rising edge");
                trigger_callback("rising");
            } else if event.edge == Edge::Falling {
                println!("Detected falling edge");
                trigger_callback("falling");
            }
        }
    }

    Ok(())
}