#include <linux/init.h>           
#include <linux/module.h>         
#include <linux/device.h>         
#include <linux/kernel.h>        
#include <linux/fs.h>             
#include <linux/uaccess.h>        
#include <linux/sched/signal.h>
#include <asm/uaccess.h>


#define DEVICE_NAME "process_list"
#define CLASS_NAME  "process_class"


MODULE_LICENSE("Dual BSD/GPL");      
MODULE_AUTHOR("ngupta4");   
MODULE_DESCRIPTION("Process List");  
MODULE_VERSION("0.1");           
 
static int    majorNumber;           
char buffer[100000];
char *ap = buffer;  
static struct class*  proListClass  = NULL; 
static struct device* proListDevice = NULL; 
struct task_struct *task;
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
 
static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .release = dev_release,
};

static int __init proList_init(void){
   printk(KERN_INFO "ProcessList: Initializing the ProcessList LKM\n");
 
   majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
   if (majorNumber<0){
      printk(KERN_ALERT "ProcessList failed to register a major number\n");
      return majorNumber;
   }
   printk(KERN_INFO "ProcessList: registered correctly with major number %d\n", majorNumber);
 
   proListClass = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(proListClass)){                
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to register device class\n");
      return PTR_ERR(proListClass);       
   }
   printk(KERN_INFO "ProcessList: device class registered correctly\n");
 
   proListDevice = device_create(proListClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
   if (IS_ERR(proListDevice)){            
      class_destroy(proListClass);        
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to create the device\n");
      return PTR_ERR(proListDevice);
   }
   printk(KERN_INFO "ProcessList: device class created correctly\n");
   return 0;
}
 
static void __exit proList_exit(void){
   device_destroy(proListClass, MKDEV(majorNumber, 0));     
   class_unregister(proListClass);                         
   class_destroy(proListClass);                            
   unregister_chrdev(majorNumber, DEVICE_NAME);          
   printk(KERN_INFO "ProcessList: Goodbye from the LKM!\n");
}
 
static int dev_open(struct inode *inodep, struct file *filep){
   //numberOpens++;
   printk(KERN_INFO "ProcessList: Device has been opened\n");
   return 0;
}

char * get_task_state(long state)
{
    switch (state) {
        case TASK_RUNNING:
            return "TASK_RUNNING";
        case TASK_INTERRUPTIBLE:
            return "TASK_INTERRUPTIBLE";
        case TASK_UNINTERRUPTIBLE:
            return "TASK_UNINTERRUPTIBLE";
        case __TASK_STOPPED:
            return "__TASK_STOPPED";
        case __TASK_TRACED:
            return "__TASK_TRACED";
	case EXIT_DEAD:
	    return "EXIT_DEAD";
	case EXIT_ZOMBIE:
	    return "EXIT_ZOMBIE";
	case TASK_PARKED:
	    return "TASK_PARKED";
	case TASK_DEAD:
	    return "TASK_DEAD";
	case TASK_WAKEKILL:
	    return "TASK_WAKEKILL";
	case TASK_WAKING:
	    return "TASK_WAKING";
	case TASK_NOLOAD:
	    return "TASK_NOLOAD";
	case TASK_NEW:
	    return "TASK_NEW";
	case TASK_STATE_MAX:
	    return "TASK_STATE_MAX";
	default:
	    return NULL;
    }
}

static ssize_t dev_read(struct file *filep, char *ubuffer, size_t len, loff_t *offset){
   int errorCount = 0;
	const char* stateName = NULL;
	char * state_name = NULL;
	int sizeBuffer = 0;
	printk(KERN_INFO "PROLIST: Reading from device\n");
	for_each_process(task){
		struct task_struct *parent = task->parent;
    		pid_t parent_pid = parent -> pid; 
		unsigned int state = task -> state;
		state_name = get_task_state(task->state);
		if(state_name == NULL){

		if (state == (TASK_WAKEKILL | TASK_UNINTERRUPTIBLE)){
			state_name =  "TASK_WAKEKILL, TASK_UNINTERRUPTIBLE";
		}else if(state == (TASK_WAKEKILL | __TASK_STOPPED)){
			state_name = "TASK_WAKEKILL , __TASK_STOPPED";
		}else if(state == (TASK_WAKEKILL | __TASK_TRACED)){
			state_name = "TASK_WAKEKILL , __TASK_TRACED";
		}else if(state == (TASK_UNINTERRUPTIBLE | TASK_NOLOAD)){
			state_name = "TASK_UNINTERRUPTIBLE , TASK_NOLOAD"; 
		}else if(state == (TASK_INTERRUPTIBLE | TASK_UNINTERRUPTIBLE)){
			state_name = "TASK_INTERRUPTIBLE , TASK_UNINTERRUPTIBLE";
		}else if(state == (TASK_RUNNING | TASK_INTERRUPTIBLE | TASK_UNINTERRUPTIBLE | __TASK_STOPPED | __TASK_TRACED | EXIT_ZOMBIE | EXIT_DEAD)){
			state_name = "TASK_RUNNING , TASK_INTERRUPTIBLE , TASK_UNINTERRUPTIBLE , __TASK_STOPPED , __TASK_TRACED , EXIT_ZOMBIE , EXIT_DEAD";
		}else{
			state_name = "OTHER";
		}
		}
	sprintf(buffer + strlen(buffer) ,"\t\nPROCESS ID: %d\tPARENT PID: %d\tCPU: %d\tSTATE: %s\n",task->pid, parent_pid, task_cpu(task), state_name);
	}
	printk(KERN_INFO "ProcessList: Kernal print success\n");
	sizeBuffer = strlen(buffer);
	errorCount = copy_to_user(ubuffer, &buffer, sizeBuffer);
	printk("ProcessList: copy_to_user returned (%d)", errorCount);
	return sizeBuffer;
}
 
static int dev_release(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "ProcessList: Device successfully closed\n");
   return 0;
}
 
module_init(proList_init);
module_exit(proList_exit);
