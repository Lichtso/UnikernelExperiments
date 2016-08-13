#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <elfio/elfio.hpp>

ELFIO::elfio reader;

struct BootFileHead {
    unsigned int jumpInstruction;
    unsigned char magic[8];
    unsigned int checkSum, length, pubicHeaderSize,
                 a15PowerGpio, returnAddress, runAddress, bootCPU;
    unsigned char platform[8];
    unsigned int prvtHeaderSize,
                 enableDebugMessages,
                 dramClk, dramType, dramZq, dramOdtEnable,
                 dramParam[2], dramMr[4], dramTpr[22],
                 uartPort, uartCtrl[4];
    int enableJtag;
    unsigned int jtagGpio[10], storageGpio0[32],
                 storageGpio2[32], userData[64],
                 commitString[16];
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
    0, 0x30, 0,
    SPL_sRAM_Offset,
    SPL_sRAM_Offset,
    0,
    {0, 0, '4', '.', '0', '.', '0', 0},
    0, 1,
    0x000002A0, 0x00000003, 0x003B3BBB, 0x00000001,
    {0x10E410E4, 0x00001000},
    {0x00001840, 0x00000040, 0x00000018, 0x00000002},
    {
        0x004A2195, 0x02424190, 0x0008B060, 0x04B005DC,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00008808, 0x20250000,
        0x00000000, 0x04000800, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000
    },
    0,
    {0x01040802, 0x0000FFFF, 0x01040902, 0x0000FFFF},
    1,
    {
        0xFF040002, 0x0000FFFF, 0xFF040102, 0x0000FFFF,
        0xFF040202, 0x0000FFFF, 0xFF040302, 0x0000FFFF,
        0x00000000, 0x00000000
    },
    {
        0x01020206, 0x0000FF02, 0x01020306, 0x0000FF02,
        0x01020106, 0x0000FF02, 0x01020006, 0x0000FF02,
        0x01020506, 0x0000FF02, 0x01020406, 0x0000FF02,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000
    },
    {
        0x01030503, 0x0000FF03, 0x01030603, 0x0000FF03,
        0x01030803, 0x0000FF03, 0x01030903, 0x0000FF03,
        0x01030A03, 0x0000FF03, 0x01030B03, 0x0000FF03,
        0x01030C03, 0x0000FF03, 0x01030D03, 0x0000FF03,
        0x01030E03, 0x0000FF03, 0x01030F03, 0x0000FF03,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000
    },
    {
        0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF,
        0x00000002, 0xFFFFFFFF, 0x00000001, 0x00000000,
        0x00000001, 0x00000000, 0x00000004, 0x00000000,
        0x00000008, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000
    },
    {
        0x30353430, 0x38613136, 0x35326262, 0x62633038,
        0x30616633, 0x30336532, 0x32356631, 0x35313061,
        0x63303430, 0x66383531, 0x0000000A, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000
    }
};

int main(int argc, char** argv) {
    if(argc != 3) {
        std::cout << "Usage: InputFile OutputFile" << std::endl;
        return 1;
    }

    if(!reader.load(argv[1])) {
        std::cout << "Failed to open file " << argv[1] << std::endl;
        return 2;
    }

    if(reader.get_class() != ELFCLASS32 || reader.get_encoding() != ELFDATA2LSB) {
        std::cout << "32 bit little endian is required" << std::endl;
        return 3;
    }

    output = open(argv[2], O_RDWR|O_CREAT|O_TRUNC, 0666);
    if(output < 0) {
        std::cout << "Failed to open file " << argv[2] << std::endl;
        return 4;
    }

    for(unsigned int i = 0; i < reader.sections.size(); ++i) {
        ELFIO::section* psec = reader.sections[i];
        address = psec->get_address()-SPL_sRAM_Offset-headerLength;
        if(psec->get_type() != SHT_PROGBITS || address < 0)
            continue;

        address += headerLength;
        if(header.length < address+psec->get_size())
            header.length = address+psec->get_size();
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

    header.length = (header.length+blockSize-1)/blockSize*blockSize;
    std::cout << "Length: " << std::dec << header.length << std::endl;

    lseek(output, SPL_SD_Offset, SEEK_SET);
    write(output, (char*)&header, headerLength);
    ftruncate(output, SPL_SD_Offset+header.length);

    lseek(output, SPL_SD_Offset, SEEK_SET);
    header.checkSum = 0;
    for(unsigned int i = 0; i < header.length/4; ++i) {
        read(output, (char*)&buffer, 4);
        header.checkSum += buffer;
    }

    lseek(output, SPL_SD_Offset+12, SEEK_SET);
    write(output, (char*)&header.checkSum, 4);
    close(output);
    std::cout << "Check sum: " << std::hex << header.checkSum << std::endl;

    return 0;
}
