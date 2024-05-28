.section ".text"

.macro save_context
    // Make room on stack for process context
    sub sp, sp, #816
    
    // Save FP registers
    st4 {v0.2d, v1.2d, v2.2d, v3.2d}, [sp], #64
    st4 {v4.2d, v5.2d, v6.2d, v7.2d}, [sp], #64
    st4 {v8.2d, v9.2d, v10.2d, v11.2d}, [sp], #64
    st4 {v12.2d, v13.2d, v14.2d, v15.2d}, [sp], #64
    st4 {v16.2d, v17.2d, v18.2d, v19.2d}, [sp], #64
    st4 {v20.2d, v21.2d, v22.2d, v23.2d}, [sp], #64
    st4 {v24.2d, v25.2d, v26.2d, v27.2d}, [sp], #64
    st4 {v28.2d, v29.2d, v30.2d, v31.2d}, [sp], #64
    
    // Save X0-X29
    stp x0, x1, [sp], #16
    stp x2, x3, [sp], #16
    stp x4, x5, [sp], #16
    stp x6, x7, [sp], #16
    stp x8, x9, [sp], #16
    stp x10, x11, [sp], #16
    stp x12, x13, [sp], #16
    stp x14, x15, [sp], #16
    stp x16, x17, [sp], #16
    stp x18, x19, [sp], #16
    stp x20, x21, [sp], #16
    stp x22, x23, [sp], #16
    stp x24, x25, [sp], #16
    stp x26, x27, [sp], #16
    stp x28, x29, [sp], #16
    
    // Save x30 and SP
    mrs x8, sp_el0
    stp x30, x8, [sp], #16

    // Save PC and program status
    mrs x8, elr_el1
    mrs x9, spsr_el1
    stp x8, x9, [sp], #16

    // Save FPCR and FPSR
    mrs x8, fpcr
    mrs x9, fpsr
    stp x8, x9, [sp], #16

    // Save kernel SP
    mov x8, sp
    add x8, x8, #16
    stp x8, x8, [sp], #16

    // Set SP to beginning of context struct
    sub sp, sp, #816
.endm

_sync_el0_wrapper:
    save_context

    // Obtain and unpack exception syndrome
    mrs x8, esr_el1
    mov x9, x8
    ldr x10, = 0x1F03FFFFFF
    bic x8, x8, x10
    lsr x8, x8, #26
    mvn x10, x10
    bic x9, x9, x10

    // Point X5 to process context struct in case we are doing a syscall
    mov x5, sp

    // If exception class 0x15, we are doing a syscall
    cmp x8, #0x15
    bne _not_syscall

    bl do_syscall
    bl load_context

_not_syscall:
    // Else, call handle_sync with exception class and ISR
    mov x0, x8
    mov x1, x9
    mov x2, x5
    bl handle_sync
    
    // Load next process context returned from handle_sync
    bl load_context

_sync_el1_wrapper:
    // Save caller-saved registers
    stp x0, x1, [sp, #-16]!
    stp x2, x3, [sp, #-16]!
    stp x4, x5, [sp, #-16]!
    stp x6, x7, [sp, #-16]!
    stp x8, x9, [sp, #-16]!
    stp x10, x11, [sp, #-16]!
    stp x12, x13, [sp, #-16]!
    stp x14, x15, [sp, #-16]!
    stp x16, x17, [sp, #-16]!
    stp x18, lr, [sp, #-16]!

    // Unpack exception syndrome register
    mrs x0, esr_el1
    mov x1, x0
    ldr x2, = 0x1F03FFFFFF
    bic x0, x0, x2
    lsr x0, x0, #26
    mvn x2, x2
    bic x1, x1, x2

    // Give handle_sync() a NULL-pointer as a process context, as we came from kernelspace
    mov x2, #0
    
    // Call handle_sync()
    bl handle_sync

    // Restore saved registers
    ldp x18, lr, [sp], #16
    ldp x16, x17, [sp], #16
    ldp x14, x15, [sp], #16
    ldp x12, x13, [sp], #16
    ldp x10, x11, [sp], #16
    ldp x8, x9, [sp], #16
    ldp x6, x7, [sp], #16
    ldp x4, x5, [sp], #16
    ldp x2, x3, [sp], #16
    ldp x0, x1, [sp], #16
    
    // Exception return
    eret

_irq_el0_wrapper:
    save_context

    // Call find_irq_source() to obtain an interrupt number
    bl find_irq_source
    
    // Point X1 to saved process context, then call handle_irq()
    mov x1, sp
    bl handle_irq

    // Load context returned by handle_irq()
    bl load_context

_irq_el1_wrapper:
    // Save caller-saved registers
    stp x0, x1, [sp, #-16]!
    stp x2, x3, [sp, #-16]!
    stp x4, x5, [sp, #-16]!
    stp x6, x7, [sp, #-16]!
    stp x8, x9, [sp, #-16]!
    stp x10, x11, [sp, #-16]!
    stp x12, x13, [sp, #-16]!
    stp x14, x15, [sp, #-16]!
    stp x16, x17, [sp, #-16]!
    stp x18, lr, [sp, #-16]!

    // Call find_irq_source() to obtain an interrupt number 
    bl find_irq_source

    // Set X1 to NULL because we came from kernelspace, then call handle_irq()
    mov x1, #0
    bl handle_irq

    // Restore saved registers
    ldp x18, lr, [sp], #16
    ldp x16, x17, [sp], #16
    ldp x14, x15, [sp], #16
    ldp x12, x13, [sp], #16
    ldp x10, x11, [sp], #16
    ldp x8, x9, [sp], #16
    ldp x6, x7, [sp], #16
    ldp x4, x5, [sp], #16
    ldp x2, x3, [sp], #16
    ldp x0, x1, [sp], #16

    // Exception return
    eret

.balign 0x800
.global vector_table_el1
vector_table_el1:
    _ex_el1_curr_sp0_sync:
        b hacf
        
    .balign 0x80
    _ex_el1_curr_sp0_irq:
        b hacf
        
    .balign 0x80
    _ex_el1_curr_sp0_fiq:
        b hacf
        
    .balign 0x80
    _ex_el1_curr_sp0_serror:
        b hacf
        
    .balign 0x80
    _ex_el1_curr_spx_sync:
        b _sync_el1_wrapper
        
    .balign 0x80
    _ex_el1_curr_spx_irq:
        b _irq_el1_wrapper
        
    .balign 0x80
    _ex_el1_curr_spx_fiq:
        b _irq_el1_wrapper
        
    .balign 0x80
    _ex_el1_curr_spx_serror:
        b hacf
        
    .balign 0x80
    _ex_el1_lower_aarch64_sync:
        b _sync_el0_wrapper
        
    .balign 0x80
    _ex_el1_lower_aarch64_irq:
        b _irq_el0_wrapper
        
    .balign 0x80
    _ex_el1_lower_aarch64_fiq:
        b _irq_el0_wrapper
        
    .balign 0x80
    _ex_el1_lower_aarch64_serror:
        b hacf
        
    .balign 0x80
    _ex_el1_lower_aarch32_sync:
        b hacf
        
    .balign 0x80
    _ex_el1_lower_aarch32_irq:
        b hacf

    .balign 0x80
    _ex_el1_lower_aarch32_fiq:
        b hacf

    .balign 0x80
    _ex_el1_lower_aarch32_serror:
        b hacf
