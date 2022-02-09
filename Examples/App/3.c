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
void putch(int c) {
      //int c=0;

      struct termios org_opts, new_opts;
      int res=0;
          //-----  store old settings -----------
      res=tcgetattr(STDIN_FILENO, &org_opts);
      assert(res==0);
          //---- set new terminal parms --------
      memcpy(&new_opts, &org_opts, sizeof(new_opts));
      new_opts.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ECHOPRT | ECHOKE | ICRNL);
      tcsetattr(STDIN_FILENO, TCSANOW, &new_opts);
      putchar(c);
          //------  restore old settings ---------
      res=tcsetattr(STDIN_FILENO, TCSANOW, &org_opts);
      assert(res==0);
      //return(c);
}


int main(void)
{
    int retval;
int fd;
    char mes;

  //system ("/bin/stty raw");
  fd = open("./myfifo", O_RDONLY|O_NONBLOCK);  
    
   while(mes != 'q')
{
    retval = read(fd,&mes,1);
    

    if(retval > 0)
    {


    //printf("%c",mes);
putchar(mes);
fflush(stdout);

    }
    
}
close(fd);
//system ("/bin/stty cooked");    
return 0;
}
