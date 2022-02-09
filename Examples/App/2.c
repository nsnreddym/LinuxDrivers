#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
       #include <sys/types.h>
       #include <sys/stat.h>
       #include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

/*------------------------------------------------*/
int getch(void) {
      int c=0;

      struct termios org_opts, new_opts;
      int res=0;
          //-----  store old settings -----------
      res=tcgetattr(STDIN_FILENO, &org_opts);
      assert(res==0);
          //---- set new terminal parms --------
      memcpy(&new_opts, &org_opts, sizeof(new_opts));
      new_opts.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ECHOPRT | ECHOKE | ICRNL);
      tcsetattr(STDIN_FILENO, TCSANOW, &new_opts);
      c=getchar();
          //------  restore old settings ---------
      res=tcsetattr(STDIN_FILENO, TCSANOW, &org_opts);
      assert(res==0);
      return(c);
}

int main(void)
{
    int retval;
int fd;
    char ch;

    //system ("/bin/stty raw");

    retval = mkfifo("./myfifo", 0777);

    if(retval == 0)
    {
       printf("FIFO created\n");
    }
    else
    {
       printf("Error in fifo\n");
       return -1;
    }
 
    fd = open("./myfifo", O_WRONLY);
    
    while('q' != (ch = getch()))
    {
      //write(fd,"Hello",5);
      putchar(ch);
      fd = open("./myfifo", O_WRONLY|O_NONBLOCK);
      //printf("Data Read: %d \'%c\'\n",ch,ch);
      write(fd,&ch,1);
//putchar(ch);
      //printf("Data Sent: %d \'%c\'\n",ch,ch);
      close(fd);
   }

      fd = open("./myfifo", O_WRONLY|O_NONBLOCK);
      //printf("Data Read: %d \'%c\'\n",ch,ch);
      write(fd,&ch,1);
      //printf("Data Sent: %d \'%c\'\n",ch,ch);
      close(fd);
    printf("Data sent\n");

//system ("/bin/stty cooked");
    return 0;
}
