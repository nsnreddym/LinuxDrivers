#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h> 
#include <linux/usb.h> 
#include <linux/slab.h> 
#include <linux/cdev.h> 

#define VENDOR_ID 0x05C6
#define PRODUCT_ID 0x9003

struct ReadSize
{
   char *buffer;
   int *length;
};

struct MyCharDev
{
   /* Char dev data */
   int devstate;
   dev_t dev_no;
   struct cdev c_dev;
   struct class *cls_dev;
   struct device *devptr;  
   
   /* USB dev data */
   struct usb_device *usbdev;
   struct usb_endpoint_descriptor *out_ep;
   struct usb_endpoint_descriptor *in_ep;
   struct usb_interface *interface;
   
   /* Buffers */
   char *txbuf;
   char *rxbuf;
       
};

static void UC20_disconnect(struct usb_interface *intf);
static int UC20_probe(struct usb_interface *interface,
		                const struct usb_device_id *id);
		                
static int Create_dev(char *devname, 
                       struct MyCharDev *dev, 
                       struct file_operations *FileOps);		                

static int gsm_open(struct inode *inode, struct file *filp);
static int gsm_release(struct inode *inode, struct file *filp);
static ssize_t gsm_read(struct file *filp,
                        char *buffer,
                        size_t length,
                        loff_t *offset);
                        
static int gsm_write(struct file *filp, 
                     const char *buf, 
                     size_t len, 
                     loff_t *off);
                     


static struct usb_device_id UC20id_table[] = {
{USB_DEVICE_INTERFACE_NUMBER(VENDOR_ID, PRODUCT_ID, 0)},
{USB_DEVICE_INTERFACE_NUMBER(VENDOR_ID, PRODUCT_ID, 1)},
{USB_DEVICE_INTERFACE_NUMBER(VENDOR_ID, PRODUCT_ID, 2)},
{USB_DEVICE_INTERFACE_NUMBER(VENDOR_ID, PRODUCT_ID, 3)},
{USB_DEVICE_INTERFACE_NUMBER(VENDOR_ID, PRODUCT_ID, 4)},
{ },
};

MODULE_DEVICE_TABLE (usb, UC20id_table);


static struct file_operations gsm_fops = {
  .read = gsm_read,
  .write = gsm_write,
  .open = gsm_open,
  .release = gsm_release
};

static struct usb_driver UC20_usbdriver = {
	.name =		"usbUC20",
	.probe =	UC20_probe,
	.disconnect =	UC20_disconnect,
	.id_table =	UC20id_table,
};

struct MyCharDev GSM_dev;

static char *mybuffer;

static void Urb_bulk_callback(struct urb *urb)
{
   int retval;
   char *buf2;

   if(urb->status)
   {
     printk(KERN_ALERT "URB Sent failed\n");

     return;
   }
}

static void Urb_bulk_callreturn(struct urb *urb)
{
   int retval;
   char *buf2;
   struct ReadSize *RSize;

   if(urb->status)
   {
     printk(KERN_ALERT "URB Sent failed\n");

     return;
   } 
   RSize = urb->context;  
   printk(KERN_INFO "Drv_UC20: Buffer %u\n",RSize->buffer);
   printk(KERN_INFO "Drv_UC20: GSM_dev.rxbuf %s\n",GSM_dev.rxbuf);
   
   /**/   
   if(retval = copy_to_user(RSize->buffer,GSM_dev.rxbuf,urb->actual_length))
   {
      printk(KERN_INFO "Drv_UC20: Copy failed : %d\n",retval);
   }
   printk(KERN_INFO "Drv_UC20: urb->actual_length %d\n",urb->actual_length);
   
   kfree(RSize);
 
}



/* *****************  Usb drivers ************** */
static void UC20_disconnect(struct usb_interface *intf)
{
   device_destroy(GSM_dev.cls_dev, GSM_dev.dev_no);
   class_destroy(GSM_dev.cls_dev);
   unregister_chrdev_region(GSM_dev.dev_no, 1);
   
   /*kfree(GSM_dev.txbuf);
   kfree(GSM_dev.rxbuf);*/
   
   printk(KERN_ALERT "Drv_UC20: disconnected device\n");
}

static int UC20_probe(struct usb_interface *interface,
		                const struct usb_device_id *id)
{
   struct usb_host_interface *cursetting;
   struct usb_endpoint_descriptor *endpoint;
   
   int i;
   int Major;
   
   cursetting = interface->cur_altsetting;
   
   switch(cursetting->desc.bInterfaceNumber)
   {
      case 0:
         break;
      case 1:
         break;
      case 2: //AT Port

         for(i=0; i < cursetting->desc.bNumEndpoints; i++)
         {
            endpoint = &cursetting->endpoint[i].desc;
            
            if(endpoint->bEndpointAddress & 0x80)
            {
               GSM_dev.in_ep = endpoint;               
            }
            else
            {
               GSM_dev.out_ep = endpoint; 
            }
         }
         
         GSM_dev.usbdev = interface_to_usbdev(interface); 
         GSM_dev.interface = interface; 
         
         printk(KERN_INFO "Drv_UC20: GSM device id : %d is in path %s\n",
                GSM_dev.usbdev->devnum,
                GSM_dev.usbdev->devpath);
         
         Major = Create_dev("UC20_GSM",&GSM_dev,&gsm_fops);
         
         if (Major < 0) 
         {
            printk(KERN_ALERT "Drv_UC20: Registering GSM device failed\n");
            return -1;
         }
         printk(KERN_INFO "Drv_UC20: GSM device is attached to 'UC20_GSM'\n");
         
         GSM_dev.txbuf = kmalloc(100,GFP_KERNEL); 
         GSM_dev.rxbuf = GSM_dev.txbuf;//kmalloc(100,GFP_KERNEL); 
      
         break;
      case 3:
         break;
      case 4:
         break;   
   }
   
  return 0; 
   
}

/* ***************** Char Drivers ************** */
static int gsm_open(struct inode *inode, struct file *filp)
{
  printk(KERN_INFO "Open function called\n");
  return 0;
}

static int gsm_release(struct inode *inode, struct file *filp)
{
  printk(KERN_INFO "Release function called\n");
  return 0;
}

static ssize_t gsm_read(struct file *filp,
                        char *buffer,
                        size_t length,
                        loff_t *offset)
{
   struct urb *myurb;
   struct ReadSize *RSize;
   int retval;
   int cnt;
   
   /* Allocate URB memory */
   myurb = usb_alloc_urb(0,GFP_KERNEL);
   if(!myurb)
   {
       printk(KERN_ALERT "Drv_UC20: URB Creating failed\n");

       return -1;
   }

   /* Allocate buffer */
   GSM_dev.rxbuf = usb_alloc_coherent(GSM_dev.usbdev,length,
                                      GFP_KERNEL,&myurb->transfer_dma);   
   
                                                         
   if(!GSM_dev.rxbuf)
   {
       printk(KERN_ALERT "Drv_UC20: Buffer Creating failed\n");

       return -1;
   }
   
   retval = usb_bulk_msg(GSM_dev.usbdev,
                         usb_rcvbulkpipe(GSM_dev.usbdev,3),
                         GSM_dev.rxbuf,length,&cnt, HZ*10);
                         
   if (!retval) 
   {
      if (copy_to_user (buffer, GSM_dev.rxbuf, cnt))
      {
         retval = -EFAULT;
         printk(KERN_ALERT "Drv_UC20: Error copying\n");
      }
      else
      {
         retval = cnt;
      }
   }
   else
   {
      printk(KERN_ALERT "Drv_UC20: Error reading\n");
      retval = -1;
   }
                         
/*
   RSize = kmalloc(sizeof(struct ReadSize), GFP_KERNEL);
   
   printk(KERN_INFO "Drv_UC20: Buffer %u\n",buffer);
   RSize->buffer = buffer;
   RSize->length = &length;
   printk(KERN_INFO "Drv_UC20: Buffer %u\n",RSize->buffer);
      
   usb_fill_bulk_urb(myurb, GSM_dev.usbdev, 
                     usb_rcvbulkpipe(GSM_dev.usbdev,3),
                     GSM_dev.rxbuf,40,Urb_bulk_callreturn,RSize);

   retval = usb_submit_urb(myurb,GFP_KERNEL);
    
   if(retval)
   {
       printk(KERN_ALERT "SUbmitting URB failed\n");

       return -1;
   }*/

   return retval;
}

static int gsm_write(struct file *filp, 
                     const char *buffer, 
                     size_t length, 
                     loff_t *off)
{
   int cnt;   
   int retval;
   struct urb *myurb;
   
   /* Allocate URB memory */
   myurb = usb_alloc_urb(0,GFP_KERNEL);
   if(!myurb)
   {
       printk(KERN_ALERT "Drv_UC20: URB Creating failed\n");

       return -1;
   }
   
   /* Allocate buffer */
   GSM_dev.txbuf = usb_alloc_coherent(GSM_dev.usbdev,length,
                                      GFP_KERNEL,&myurb->transfer_dma);                            
   if(!GSM_dev.txbuf)
   {
       printk(KERN_ALERT "Drv_UC20: Buffer Creating failed\n");

       return -1;
   }
 
   /* Copy memory */
   copy_from_user(GSM_dev.txbuf,buffer,length);
   
   /* Send URB 
   retval = usb_bulk_msg(GSM_dev.usbdev,
                         usb_sndbulkpipe(GSM_dev.usbdev,
                                         GSM_dev.out_ep->bEndpointAddress),
                         GSM_dev.txbuf,length,&cnt, HZ*10);
   */
   usb_fill_bulk_urb(myurb, GSM_dev.usbdev, 
                     usb_sndbulkpipe(GSM_dev.usbdev,
                                     GSM_dev.out_ep->bEndpointAddress),
                     GSM_dev.txbuf,length,Urb_bulk_callback,&cnt);

   myurb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP | URB_ZERO_PACKET;

   retval = usb_submit_urb(myurb,GFP_KERNEL);
   
   if(retval)
   {
       printk(KERN_ALERT "SUbmitting URB failed\n");

       return -1;
   }

   return 0;
}

static int Create_dev(char *devname, 
                       struct MyCharDev *dev, 
                       struct file_operations *FileOps)
{
   int err;
   
   /* Allocate device no */
	err = alloc_chrdev_region(&dev->dev_no, 0, 1, devname);
	if (err < 0) {
		printk(KERN_WARNING "Drv_UC20: alloc_chrdev_region() failed\n");
		return err;
	}
	
	/* Create class */
	dev->cls_dev = class_create(THIS_MODULE, devname);
	if (dev->cls_dev == NULL) {
		printk(KERN_WARNING "Drv_UC20: class_create() failed\n");
		return -1;
	}
	
	/* Create device */
	dev->devptr = device_create(dev->cls_dev, NULL, dev->dev_no, NULL, devname);
	if (dev->devptr == NULL) 
	{
	   class_destroy(dev->cls_dev);
	   unregister_chrdev_region(dev->dev_no, 1);
	   
		printk(KERN_WARNING "Drv_UC20: cdev_add() failed\n");
		return -1;
	}
	
	/* Initialize Char dev */
	cdev_init(&dev->c_dev, FileOps);
	
	/* add node */
	err = cdev_add(&dev->c_dev,dev->dev_no,1);
	if (err < 0) 
	{
	   device_destroy(dev->cls_dev, dev->dev_no);
	   class_destroy(dev->cls_dev);
	   unregister_chrdev_region(dev->dev_no, 1);
	   
		printk(KERN_WARNING "Drv_UC20: cdev_add() failed\n");
		return -1;
	}
	
	return 0;

}

/* ***************** Module Code *************** */

int init_module(void);
void cleanup_module(void);

int init_module(void)
{
   int retval;
   
   /* Register USB */ 
   retval = usb_register(&UC20_usbdriver);   
   
   if(retval) 
   {
      printk(KERN_ALERT "Drv_UC20: Registering USB device failed\n");
      return -1;
   }
   
   mybuffer = kmalloc(100,GFP_KERNEL);
   
   /* Creating GSM device
   Major = Create_dev("UC20_GPS",&GSM_dev,&gsm_fops);
   
   if (Major < 0) 
   {
      printk(KERN_ALERT "Drv_UC20: Registering char device failed\n");
      return -1;
   }
   printk(KERN_INFO "Drv_UC20: Registering char device with %d:%d\n",
          MAJOR(GSM_dev.dev_no), MINOR(GSM_dev.dev_no)); 
          
   mybuffer = kmalloc(100,GFP_KERNEL);   
   
    */
   return 0;
}

void cleanup_module(void)
{   
   usb_deregister(&UC20_usbdriver);
   
   printk(KERN_INFO "Drv_UC20: Deregistering USB device\n"); 
   
}

MODULE_LICENSE("GPL");

