.section ".text"

.global load_context
load_context:
    // Load FP registers from (*x0)
    ld4 {v0.2d, v1.2d, v2.2d, v3.2d}, [x0], #64
    ld4 {v4.2d, v5.2d, v6.2d, v7.2d}, [x0], #64
    ld4 {v8.2d, v9.2d, v10.2d, v11.2d}, [x0], #64
    ld4 {v12.2d, v13.2d, v14.2d, v15.2d}, [x0], #64
    ld4 {v16.2d, v17.2d, v18.2d, v19.2d}, [x0], #64
    ld4 {v20.2d, v21.2d, v22.2d, v23.2d}, [x0], #64
    ld4 {v24.2d, v25.2d, v26.2d, v27.2d}, [x0], #64
    ld4 {v28.2d, v29.2d, v30.2d, v31.2d}, [x0], #64
    
    // Skip X0-X3 (we need to use them as scratch registers)
    add x0, x0, #32
    
    // Load X4-X30, SP
    ldp x4, x5, [x0], #16
    ldp x6, x7, [x0], #16
    ldp x8, x9, [x0], #16
    ldp x10, x11, [x0], #16
    ldp x12, x13, [x0], #16
    ldp x14, x15, [x0], #16
    ldp x16, x17, [x0], #16
    ldp x18, x19, [x0], #16
    ldp x20, x21, [x0], #16
    ldp x22, x23, [x0], #16
    ldp x24, x25, [x0], #16
    ldp x26, x27, [x0], #16
    ldp x28, x29, [x0], #16
    ldp x30, x1, [x0], #16
    msr sp_el0, x1

    // Load PC, status register
    ldp x2, x3, [x0], #16
    msr elr_el1, x2
    msr spsr_el1, x3

    // Load FP control & status
    ldp x2, x3, [x0], #16
    msr fpcr, x2
    msr fpsr, x3

    // Load kernel SP
    ldp x2, x3, [x0, #-8]
    mov sp, x3

    // Go back to X0-X3
    sub x0, x0, #288

    // Load X2 & X3
    ldp x2, x3, [x0, #16]

    // Load X0 & X1
    ldp x0, x1, [x0]
    
    eret
