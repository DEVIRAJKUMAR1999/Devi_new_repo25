#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/mutex.h>
#include <linux/sched.h>


// ********************define macros and data structures*****************

#define DEVICE_NAME "vicharak"
#define CLASS_NAME "DRK_char_class"

#define SET_SIZE_OF_QUEUE _IOW('a', 'a', int32_t*)
#define PUSH_DATA _IOW('a', 'b', struct data*)
#define POP_DATA _IOR('a', 'c', struct data*)

struct data {
    int32_t length;
    char *data;
};

// **************************global variables*************************** 

static dev_t dev_num;
static struct cdev c_dev;
static struct class *cls;

static char **queue = NULL;
static int32_t queue_size = 0;
static int32_t front = -1;
static int32_t rear = -1;

static DEFINE_MUTEX(queue_mutex);
static DECLARE_WAIT_QUEUE_HEAD(queue_wait);


//***************************initialize queue****************************

static int init_queue(int32_t size) {
    int i;
    
    mutex_lock(&queue_mutex);
    if (queue != NULL) {
        // Clear existing queue
        for (i = 0; i < queue_size; i++) {
            kfree(queue[i]);
        }
        kfree(queue);
    }
    // Initialize new queue
    queue_size = size;
    queue = kmalloc(sizeof(char*) * queue_size, GFP_KERNEL);
    if (!queue) {
        mutex_unlock(&queue_mutex);
        printk(KERN_ALERT "Failed to allocate memory for queue\n");
        return -ENOMEM;
    }
    for (i = 0; i < queue_size; i++) {
        queue[i] = NULL;
    }
    front = rear = -1;
    mutex_unlock(&queue_mutex);
    return 0;
}

//************************check if queue is_full/empty*********************


static int is_full(void) {
    return ((front == 0 && rear == queue_size - 1) || (rear == (front - 1) % (queue_size -1)));
}

static int is_empty(void) {
    return (front == -1);
}

//****************************enqueue data*****************************


static int enqueue(char *data) {
    if (is_full()) {
        printk(KERN_INFO "Queue is full\n");
        return -1;
    }
    if (front == -1) { // Insert First Element
        front = rear = 0;
    } else if (rear == queue_size -1 && front != 0) {
        rear = 0;
    } else {
        rear++;
    }
    queue[rear] = data;
    return 0;
}

//****************************dequeue data*****************************

static char* dequeue(void) {
    char *data;
    if (is_empty()) {
        printk(KERN_INFO "Queue is empty\n");
        return NULL;
    }
    data = queue[front];
    queue[front] = NULL;
    if (front == rear) {
        front = rear = -1;
    } else if (front == queue_size - 1) {
        front = 0;
    } else {
        front++;
    }
    return data;
}

//****************************IOCTL function*****************************

static long device_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    int32_t size;
    struct data user_data;
    char *kernel_data;
    int ret;
    
    switch (cmd) {
        case SET_SIZE_OF_QUEUE:
            if (copy_from_user(&size, (int32_t*)arg, sizeof(int32_t))) {
                return -EFAULT;
            }
            ret = init_queue(size);
            return ret;
        
        case PUSH_DATA:
            if (copy_from_user(&user_data, (struct data*)arg, sizeof(struct data))) {
                return -EFAULT;
            }
            kernel_data = kmalloc(user_data.length + 1, GFP_KERNEL);
            if (!kernel_data) {
                return -ENOMEM;
            }
            if (copy_from_user(kernel_data, user_data.data, user_data.length)) {
                kfree(kernel_data);
                return -EFAULT;
            }
            kernel_data[user_data.length] = '\0'; // Null-terminate
            mutex_lock(&queue_mutex);
            while (is_full()) {
                mutex_unlock(&queue_mutex);
                if (wait_event_interruptible(queue_wait, !is_full())) {
                    kfree(kernel_data);
                    return -ERESTARTSYS;
                }
                mutex_lock(&queue_mutex);
            }
            enqueue(kernel_data);
            mutex_unlock(&queue_mutex);
            wake_up_interruptible(&queue_wait);
            return 0;
        
        case POP_DATA:
            mutex_lock(&queue_mutex);
            while (is_empty()) {
                mutex_unlock(&queue_mutex);
                if (wait_event_interruptible(queue_wait, !is_empty())) {
                    return -ERESTARTSYS;
                }
                mutex_lock(&queue_mutex);
            }
            kernel_data = dequeue();
            mutex_unlock(&queue_mutex);
            wake_up_interruptible(&queue_wait);
            user_data.length = strlen(kernel_data);
            if (copy_to_user(((struct data*)arg)->data, kernel_data, user_data.length)) {
                kfree(kernel_data);
                return -EFAULT;
            }
            if (copy_to_user(&((struct data*)arg)->length, &user_data.length, sizeof(int32_t))) {
                kfree(kernel_data);
                return -EFAULT;
            }
            kfree(kernel_data);
            return 0;
        
        default:
            return -EINVAL;
    }
    return 0;
}

//****************************file operations struct **********************

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = device_ioctl,
};

//******************************module init *****************************

static int __init my_char_init(void) {
    int ret;
    
    // Allocate device numbers
    if ((ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME)) < 0) {
        printk(KERN_ALERT "Failed to allocate device numbers\n");
        return ret;
    }
    
    // Create device class
    if (IS_ERR(cls = class_create(THIS_MODULE, CLASS_NAME))) {
        unregister_chrdev_region(dev_num, 1);
        printk(KERN_ALERT "Failed to create device class\n");
        return PTR_ERR(cls);
    }
    
    // Create device node
    if (IS_ERR(device_create(cls, NULL, dev_num, NULL, DEVICE_NAME))) {
        class_destroy(cls);
        unregister_chrdev_region(dev_num, 1);
        printk(KERN_ALERT "Failed to create device\n");
        return -1;
    }
    
    // Initialize and add cdev
    cdev_init(&c_dev, &fops);
    if ((ret = cdev_add(&c_dev, dev_num, 1)) < 0) {
        device_destroy(cls, dev_num);
        class_destroy(cls);
        unregister_chrdev_region(dev_num, 1);
        printk(KERN_ALERT "Failed to add cdev\n");
        return ret;
    }
    
    mutex_init(&queue_mutex);
    printk(KERN_INFO "vicharak device initialized\n");
    return 0;
}


//****************************DRK_char_exit*****************************

static void __exit my_char_exit(void) {
    int i;
    // Clean up queue
    mutex_lock(&queue_mutex);
    if (queue != NULL) {
        for (i = 0; i < queue_size; i++) {
            kfree(queue[i]);
        }
        kfree(queue);
    }
    mutex_unlock(&queue_mutex);
    cdev_del(&c_dev);
    device_destroy(cls, dev_num);
    class_destroy(cls);
    unregister_chrdev_region(dev_num, 1);
    printk(KERN_INFO "vicharak device exited\n");
}

//*************************module_data **********************************

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Devi");
MODULE_DESCRIPTION("Dynamic circular queue character device driver");
MODULE_VERSION("1.0");

module_init(my_char_init);
module_exit(my_char_exit);
 
