#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h> 
#include <linux/usb.h> 

int init_module(void);
void cleanup_module(void);

#define SUCCESS 0
#define DEVICE_NAME "chardev"
#define BUF_LEN 80

//GSM
#define VENDOR_ID 0x05C6
#define PRODUCT_ID 0x9003

/* //FTDI
#define VENDOR_ID 0x0403
#define PRODUCT_ID 0x6014*/

static struct usb_device_id id_table [] = {
/*{ USB_DEVICE(VENDOR_ID, PRODUCT_ID) },*/
{USB_DEVICE_INTERFACE_NUMBER(VENDOR_ID, PRODUCT_ID, 0)},
{USB_DEVICE_INTERFACE_NUMBER(VENDOR_ID, PRODUCT_ID, 1)},
{USB_DEVICE_INTERFACE_NUMBER(VENDOR_ID, PRODUCT_ID, 2)},
{USB_DEVICE_INTERFACE_NUMBER(VENDOR_ID, PRODUCT_ID, 3)},
{USB_DEVICE_INTERFACE_NUMBER(VENDOR_ID, PRODUCT_ID, 4)},
{ },

};
/*	
	{ },
};
*/

MODULE_DEVICE_TABLE (usb, id_table);

static struct usb_device *device;


static int led_probe(struct usb_interface *interface,
		      const struct usb_device_id *id)
{

/* Initialize our local device structure 
dev = kmalloc(sizeof(struct usb_led), GFP_KERNEL);
memset (dev, 0x00, sizeof (*dev));

dev->udev = usb_get_dev(udev);
usb_set_intfdata (interface, dev);*/

/* Create our three sysfs files in the USB
* device directory 
device_create_file(&interface->dev, &dev_attr_blue);
device_create_file(&interface->dev, &dev_attr_red);
device_create_file(&interface->dev, &dev_attr_green);

dev_info(&interface->dev,
    "USB LED device now attached\n");*/

   printk(KERN_ALERT "USBModule: probe device\n");

        struct usb_host_interface *iface_desc;
	struct usb_endpoint_descriptor *endpoint;
	int i;

	iface_desc = interface->cur_altsetting;
	printk(KERN_INFO "Pen i/f %d now probed: (%04X:%04X)\n",
			iface_desc->desc.bInterfaceNumber,
			id->idVendor, id->idProduct);
	printk(KERN_INFO "ID->bNumEndpoints: %02X\n",
			iface_desc->desc.bNumEndpoints);
	printk(KERN_INFO "ID->bInterfaceClass: %02X\n",
			iface_desc->desc.bInterfaceClass);

	for (i = 0; i < iface_desc->desc.bNumEndpoints; i++)
	{
		endpoint = &iface_desc->endpoint[i].desc;

		printk(KERN_INFO "ED[%d]->bEndpointAddress: 0x%02X\n",
				i, endpoint->bEndpointAddress);
		printk(KERN_INFO "ED[%d]->bmAttributes: 0x%02X\n",
				i, endpoint->bmAttributes);
		printk(KERN_INFO "ED[%d]->wMaxPacketSize: 0x%04X (%d)\n",
				i, endpoint->wMaxPacketSize,
				endpoint->wMaxPacketSize);
	}

	device = interface_to_usbdev(interface);

        printk(KERN_INFO "USBModule: Device created...\n");

return 0;
}

static void led_disconnect(struct usb_interface *intf)
{
   printk(KERN_ALERT "USBModule: disconnected device\n");

}

static struct usb_driver led_driver = {
	//.owner =	THIS_MODULE,
	.name =		"usbled",
	.probe =	led_probe,
	.disconnect =	led_disconnect,
	.id_table =	id_table,
};

int init_module(void)
{
   int retval;
   printk(KERN_ALERT "USBModule: Loaded driver\n");

/**/
retval = usb_register(&led_driver);
if (retval)
	printk(KERN_ALERT "usb_register failed. "
	    "Error number %d", retval);

   return SUCCESS;
}

void cleanup_module(void)
{
   usb_deregister(&led_driver);
   
   printk(KERN_ALERT "USBModule: Unloaded driver\n");
}
MODULE_LICENSE("GPL");
