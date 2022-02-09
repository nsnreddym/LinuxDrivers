
#include <linux/init.h>
#include <linux/module.h>
#include <linux/tty_driver.h>
#include <linux/tty.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/tty_flip.h>

#define TINY_TTY_MINORS   1
#define TINY_TTY_MAJOR    240

struct tiny_serial {
	struct tty_struct	*tty;		/* pointer to the tty for this device */
	int			open_count;	/* number of times this port has been opened */
	struct semaphore	sem;		/* locks this structure */
	struct timer_list	*timer;
};


static struct tty_driver *tiny_tty_driver;
static struct device *retval;

static struct tiny_serial *tiny_table[TINY_TTY_MINORS];	/* initially all NULL */


static int tiny_open(struct tty_struct *tty, struct file *file)
{
    struct tiny_serial *tiny;
    struct timer_list *timer;
    int index;

    /* initialize the pointer in case something fails */
    tty->driver_data = NULL;

    /* get the serial object associated with this tty pointer */
    index = tty->index;
    tiny = tiny_table[index];

    if (tiny == NULL) {

        printk(KERN_INFO "Sample: Entered into Open\n");
        /* first time accessing this device, let's create it */
        tiny = kmalloc(sizeof(*tiny), GFP_KERNEL);
        if (!tiny)
            return -ENOMEM;

        printk(KERN_INFO "Sample: Pointer is %p",tiny);
        //init_MUTEX(&tiny->sem);
        tiny->open_count = 0;
        tiny->timer = NULL;
        //tiny_table[index] = tiny;
    }

    //down(&tiny->sem);

    /* save our structure within the tty structure */
    tty->driver_data = tiny;
    tiny->tty = tty;

printk(KERN_INFO "Sample: Exit into Open\n");

    return 0;
}

void tiny_close(struct tty_struct * tty, struct file * filp)
{
   struct tiny_serial *tiny = tty->driver_data;

   if (tiny)
   {
       //del_timer(tiny->timer);
   }
}

int  tiny_write(struct tty_struct * tty,
		      const unsigned char *buf, int count)
{
    struct tiny_serial *tiny = tty->driver_data;
    int i;
    int retval = -EINVAL;

    printk(KERN_DEBUG "Sample: %s - ", __FUNCTION__);

    if (!tiny)
        return -ENODEV;

    printk(KERN_DEBUG "%s - ", __FUNCTION__);
    for (i = 0; i < count; ++i)
        printk("%02x ", buf[i]);
    printk("\n");

        tty->port->low_latency = 1;
        tty_insert_flip_char(tty->port,'S',0);
        tty_flip_buffer_push(tty->port);
        tty_insert_flip_char(tty->port,'a',TTYB_NORMAL);
        tty_flip_buffer_push(tty->port);
        tty_insert_flip_char(tty->port,'t',TTYB_NORMAL);
        tty_flip_buffer_push(tty->port);
        tty_insert_flip_char(tty->port,'y',TTYB_NORMAL);
        tty_flip_buffer_push(tty->port);
        tty_insert_flip_char(tty->port,'a',TTYB_NORMAL);
        tty_flip_buffer_push(tty->port);
        tty_insert_flip_char(tty->port,'\n',TTYB_NORMAL);
        tty_flip_buffer_push(tty->port);


    return retval;
}

int  tiny_write_room(struct tty_struct *tty)
{
   printk(KERN_INFO "Simple-driver: Write room called");
}

void tiny_set_termios(struct tty_struct *tty, struct ktermios * old)
{
   printk(KERN_INFO "Simple-driver: set termios  called");
}
 
static int tiny_read_proc(char *page, char **start, off_t off, int count,
                          int *eof, void *data)
{
   struct tiny_serial *tiny;
   off_t begin = 0;
   int length = 0;
   int i;

   length += sprintf(page, "tinyserinfo:1.0 driver:%s\n", "1.0");

   for (i = 0; i < TINY_TTY_MINORS && length < PAGE_SIZE; ++i) 
   {
     tiny = tiny_table[i];
     if (tiny == NULL)
       continue;

     length += sprintf(page+length, "%d\n", i);

     if ((length + begin) > (off + count))
       goto done;

     if ((length + begin) < off) 
     {
       begin += length;
       length = 0;
     }
   }
  
   *eof = 1;
done:
   if (off >= (length + begin))
      return 0;

   *start = page + (off-begin);

   return (count < begin+length-off) ? count : begin + length-off;
}

   static struct tty_operations serial_ops = 
   {
    .open = tiny_open,
    .close = tiny_close,
    .write = tiny_write,
    .write_room = tiny_write_room,
    .set_termios = tiny_set_termios,
    //.read_proc = tiny_read_proc,
   };

static struct tty_port tiny_tty_port[TINY_TTY_MINORS];
  
static int __init my_init(void)
{
   unsigned int i;
   

   tiny_tty_driver = alloc_tty_driver(TINY_TTY_MINORS);
   
   if (!tiny_tty_driver)
     return -ENOMEM;


    /* initialize the tty driver */
    tiny_tty_driver->owner = THIS_MODULE;
    tiny_tty_driver->driver_name = "tiny_tty";
    tiny_tty_driver->name = "ttyU";
    //tiny_tty_driver->devfs_name = "tts/ttty%d";
    tiny_tty_driver->major = TINY_TTY_MAJOR,
    tiny_tty_driver->type = TTY_DRIVER_TYPE_SERIAL,
    tiny_tty_driver->subtype = SERIAL_TYPE_NORMAL,
    tiny_tty_driver->flags = TTY_DRIVER_REAL_RAW ,
    tiny_tty_driver->init_termios = tty_std_termios;
    tiny_tty_driver->init_termios.c_cflag = B9600 | CS8 | CREAD | HUPCL | CLOCAL;
    tty_set_operations(tiny_tty_driver, &serial_ops);

    for (i = 0; i < TINY_TTY_MINORS; i++) 
    {
		tty_port_init(tiny_tty_port + i);
		tty_port_link_device(tiny_tty_port + i, tiny_tty_driver, i);
    }

    /* register the tty driver */
    retval = tty_register_driver(tiny_tty_driver);
    if (retval) 
    {
      printk(KERN_ERR "Simple-driver: failed to register tiny tty driver");
      put_tty_driver(tiny_tty_driver);
      return retval;
    }

    for (i = 0; i < TINY_TTY_MINORS; ++i)
    {
      tty_register_device(tiny_tty_driver, i, NULL);
    }
   
    struct termios tty_std_termios = 
    {
    .c_iflag = ICRNL | IXON,
    .c_oflag = OPOST | ONLCR,
    .c_cflag = B38400 | CS8 | CREAD | HUPCL,
    .c_lflag = ISIG | ICANON | ECHO | ECHOE | ECHOK |
               ECHOCTL | ECHOKE | IEXTEN,
    .c_cc = INIT_C_CC
    };   

   printk( KERN_INFO "Simple-driver: testing is called" );
   
   return  0;
}
   
static void __exit my_exit(void)
{
   unsigned int i;

   for (i = 0; i < TINY_TTY_MINORS; ++i)
   {
     tty_unregister_device(tiny_tty_driver, i);
   }
   
   tty_unregister_driver(tiny_tty_driver);
   
   printk( KERN_INFO "Simple-driver: my_exit() is called" );
                       return;
}
   
module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("N S N Reddy M <nsnreddym@gmail.com>");
MODULE_DESCRIPTION("Our First Driver");
