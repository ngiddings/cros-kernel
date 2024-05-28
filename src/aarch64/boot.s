.section ".boot.data"
.balign 4096
ttl0:
.skip 4096
ttl1:
.skip 4096
ttl2:
.skip 4096

.section ".boot.text"
 
// Entry point for the kernel. Registers:
// x0 -> 32 bit pointer to DTB in memory (primary core only) / 0 (secondary cores)
.globl _start
_start:

    // Check if we're already in EL1
  /*ldr     x6, =_el1_prepare
    bic     x6, x6, #0xFFFF << 48
    mrs     x5, CurrentEL
    cmp     x5, #8
    blt     x6*/
    
    // Disable IRQ routing to EL2, set EL1 execution mode to AArch64
    mrs     x5, HCR_EL2
    orr     x5, x5, #0x80000000
    bic     x5, x5, #0x38
    msr     HCR_EL2, x5

    // Set virtual processor ID
    mrs     x5, MIDR_EL1
    msr     VPIDR_EL2, x5

    // Enable floating-point
    mrs     x5, CPACR_EL1
    orr     x5, x5, #3 << 20
    msr     CPACR_EL1, x5

    // Set stack before our code
    ldr     x5, =_start
    msr     SP_EL1, x5

    // Point exception link register to _el1_begin
    ldr     x5, =_el1_begin
    msr     ELR_EL2, x5

    // Modify saved program status register to mask exceptions and be in EL1
    mrs     x5, SPSR_EL2
    bic     x5, x5, #0xF
    mov     x6, #0x3C5
    orr     x5, x5, x6
    msr     SPSR_EL2, x5

    // Fall into EL1, jump to _el1_begin
    eret
_el1_prepare:
    ldr     x5, =_start
    mov     sp, x5
_el1_begin:
    // Save x0 on stack
    stp x0, x1, [sp, #-16]!

    ldr x0, =bootstrap_vector_table_el1
    msr VBAR_EL1, x0

    // Prepare arguments for call to mmio_init
    ldr     x0, =0x2000000
    ldr     x1, =ttl0
    ldr     x2, =ttl1
    ldr     x3, =ttl2
    ldr     x4, =mmu_init

    // Call mmio_init
    blr     x4

    // Restore x0
    ldp x0, x1, [sp], #16
    ldr     x1, =0x2000000

    // Initialize stack in high memory
    ldr     x5, =__begin
    mov     sp, x5

    stp     x0, x1, [sp, #-16]!

    ldr     x5, =_init
    blr     x5

    ldp     x0, x1, [sp], #16

    ldr     x5, =aarch64_boot
    blr     x5

halt:
    wfe
    b       halt

.balign 0x800
bootstrap_vector_table_el1:
    bootstrap__ex_el1_curr_sp0_sync:
        mrs x0, ESR_EL1
        bl handle_ex
        b halt
        
    .balign 0x80
    bootstrap__ex_el1_curr_sp0_irq:
        bl handle_ex
        b halt
        
    .balign 0x80
    bootstrap__ex_el1_curr_sp0_fiq:
        bl handle_ex
        b halt
        
    .balign 0x80
    bootstrap__ex_el1_curr_sp0_serror:
        bl handle_ex
        b halt
        
    .balign 0x80
    bootstrap__ex_el1_curr_spx_sync:
        ldr x0, =_start
        mov sp, x0
        mrs x0, ESR_EL1
        bl handle_ex
        b halt
        
    .balign 0x80
    bootstrap__ex_el1_curr_spx_irq:
        bl handle_ex
        b halt
        
    .balign 0x80
    bootstrap__ex_el1_curr_spx_fiq:
        bl handle_ex
        b halt
        
    .balign 0x80
    bootstrap__ex_el1_curr_spx_serror:
        bl handle_ex
        b halt
        
    .balign 0x80
    bootstrap__ex_el1_lower_aarch64_sync:
        bl handle_ex
        b halt
        
    .balign 0x80
    bootstrap__ex_el1_lower_aarch64_irq:
        bl handle_ex
        b halt
        
    .balign 0x80
    bootstrap__ex_el1_lower_aarch64_fiq:
        bl handle_ex
        b halt
        
    .balign 0x80
    bootstrap__ex_el1_lower_aarch64_serror:
        bl handle_ex
        b halt
        
    .balign 0x80
    bootstrap__ex_el1_lower_aarch32_sync:
        bl handle_ex
        b halt
        
    .balign 0x80
    bootstrap__ex_el1_lower_aarch32_irq:
        bl handle_ex
        b halt

    .balign 0x80
    bootstrap__ex_el1_lower_aarch32_fiq:
        bl handle_ex
        b halt

    .balign 0x80
    bootstrap__ex_el1_lower_aarch32_serror:
        bl handle_ex
        b halt
