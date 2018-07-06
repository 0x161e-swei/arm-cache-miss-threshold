/*
 * Enable user-mode ARM performance counter access.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/smp.h>

#if defined(__aarch64__)

static void enable_cycle_counters(void* data) {
        u64 val;
	/* Disable cycle counter overflow interrupt */
	asm volatile("msr pmintenset_el1, %0" : : "r" ((u64)(0 << 31)));
	/* Enable cycle counter */
	asm volatile("msr pmcntenset_el0, %0" :: "r" BIT(31));
	/* Enable user-mode access to cycle counters. */
	asm volatile("msr pmuserenr_el0, %0" : : "r"(BIT(0) | BIT(2)));
	/* Clear cycle counter and start */
	asm volatile("mrs %0, pmcr_el0" : "=r" (val));
        printk(KERN_INFO "pmcr_el0 old value %llx", val);
	val |= (BIT(0) | BIT(2) | BIT(6));
        isb();
        asm volatile("msr pmcr_el0, %0" : : "r" (val));
	val = BIT(27);
	asm volatile("msr pmccfiltr_el0, %0" : : "r" (val));
}

static void disable_cycle_counters(void* data) {
	/* Disable cycle counter */
	u64 val;
	asm volatile("mrs %0, pmcr_el0" : "=r" (val));
        printk(KERN_INFO "pmcr_el0 current value %x", val);
	asm volatile("msr pmcntenset_el0, %0" :: "r" (0 << 31));
	/* Disable user-mode access to counters. */
	asm volatile("msr pmuserenr_el0, %0" : : "r"((u64)0));

}

#else

#define DRVR_NAME "enable_arm_cpu_counters"

static void enable_cycle_counters(void* data) {
    uint32_t pmcr = 0;
    /* Enable user - mode access to counters . */
    asm volatile ( "mcr p15, 0, %0, c9, c14, 0" :: "r" (1) );
   
    /* Disable Overflow Interrupt for all counters including PMCCNTR */
    asm volatile ( "mcr p15, 0, %0, c9, c14, 1 " :: "r" (0x0));

    // PMCR[bit 0] = E (Enable all counters, including PMCNTR)
    // PMCR[bit 1] = P (Reset all counters, expect PMCNTR)
    // PMCR[bit 2] = C (Reset PMCNTR)
    // PMCR[bit 3] = D (Cycle counter clock divider, switches to counting every 64 cycles)
    // PMCR[bit 4] = X (Export of events to an external debug device)
    // PMCR[bit 5] = DP (Disable PMCCNTR when event counting is prohibited)
    // PMCR[bits 11:15] = N (Number of event counters)
    asm volatile ( "mrc p15, 0, %0, c9, c12, 0" : "=r"(pmcr));
    pmcr &= ~0x0000003fu;
    pmcr |=  0x00000015u;
    /* Write to Performance Monitor Control Register */
    asm volatile ( "mcr p15, 0, %0, c9, c12, 0" :: "r" (pmcr));

    /* Write to Count Enable Set Reg, enabling cycle counter (top bit) */
    asm volatile ( "mcr p15, 0, %0, c9, c12, 1" :: "r" (0x80000000));

    asm volatile ( "mrc p15, 0, %0, c0, c0, 0" : "=r"(pmcr));
    /* check if this is cortex A15 */
    if ((pmcr & 0xfff0) == 0xc0f0) {
        asm volatile ( "mrc p15, 1, %0, c15, c0, 3" : "=r"(pmcr));
        printk(KERN_INFO "L2PFR %x", pmcr);
        // reading the following regs from nonsecure world faults the machine
        // MRC p15, 0, <Rt>, c1, c1, 0
        // asm volatile ( "mrc p15, 0, %0, c1, c1, 0" : "=r"(pmcr));
        // printk(KERN_INFO "Secure Mode %x", pmcr);
        /* Read Secure Configuration Register data */
	    /* disable all hardware prefetchers */
        // pmcr = 0x400;
        // asm volatile ( "mcr p15, 1, %0, c15, c0, 3" :: "r"(pmcr));
    }

}

static void disable_cycle_counters(void* data) {
    /* Disable PMU */
    asm volatile ( "mcr p15, 0, %0, c9, c12, 0" :: "r" (0) ) ;
    /* Disable user - mode access to counters . */
    asm volatile ( "mcr p15, 0, %0, c9, c14, 0" :: "r" (0) ) ;
}

#endif

static int __init init (void) {
    on_each_cpu(enable_cycle_counters, NULL, 1);
    return 0;
}
static void __exit fini (void) {
    on_each_cpu(disable_cycle_counters, NULL, 1);
}

/* Following line is a hack for kernel version compatibility */
// MODULE_INFO(vermagic, "3.10.103+ SMP preempt mod_unload ARMv7 p2v8 ");
MODULE_DESCRIPTION("Enables user-mode access to ARMv8 PMU counters");
MODULE_LICENSE("GPL");
module_init(init);
module_exit(fini);

