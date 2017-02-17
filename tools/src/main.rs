extern crate byteorder;
extern crate elf;
use std::path::PathBuf;
use std::fs::OpenOptions;
use std::io::*;
use byteorder::{LittleEndian, ReadBytesExt, WriteBytesExt};

macro_rules! try_IO {
    ($file_path:ident, $expr:expr) => (match $expr {
        Ok(value) => value,
        Err(error) => {
            println!("Error in {:?}: {:?}", $file_path, error);
            return;
        }
    })
}

fn main() {
    let args: Vec<_> = std::env::args().collect();
    if args.len() < 5 {
        println!("Usage image_builder virtual_offset physical_offset *.bin *.elf ...");
        return;
    }

    let out_file_path = PathBuf::from(&args[3]);
    let mut out_file = try_IO!(out_file_path, OpenOptions::new().read(true).write(true).create(true).truncate(true).open(&out_file_path));

    let virtual_offset = u64::from_str_radix(&args[1][2..], 16).unwrap();
    let physical_offset = u64::from_str_radix(&args[2][2..], 16).unwrap();
    let boot_file_header_length :u32 = 20;
    let mut payload_length :u32 = boot_file_header_length;
    let mut check_sum :u32 = 0;
    let entry_architecture_size = 32;

    let mut next_address = boot_file_header_length as u64;
    for in_file_path_str in &args[4..] {
        let in_file_path = PathBuf::from(in_file_path_str);
        let in_file = try_IO!(in_file_path, elf::File::open_path(&in_file_path));
        let in_file_name = in_file_path.file_name().unwrap().to_str().unwrap();

        if in_file.ehdr.data != elf::types::Data(1) {
            println!("{:?} is not little endian", in_file_name);
            return;
        }

        let architecture_size = match in_file.ehdr.class {
            elf::types::Class(1) => 32,
            elf::types::Class(2) => 64,
            elf::types::Class(_) => {
                println!("{:?} is of unknown architecture size", in_file_name);
                return;
            },
        };

        if next_address == boot_file_header_length as u64 && architecture_size != entry_architecture_size {
            println!("{:?} first ELF file is not {} bit", in_file_name, entry_architecture_size);
            return;
        }
        println!("{} {}bit", in_file_name, architecture_size);

        for section in &in_file.sections {
            if section.shdr.size == 0 || section.shdr.addr < virtual_offset || section.shdr.shtype != elf::types::SectionType(1) {
                continue;
            }

            let mut address = section.shdr.addr-virtual_offset;
            if address < next_address {
                println!("{:?} contains a section address which is getting smaller again", in_file_name);
                return;
            }
            payload_length += (address-next_address) as u32;
            next_address = address+section.shdr.size;
            address += physical_offset;
            println!("\t{:08X} {:08X} {:08X} {:8} {}", section.shdr.addr, address, address+section.shdr.size, section.shdr.size, section.shdr.name);

            try_IO!(out_file_path, out_file.seek(SeekFrom::Start(address)));
            try_IO!(out_file_path, out_file.write(&section.data));
            payload_length += section.shdr.size as u32;
        }
    }

    let block_size = 512;
    let needs_trailing_zeros = payload_length % block_size > 0;
    println!("Length: {} bytes", payload_length);
    payload_length = (payload_length+block_size-1)/block_size;
    println!("{} byte blocks: {}", block_size, payload_length);
    payload_length *= block_size;

    let mut boot_file_header = [0u8; 20];
    let jump_instruction :u32 = match entry_architecture_size {
        32 => 0xEA000000|((boot_file_header_length-8)/4),
        64 => 0x14000000|(boot_file_header_length/4),
        _ => {
            println!("Unknown entry architecture size");
            return;
        },
    };
    (&mut boot_file_header[0..4]).write_u32::<LittleEndian>(jump_instruction).unwrap();
    (&mut boot_file_header[4..12]).clone_from_slice(b"eGON.BT0");
    (&mut boot_file_header[12..16]).write_u32::<LittleEndian>(0x5F0A6C39).unwrap();
    (&mut boot_file_header[16..20]).write_u32::<LittleEndian>(payload_length).unwrap();
    try_IO!(out_file_path, out_file.seek(SeekFrom::Start(physical_offset)));
    try_IO!(out_file_path, out_file.write(&boot_file_header));
    if needs_trailing_zeros {
        try_IO!(out_file_path, out_file.seek(SeekFrom::Start(physical_offset-1+payload_length as u64)));
        try_IO!(out_file_path, out_file.write(&[0u8]));
    }

    try_IO!(out_file_path, out_file.seek(SeekFrom::Start(physical_offset)));
    for _ in 0..payload_length/4 {
        let mut buffer = [0u8; 4];
        try_IO!(out_file_path, out_file.read(&mut buffer));
        check_sum = check_sum.wrapping_add((&buffer[0..4]).read_u32::<LittleEndian>().unwrap());
    }
    (&mut boot_file_header[12..16]).write_u32::<LittleEndian>(check_sum).unwrap();
    try_IO!(out_file_path, out_file.seek(SeekFrom::Start(physical_offset+12)));
    try_IO!(out_file_path, out_file.write(&boot_file_header[12..16]));
    println!("Check sum: {:08X}", check_sum.swap_bytes());

    try_IO!(out_file_path, out_file.flush());
}
