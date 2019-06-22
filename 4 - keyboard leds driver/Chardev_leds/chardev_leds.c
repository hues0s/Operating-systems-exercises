/* 
		DUDAS


TODO usar init_module() y cleanup_module(), o las macros __init y __exit????
*/



#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>	/* for copy_to_user */
#include <linux/cdev.h>

#include <asm-generic/errno.h>
#include <linux/init.h>
#include <linux/tty.h>      /* For fg_console */
#include <linux/kd.h>       /* For KDSETLED */
#include <linux/vt_kern.h>
#include <linux/version.h> /* For LINUX_VERSION_CODE */


/*
 *  Prototypes
 */

int init_module(void);
void cleanup_module(void);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "leds" /* Dev name as it appears in /proc/devices */
#define BUF_LEN 50		/* Longitud maxima del string de enteros, INCLUYENDO 1 PARA EL \N Y 49 PARA NUMEROS */

static struct file_operations fops = {
    .write = device_write
};

struct tty_driver* kbd_driver= NULL; //Almacena el driver del teclado

dev_t start;
struct cdev* chardev=NULL;



/* Get driver handler */
struct tty_driver* get_kbd_driver_handler(void)
{
#if ( LINUX_VERSION_CODE > KERNEL_VERSION(2,6,32) )
    return vc_cons[fg_console].d->port.tty->driver;
#else
    return vc_cons[fg_console].d->vc_tty->driver;
#endif
}

/* Set led state to that specified by mask */
static inline int set_leds(struct tty_driver* handler, unsigned int mask)
{
#if ( LINUX_VERSION_CODE > KERNEL_VERSION(2,6,32) )
    return (handler->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED,mask);
#else
    return (handler->ops->ioctl) (vc_cons[fg_console].d->vc_tty, NULL, KDSETLED, mask);
#endif
}


/*
 * This function is called when the module is loaded
 */
int init_module(void){

	int major;		/* Major number assigned to our device driver */
    int minor;		/* Minor number assigned to the associated character device */
    int ret;

    /* Get available (major,minor) range */
    if ((ret=alloc_chrdev_region (&start, 0, 1,DEVICE_NAME))) {
        printk(KERN_INFO "Can't allocate chrdev_region()");
        return ret;
    }

    /* Create associated cdev */
    if ((chardev=cdev_alloc())==NULL) {
        printk(KERN_INFO "cdev_alloc() failed ");
        unregister_chrdev_region(start, 1);
        return -ENOMEM;
    }

    cdev_init(chardev,&fops);

    if ((ret=cdev_add(chardev,start,1))) {
        printk(KERN_INFO "cdev_add() failed ");
        kobject_put(&chardev->kobj);
        unregister_chrdev_region(start, 1);
        return ret;
    }

    major=MAJOR(start);
    minor=MINOR(start);

    printk(KERN_INFO "I was assigned major number %d. To talk to\n", major);
    printk(KERN_INFO "the driver, create a dev file with\n");
    printk(KERN_INFO "'sudo mknod -m 666 /dev/%s c %d %d'.\n", DEVICE_NAME, major,minor);
    printk(KERN_INFO "Remove the device file and module when done.\n");

    kbd_driver= get_kbd_driver_handler(); //Inicializamos el driver del teclado

    return SUCCESS;
}

/*
 * This function is called when the module is unloaded
 */
void cleanup_module(void){

    /* Destroy chardev */
    if (chardev)
        cdev_del(chardev);

    /*
     * Unregister the device
     */
    unregister_chrdev_region(start, 1);
}

/*
 * Called when a process writes to dev file: echo "hi" > /dev/chardev

 	Devuelve el numero de bytes leidos de buff
 */
static ssize_t
device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{

    unsigned int maskPropia = 0; //Aqui acumularemos la mascara de bits
    //int numeroParcial; //En cada iteracion leeremos una posicion de kbuf
    bool checkIfHasAppearedBefore[3] = {false, false, false}; //Array para ver si el nº ya se ha leido antes
    //unsigned int numerosRestantesPorLeer = 0; //Como no se permite usar bucles for, lo haremos manualmente
    char kbuf[BUF_LEN]; //len es el numero de bytes que ocupa el array buff
                    //buff es un array de char que almacena lo escrito por el usuario
    int i = 0;

    set_leds(kbd_driver,0); //Apagamos los leds por si hubiera algun error al escribir, para que no queden encendidos.

    //Comprobamos que no superamos el tamaño de buffer, para eviar desbordamientos en la pila del kernel.
    if(len >= BUF_LEN) return -ENOSPC;

	if(copy_from_user(kbuf,buff,len) > 0){ //Esta funcion devuelve el nº de bytes que NO SE HAN COPIADO.
		return -EFAULT;
	}

    kbuf[len]='\0';

    11111111111111111111111111111111111111111111111111

	//Ahora procesamos los datos leidos, utilizando siempre el array local KBUF, puesto
	//que no podemos confiar en los punteros a espacio de usuario (ej. *buff)

	for(; i < strlen(kbuf); ++i){

        switch(kbuf[i]){
            case '1':
            if(!checkIfHasAppearedBefore[0]){
                maskPropia += 2;
                checkIfHasAppearedBefore[0] = true;
            }
            break;
            case '2':
            if(!checkIfHasAppearedBefore[1]){
                maskPropia += 4;
                checkIfHasAppearedBefore[1] = true;
            }
            break;
            case '3':
            if(!checkIfHasAppearedBefore[2]){
                maskPropia += 1;
                checkIfHasAppearedBefore[2] = true;
            }
            break;
            case '\n':
            break;
            default:
            return -EINVAL;
            break;
        }


/*
		numeroParcial = kbuf[i] - '0';
		if(!checkIfHasAppearedBefore[numeroParcial - 1]){ //Si esa casilla esta a false, el numero aun no se habia leido.
			checkIfHasAppearedBefore[numeroParcial - 1] = true;
			if(numeroParcial == 1) maskPropia += 2;
			else if(numeroParcial == 2) maskPropia += 4;
			else if(numeroParcial == 3) maskPropia += 1;
		}
*/
	}

	set_leds(kbd_driver,maskPropia);

	return len; //Si llega hasta aqui quiere decir que la funcion copy_from_user ha leido todo correctamente
				//por lo que write devuelve el tamaño de bytes del buffer
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Modleds");