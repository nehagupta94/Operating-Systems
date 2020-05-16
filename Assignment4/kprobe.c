#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/ktime.h>

#define MAX_SYMBOL_LEN	64
#define BUFFER_SIZE 500

static char symbol[MAX_SYMBOL_LEN] = "handle_mm_fault";
module_param_string(symbol, symbol, sizeof(symbol), 0644);

static char *name = "handle_mm_fault";

int position=0;
pid_t process_id= 1;
module_param(process_id, int, 0);

/* For each probe you need to allocate a kprobe structure */
static struct kprobe kp = {
	.symbol_name	= symbol,
};

struct DataBuffer{
	unsigned long address;
	long time;
};

static struct kprobe kp;
struct DataBuffer buffer[BUFFER_SIZE];

static int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
	struct timespec xtime;
	//printk(KERN_INFO "In Prehandler currentPID %d\n",current->pid);
	//printk(KERN_INFO "Page fault for process id %d occured at Address 0x%lx at time %ld\n",current->pid,regs->si,(long)xtime.tv_nsec);
	if(process_id == current->pid) {
		xtime = current_kernel_time();
		printk(KERN_INFO "Page fault for process id %d occured at Address 0x%ld at time %ld\n",current->pid,regs->si,(long)xtime.tv_nsec);
	}
	return 0;
}
/* kprobe post_handler: called after the probed instruction is executed */
static void handler_post(struct kprobe *p, struct pt_regs *regs,
				unsigned long flags)
{
}


static int __init kprobe_init(void)
{
	int ret;
	kp.pre_handler = handler_pre;
	kp.post_handler = handler_post;
	kp.symbol_name = name;

	ret = register_kprobe(&kp);
	if (ret < 0) {
		pr_err("register_kprobe failed, returned %d\n", ret);
		return ret;
	}
	pr_info("Planted kprobe at %p\n", kp.addr);
	return 0;
}

static void __exit kprobe_exit(void)
{
	unregister_kprobe(&kp);
	pr_info("kprobe at %p unregistered\n", kp.addr);
}

module_init(kprobe_init)
module_exit(kprobe_exit)
MODULE_LICENSE("Dual BSD/GPL");
