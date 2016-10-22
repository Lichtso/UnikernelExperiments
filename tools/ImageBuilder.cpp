#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <elfio/elfio.hpp>

struct BootFileHeader {
    const static unsigned char magicSeed[8];
    const static unsigned int  checkSumSeed = 0x5F0A6C39,
                               blockSize = 512;

    unsigned int jumpInstruction;
    unsigned char magic[8];
    unsigned int checkSum, payloadLength;
    void initialize(unsigned char architectureSize) {
        checkSum = checkSumSeed;
        payloadLength = sizeof(struct BootFileHeader);
        memcpy(magic, magicSeed, sizeof(magicSeed));
        switch(architectureSize) {
            case 32:
                jumpInstruction = 0xEA000000|((payloadLength-8)/4);
                break;
            case 64:
                jumpInstruction = 0x14000000|(payloadLength/4);
                break;
        }
    }
};

const unsigned char BootFileHeader::magicSeed[8] = {'e', 'G', 'O', 'N', '.', 'B', 'T', '0'};

int output;
unsigned int buffer, virtualOffset, physicalOffset;
struct BootFileHeader header;

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
            unsigned int architectureSize = (reader.get_class() == ELFCLASS64) ? 64 : 32;
            if(i == 4)
                header.initialize(architectureSize);
            std::cout << argv[i] << " " << architectureSize << "bit" << std::endl;
            for(unsigned int i = 0; i < reader.sections.size(); ++i) {
                ELFIO::section* psec = reader.sections[i];
                int address = psec->get_address()-virtualOffset;
                if(psec->get_type() != SHT_PROGBITS || address < static_cast<int>(sizeof(struct BootFileHeader)))
                    continue;

                if(header.payloadLength < address+psec->get_size())
                    header.payloadLength = address+psec->get_size();
                address += physicalOffset;

                std::cout << "\t"
                          << psec->get_name()
                          << std::hex << std::uppercase
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
    header.payloadLength = (header.payloadLength+BootFileHeader::blockSize-1)/BootFileHeader::blockSize;
    std::cout << BootFileHeader::blockSize << " byte blocks: " << std::dec << header.payloadLength << std::endl;
    header.payloadLength *= BootFileHeader::blockSize;

    if(!checksumOnly) {
        lseek(output, physicalOffset, SEEK_SET);
        write(output, (char*)&header, sizeof(struct BootFileHeader));
        ftruncate(output, physicalOffset+header.payloadLength);
    }

    lseek(output, physicalOffset, SEEK_SET);
    header.checkSum = 0;
    for(unsigned int i = 0; i < header.payloadLength/4; ++i) {
        read(output, (char*)&buffer, 4);
        if(checksumOnly && i == 3)
            buffer = BootFileHeader::checkSumSeed;
        header.checkSum += buffer;
    }

    if(!checksumOnly) {
        lseek(output, physicalOffset+12, SEEK_SET);
        write(output, (char*)&header.checkSum, 4);
    }

    close(output);
    std::cout << "Check sum: " << std::hex << std::uppercase << __builtin_bswap32(header.checkSum) << std::endl;

    return 0;
}
