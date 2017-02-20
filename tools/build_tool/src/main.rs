extern crate byteorder;
extern crate elf;
use std::path::PathBuf;
use std::fs::{File, OpenOptions};
use std::io::*;
use byteorder::{LittleEndian, ReadBytesExt, WriteBytesExt};
use std::process::Command;

static BOOT_FILE_BLOCK_SIZE :u32 = 512;
static BOOT_FILE_HEADER_LENGTH :u32 = 20;

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
    let linker_path = match std::env::var("LD") {
        Ok(value) => value,
        Err(_) => {
            println!("Could not find linker");
            return;
        },
    };
    let args: Vec<_> = std::env::args().collect();
    if args.len() < 5 {
        println!("Usage: LD = [linker] build_tool [virtual_offset] [physical_offset] [out.bin] [in1.o] [in2.o] [in3.o] ...");
        return;
    }

    let virtual_offset = u64::from_str_radix(&args[1][2..], 16).unwrap();
    let physical_offset = u64::from_str_radix(&args[2][2..], 16).unwrap();
    let target = &args[3];
    if target.len() < 5 || &target[target.len()-4..target.len()] != ".bin" {
        println!("Output filename is invalid");
        return;
    }

    let mut object_files :[Vec<PathBuf>; 2] = [Vec::new(), Vec::new()];
    for in_file_path_str in &args[4..] {
        let in_file_path = PathBuf::from(in_file_path_str);
        let in_file = try_IO!(in_file_path, elf::File::open_path(&in_file_path));

        if in_file.ehdr.osabi != elf::types::ELFOSABI_NONE {
            println!("{:?} is of wrong ABI", in_file_path);
            return;
        }

        if in_file.ehdr.data != elf::types::ELFDATA2LSB {
            println!("{:?} is of wrong endianess", in_file_path);
            return;
        }

        match in_file.ehdr.class {
            elf::types::ELFCLASS32 => {
                if in_file.ehdr.machine != elf::types::EM_ARM {
                    println!("{:?} is of wrong machine architecture", in_file_path);
                    return;
                }
                object_files[0].push(in_file_path);
            },
            elf::types::ELFCLASS64 => {
                if in_file.ehdr.machine != elf::types::EM_AARCH64 {
                    println!("{:?} is of wrong machine architecture", in_file_path);
                    return;
                }
                object_files[1].push(in_file_path);
            },
            elf::types::Class(_) => {
                println!("{:?} is of unknown architecture size", in_file_path);
                return;
            },
        };
    }

    let binary_file_path = PathBuf::from(target);
    let mut binary_file = try_IO!(binary_file_path, OpenOptions::new().read(true).write(true).create(true).truncate(true).open(&binary_file_path));
    let mut payload_length :u32 = BOOT_FILE_HEADER_LENGTH;
    let mut check_sum :u32 = 0;

    let mut entry_point :u64 = 0;
    let mut virtual_address = virtual_offset+BOOT_FILE_HEADER_LENGTH as u64;
    for architecture in 0..2 {
        if object_files[architecture].len() == 0 {
            continue;
        }

        let architecture_names = ["32", "64"];
        let architecture_target = format!("{}{}", &target[..target.len()-4], architecture_names[architecture]);
        let linker_script_file_path = PathBuf::from(format!("{}.lds", architecture_target));
        let elf_file_path = PathBuf::from(format!("{}.elf", architecture_target));

        let mut linker_script = try_IO!(linker_script_file_path, File::create(&linker_script_file_path));
        try_IO!(linker_script_file_path, linker_script.write(b"SECTIONS {\n"));
        try_IO!(linker_script_file_path, linker_script.write(format!("    . = 0x{:08X};", virtual_address).as_bytes()));
        try_IO!(linker_script_file_path, linker_script.write(b"
    .text : { *(.text) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    .bss : { *(.bss) }
    /DISCARD/ : { *(.ARM.exidx*) }
    /DISCARD/ : { *(.comment) }
}"));

        println!("Linking: {:?}", elf_file_path);
        try_IO!(linker_path, Command::new(&linker_path)
            .arg("-flavor").arg("gnu")
            .arg("-s")
            .arg("--script").arg(linker_script_file_path)
            .arg("-o").arg(&elf_file_path)
            .args(&object_files[architecture])
            .status());

        let in_file = try_IO!(elf_file_path, elf::File::open_path(&elf_file_path));
        if entry_point == 0 {
            entry_point = in_file.ehdr.entry;
        }
        for section in &in_file.sections {
            if section.shdr.size == 0 || section.shdr.shtype != elf::types::SHT_PROGBITS {
                continue;
            }
            if section.shdr.addr < virtual_address {
                println!("{:?} section virtual addresses are in wrong order", elf_file_path);
                return;
            }

            let physical_address = section.shdr.addr-virtual_offset+physical_offset;
            payload_length += (section.shdr.addr-virtual_address) as u32;
            virtual_address = section.shdr.addr+section.shdr.size;
            println!("  {:08X} {:08X} {:08X} {}", section.shdr.addr, section.shdr.addr+section.shdr.size, physical_address, section.shdr.name);

            try_IO!(binary_file_path, binary_file.seek(SeekFrom::Start(physical_address)));
            try_IO!(binary_file_path, binary_file.write(&section.data));
            payload_length += section.shdr.size as u32;
        }
    }

    println!("Building: {:?}", binary_file_path);
    let needs_trailing_zeros = payload_length%BOOT_FILE_BLOCK_SIZE > 0;
    println!("  {} bytes", payload_length);
    payload_length = (payload_length+BOOT_FILE_BLOCK_SIZE-1)/BOOT_FILE_BLOCK_SIZE;
    println!("  {} blocks of {} bytes", payload_length, BOOT_FILE_BLOCK_SIZE);
    payload_length *= BOOT_FILE_BLOCK_SIZE;
    println!("  {:08X} entry point", entry_point);
    entry_point = (entry_point-virtual_offset)/4;
    if entry_point > 0xEFFFFF {
        println!("Entry point is out of reach");
        return;
    }

    let mut boot_file_header = [0u8; 20];
    let jump_instruction :u32 = if object_files[0].len() > 0
        { 0xEA000000|(entry_point as u32-2) } else
        { 0x14000000|(entry_point as u32) };
    (&mut boot_file_header[0..4]).write_u32::<LittleEndian>(jump_instruction).unwrap();
    (&mut boot_file_header[4..12]).clone_from_slice(b"eGON.BT0");
    (&mut boot_file_header[12..16]).write_u32::<LittleEndian>(0x5F0A6C39).unwrap();
    (&mut boot_file_header[16..20]).write_u32::<LittleEndian>(payload_length).unwrap();
    try_IO!(binary_file_path, binary_file.seek(SeekFrom::Start(physical_offset)));
    try_IO!(binary_file_path, binary_file.write(&boot_file_header));
    if needs_trailing_zeros {
        try_IO!(binary_file_path, binary_file.seek(SeekFrom::Start(physical_offset+payload_length as u64-1)));
        try_IO!(binary_file_path, binary_file.write(&[0u8]));
    }

    try_IO!(binary_file_path, binary_file.seek(SeekFrom::Start(physical_offset)));
    for _ in 0..payload_length/4 {
        let mut buffer = [0u8; 4];
        try_IO!(binary_file_path, binary_file.read(&mut buffer));
        check_sum = check_sum.wrapping_add((&buffer[0..4]).read_u32::<LittleEndian>().unwrap());
    }
    (&mut boot_file_header[12..16]).write_u32::<LittleEndian>(check_sum).unwrap();
    try_IO!(binary_file_path, binary_file.seek(SeekFrom::Start(physical_offset+12)));
    try_IO!(binary_file_path, binary_file.write(&boot_file_header[12..16]));
    println!("  {:08X} check sum", check_sum.swap_bytes());
}
