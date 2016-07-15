#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <elfio/elfio.hpp>

ELFIO::elfio reader;

int output, address, length = 0;
unsigned int buffer, checkSum = 0;
const unsigned int
    SPL_sRAM_Offset = 0x10000,
    SPL_SD_Offset = 0x2000,
    headerLength = 0x30,
    blockSize = 512,
    jumpInstruction = 0xEA00000A,
    checkSumSample = 0x5F0A6C39;

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

    output = open(argv[2], O_RDWR|O_TRUNC|O_CREAT, 0666);
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
        if(length < address+psec->get_size())
            length = address+psec->get_size();
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

    length = (length+blockSize-1)/blockSize*blockSize;
    std::cout << "Length: " << std::dec << length << std::endl;

    lseek(output, SPL_SD_Offset, SEEK_SET);
    write(output, (char*)&jumpInstruction, 4);
    write(output, "eGON.BT0", 8);
    write(output, (char*)&checkSumSample, 4);
    write(output, (char*)&length, 4);
    ftruncate(output, SPL_SD_Offset+length);

    lseek(output, SPL_SD_Offset, SEEK_SET);
	for(unsigned int i = 0; i < length/4; ++i) {
        read(output, (char*)&buffer, 4);
        checkSum += buffer;
    }

    lseek(output, SPL_SD_Offset+12, SEEK_SET);
    write(output, (char*)&checkSum, 4);
    close(output);

    return 0;
}
