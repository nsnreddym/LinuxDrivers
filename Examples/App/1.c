#include <stdio.h>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>    
int main() {
    char byte;
    char rbyte;
    int fd = open("/dev/ttyU0", O_RDWR);
printf("Pointer = %u\n",fd);
    write(fd, "This is Satya..", 15);
    printf("Data written\n");

    byte = 0;

    //while(byte != 'q')
{
    /*byte = getc(stdin);
    write(fd,&byte,1);*/
    read(fd,&rbyte,1);
    putc(rbyte,stdout);
    read(fd,&rbyte,1);
    putc(rbyte,stdout);
    read(fd,&rbyte,1);
    putc(rbyte,stdout);
    read(fd,&rbyte,1);
    putc(rbyte,stdout);
    read(fd,&rbyte,1);
    putc(rbyte,stdout);
}

    return 0;
}
