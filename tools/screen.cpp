#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char** argv) {
    if(argc != 2) {
        printf("Expected device argument\n");
        return 1;
    }

    int fd = open(argv[1], O_RDWR);
    if(fd < 0) {
        perror("open");
        return 2;
    }

    struct termios options;
    if(tcgetattr(fd, &options) < 0) {
        perror("tcgetattr");
        return 3;
    }
    cfsetispeed(&options, 115200);
    cfsetospeed(&options, 115200);
    options.c_cflag |= (CLOCAL|CREAD);
    if(tcsetattr(fd, TCSANOW, &options) < 0) {
        perror("tcsetattr");
        return 4;
    }

    char byte;
    size_t size;
    while((size = read(fd, &byte, 1)))
        printf("%c", byte);

    close(fd);
    return 0;
}
