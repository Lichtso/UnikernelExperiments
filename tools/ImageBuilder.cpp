#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <elfio/elfio.hpp>

struct BootFileHead {
    unsigned int jumpInstruction;
    unsigned char magic[8];
    unsigned int checkSum, payloadLength;
};
const unsigned int
    headerLength = sizeof(struct BootFileHead),
    blockSize = 512;

int output;
unsigned int buffer, virtualOffset, physicalOffset;
struct BootFileHead header = {
    0xEA000000|((headerLength-8)/4),
    {'e', 'G', 'O', 'N', '.', 'B', 'T', '0'},
    0x5F0A6C39,
    headerLength
};

int main(int argc, char** argv) {
    if(argc != 2 && argc < 5) {
        std::cout << "Usage: *.bin VirtualOffset PhysicalOffset [*.elf]" << std::endl;
        return 1;
    }
    bool checksumOnly = (argc == 2);

    output = open(argv[1], (checksumOnly) ? O_RDONLY : O_RDWR|O_CREAT|O_TRUNC, 0666);
    if(output < 0) {
        std::cout << "Failed to open *.bin" << std::endl;
        return 4;
    }

    if(!checksumOnly) {
        sscanf(argv[2], "%x", &virtualOffset);
        sscanf(argv[3], "%x", &physicalOffset);

        ELFIO::elfio reader;
        for(int i = 4; i < argc; ++i) {
            if(!reader.load(argv[i])) {
                std::cout << "Failed to open " << argv[i] << std::endl;
                return 2;
            }
            if(reader.get_encoding() != ELFDATA2LSB) {
                std::cout << argv[i] << " is not little endian" << std::endl;
                return 3;
            }
            std::cout << argv[i] << " " << ((reader.get_class() == ELFCLASS64) ? 64 : 32) << "bit" << std::endl;
            for(unsigned int i = 0; i < reader.sections.size(); ++i) {
                ELFIO::section* psec = reader.sections[i];
                int address = psec->get_address()-virtualOffset;
                if(psec->get_type() != SHT_PROGBITS || address < (int)headerLength)
                    continue;

                if(header.payloadLength < address+psec->get_size())
                    header.payloadLength = address+psec->get_size();
                address += physicalOffset;

                std::cout << "\t"
                          << psec->get_name()
                          << std::hex
                          << "\t0x" << psec->get_address()
                          << "\t0x" << address
                          << "\t0x" << (address+psec->get_size())
                          << std::dec
                          << "\t" << psec->get_size()
                          << std::endl;

                lseek(output, address, SEEK_SET);
                write(output, reader.sections[i]->get_data(), psec->get_size());
            }
        }
    } else {
        lseek(output, physicalOffset+0x10, SEEK_SET);
        read(output, (char*)&header.payloadLength, 4);
    }

    std::cout << "Length: " << std::dec << header.payloadLength << " bytes" << std::endl;
    header.payloadLength = (header.payloadLength+blockSize-1)/blockSize;
    std::cout << blockSize << " byte blocks: " << std::dec << header.payloadLength << std::endl;
    header.payloadLength *= blockSize;

    if(!checksumOnly) {
        lseek(output, physicalOffset, SEEK_SET);
        write(output, (char*)&header, headerLength);
        ftruncate(output, physicalOffset+header.payloadLength);
    }

    lseek(output, physicalOffset, SEEK_SET);
    header.checkSum = 0;
    for(unsigned int i = 0; i < header.payloadLength/4; ++i) {
        read(output, (char*)&buffer, 4);
        if(checksumOnly && i == 3)
            buffer = 0x5F0A6C39;
        header.checkSum += buffer;
    }

    if(!checksumOnly) {
        lseek(output, physicalOffset+12, SEEK_SET);
        write(output, (char*)&header.checkSum, 4);
    }

    close(output);
    std::cout << "Check sum: " << std::hex << __builtin_bswap32(header.checkSum) << std::endl;

    return 0;
}
