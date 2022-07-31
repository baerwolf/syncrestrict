
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/kprobes.h>
#include <linux/ptrace.h>

#include <linux/sched.h>



#if  ((!defined(CONFIG_KPROBES)) || (!defined(CONFIG_HAVE_KPROBES)))
#   error KProbes must be supported by kernel compiling for!
#endif

#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,10,0)
#   warning This module is intended for kernel versions >= 5.10
#endif

#ifdef CONFIG_PREEMPT_RT
#   warning Realtime kernels may behave unexpected - be warned
#endif

static void syncrestrict_justreturn(void) {
    // empty dummy function
    // just implement another function within same ABI for returning pending function-call (to ksys_sync)
}

static int syncrestrict_thehook(struct kprobe *p, struct pt_regs *regs) {
    const struct cred *__currentcred = current_cred();
    if ((!capable(CAP_SYS_ADMIN)) || (strncmp(current->comm, "sync", sizeof(current->comm))==0)) {
        /* non-root/non-capable user tries to global sync the system */
        if (printk_ratelimit()) {
            printk( KERN_INFO "syncrestrict: sync (0x%px: %pS) was called by %s(%i) as euid %i - syncing denied\n", (void*)instruction_pointer(regs), (void*)instruction_pointer(regs), current->comm, current->pid, __currentcred->euid.val);
        }
        // using regs->ip directly causes problems with plattform independence (some ISAs use different names)...
        // ...use ptrace wrappers instead...
        instruction_pointer_set(regs, (unsigned long)syncrestrict_justreturn);
        return 1;
    }
    return 0; /* continue normal "ksys_sync" */
}


static int registeringsuccess = 0;
static struct kprobe ksys_sync_hook = {
//  .flags          = KPROBE_FLAG_DISABLED,
    .symbol_name    = "ksys_sync",
    .pre_handler    = syncrestrict_thehook,
};

static int __init syncrestrict_init(void)
{
        registeringsuccess = register_kprobe(&ksys_sync_hook);
        if (registeringsuccess == 0) {
            printk(KERN_INFO "syncrestrict: ksys_sync hook successfully registered\n");
        } else {
            printk(KERN_ALERT "syncrestrict: ERROR registering ksys_sync hook\n");
        }
        return 0;
}

static void __exit syncrestrict_exit(void) 
{
        if (registeringsuccess == 0) {
            unregister_kprobe(&ksys_sync_hook);
        }
        printk(KERN_INFO "syncrestrict: module unloaded - good bye\n");
}


module_init(syncrestrict_init);
module_exit(syncrestrict_exit);

MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Stephan Baerwolf" );
MODULE_DESCRIPTION( "syncrestrict hooks syscalls to sync via kprobing ksys_sync in order to apply restrictions" );
