.section .text

# void *get_vbar_el1();
.global get_vbar_el1
get_vbar_el1:
    mrs x0, vbar_el1
    ret

# void *set_vbar_el1(void *vbar_el1);
.global set_vbar_el1
set_vbar_el1:
    msr vbar_el1, x0
    ret

# int get_exception_level();
.global get_exception_level
get_exception_level:
    mrs x0, CurrentEL
    ret

# int get_daif();
.global get_daif
get_daif:
    mrs x0, daif
    ret

# int set_daif(int bits);
.global set_daif
set_daif:
    msr daif, x0
    ret

# void *get_far_el1();
.global get_far_el1
get_far_el1:
    mrs x0, far_el1
    ret

# unsigned long get_spsr_el1();
.global get_spsr_el1
get_spsr_el1:
    mrs x0, spsr_el1
    ret

# unsigned long set_spsr_el1(unsigned long spsr_el1);
.global set_spsr_el1
set_spsr_el1:
    msr spsr_el1, x0
    ret

# void *get_elr_el1();
.global get_elr_el1
get_elr_el1:
    mrs x0, elr_el1
    ret

# void *set_elr_el1(void *elr_el1);
.global set_elr_el1
set_elr_el1:
    msr elr_el1, x0
    ret

# unsigned long get_esr_el1();
.global get_esr_el1
get_esr_el1:
    mrs x0, esr_el1
    ret

# unsigned long get_fpcr();
.global get_fpcr
get_fpcr:
    mrs x0, fpcr
    ret

# unsigned long set_fpcr(unsigned long fpcr);
.global set_fpcr
set_fpcr:
    msr fpcr, x0
    ret

# unsigned long get_fpsr();
.global get_fpsr
get_fpsr:
    mrs x0, fpsr
    ret

# unsigned long set_fpsr(unsigned long fpsr);
.global set_fpsr
set_fpsr:
    msr fpsr, x0
    ret

# unsigned long get_cpacr_el1();
.global get_cpacr_el1
get_cpacr_el1:
    mrs x0, cpacr_el1
    ret

# unsigned long set_cpacr_el1(unsigned long cpacr_el1);
.global set_cpacr_el1
set_cpacr_el1:
    msr cpacr_el1, x0
    ret

# unsigned long get_sctlr_el1();
.global get_sctlr_el1
get_sctlr_el1:
    mrs x0, sctlr_el1
    ret

# unsigned long set_sctlr_el1(unsigned long sctlr_el1);
.global set_sctlr_el1
set_sctlr_el1:
    msr sctlr_el1, x0
    ret

# unsigned long get_midr_el1();
.global get_midr_el1
get_midr_el1:
    mrs x0, midr_el1
    ret

# void *get_sp_el0();
.global get_sp_el0
get_sp_el0:
    mrs x0, sp_el0
    ret

# void *set_sp_el0(void *sp_el0);
.global set_sp_el0
set_sp_el0:
    msr sp_el0, x0
    ret

# void *get_ttbr0_el1();
.global get_ttbr0_el1
get_ttbr0_el1:
    mrs x0, ttbr0_el1
    ret

# void *set_ttbr0_el1(unsigned long ttbr0_el1);
.global set_ttbr0_el1
set_ttbr0_el1:
    msr ttbr0_el1, x0
    ret
