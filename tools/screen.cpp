#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>


int main() {
    int fd = open("/dev/cu.usbserial-A900cccV", O_RDWR);
    if(fd < 0) {
        perror("open");
        return 1;
    }

    struct termios options;
    if(tcgetattr(fd, &options) < 0) {
        perror("tcgetattr");
        return 1;
    }
    cfsetispeed(&options, 115200);
    cfsetospeed(&options, 115200);
    options.c_cflag |= (CLOCAL|CREAD);
    if(tcsetattr(fd, TCSANOW, &options) < 0) {
        perror("tcsetattr");
        return 1;
    }

    while(1) {
        char byte;
        ssize_t size = read(fd, &byte, 1);
        printf("%c", byte);
    }

    close(fd);
    return 0;
}
