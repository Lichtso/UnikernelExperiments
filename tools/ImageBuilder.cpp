#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <elfio/elfio.hpp>

ELFIO::elfio reader;

struct BootFileHead {
    unsigned int jumpInstruction;
    unsigned char magic[8];
    unsigned int checkSum, payloadLength;
};
const unsigned int
    SPL_sRAM_Offset = 0x10000,
    SPL_SD_Offset = 0x2000,
    headerLength = sizeof(struct BootFileHead),
    blockSize = 512;

int output, address;
unsigned int buffer;
struct BootFileHead header = {
    0xEA000000|((headerLength-8)/4),
    {'e', 'G', 'O', 'N', '.', 'B', 'T', '0'},
    0x5F0A6C39,
    headerLength
};

int main(int argc, char** argv) {
    if(argc != 2 && argc != 3) {
        std::cout << "Usage: [*.elf] *.bin" << std::endl;
        return 1;
    }
    bool checksumOnly = (argc == 2);

    if(!checksumOnly) {
        if(!reader.load(argv[1])) {
            std::cout << "Failed to open *.elf" << std::endl;
            return 2;
        }

        if(reader.get_class() != ELFCLASS32 || reader.get_encoding() != ELFDATA2LSB) {
            std::cout << "32 bit little endian is required" << std::endl;
            return 3;
        }
    }

    if(checksumOnly)
        output = open(argv[1], O_RDWR, 0666);
    else
        output = open(argv[2], O_RDWR|O_CREAT|O_TRUNC, 0666);
    if(output < 0) {
        std::cout << "Failed to open *.bin" << std::endl;
        return 4;
    }

    if(!checksumOnly)
        for(unsigned int i = 0; i < reader.sections.size(); ++i) {
            ELFIO::section* psec = reader.sections[i];
            address = psec->get_address()-SPL_sRAM_Offset-headerLength;
            if(psec->get_type() != SHT_PROGBITS || address < 0)
                continue;

            address += headerLength;
            if(header.payloadLength < address+psec->get_size())
                header.payloadLength = address+psec->get_size();
            address += SPL_SD_Offset;

            std::cout << "\t[" << i << "] "
                      << psec->get_name()
                      << std::hex
                      << "\t0x" << psec->get_size()
                      << "\t0x" << address
                      << std::endl;

            lseek(output, address, SEEK_SET);
            write(output, reader.sections[i]->get_data(), psec->get_size());
        }
    else {
        lseek(output, SPL_SD_Offset+0x10, SEEK_SET);
        read(output, (char*)&header.payloadLength, 4);
    }

    std::cout << "Length: " << std::dec << header.payloadLength << " bytes" << std::endl;
    header.payloadLength = (header.payloadLength+blockSize-1)/blockSize;
    std::cout << blockSize << " byte blocks: " << std::dec << header.payloadLength << std::endl;
    header.payloadLength *= blockSize;

    if(!checksumOnly) {
        lseek(output, SPL_SD_Offset, SEEK_SET);
        write(output, (char*)&header, headerLength);
        ftruncate(output, SPL_SD_Offset+header.payloadLength);
    }

    lseek(output, SPL_SD_Offset, SEEK_SET);
    header.checkSum = 0;
    for(unsigned int i = 0; i < header.payloadLength/4; ++i) {
        read(output, (char*)&buffer, 4);
        if(checksumOnly && i == 3)
            buffer = 0x5F0A6C39;
        header.checkSum += buffer;
    }

    if(!checksumOnly) {
        lseek(output, SPL_SD_Offset+12, SEEK_SET);
        write(output, (char*)&header.checkSum, 4);
    }

    close(output);
    std::cout << "Check sum: " << std::hex << __builtin_bswap32(header.checkSum) << std::endl;

    return 0;
}
