#ifndef KERNEL_SYSREG_H
#define KERNEL_SYSREG_H

extern "C"
void *get_vbar_el1();

extern "C"
void *set_vbar_el1(void *vbar_el1);

extern "C"
int get_exception_level();

extern "C"
int get_daif();

extern "C"
int set_daif(int bits);

extern "C"
void *get_far_el1();

extern "C"
unsigned long get_spsr_el1();

extern "C"
unsigned long set_spsr_el1(unsigned long spsr_el1);

extern "C"
void *get_elr_el1();

extern "C"
void *set_elr_el1(void *elr_el1);

extern "C"
unsigned long get_esr_el1();

extern "C"
unsigned long get_fpcr();

extern "C"
unsigned long set_fpcr(unsigned long fpcr);

extern "C"
unsigned long get_fpsr();

extern "C"
unsigned long set_fpsr(unsigned long fpsr);

extern "C"
unsigned long get_cpacr_el1();

extern "C"
unsigned long set_cpacr_el1(unsigned long cpacr_el1);

extern "C"
unsigned long get_sctlr_el1();

extern "C"
unsigned long set_sctlr_el1(unsigned long sctlr_el1);

extern "C"
unsigned long get_midr_el1();

extern "C"
unsigned long get_sp_el0();

extern "C"
unsigned long set_sp_el0(void *sp_el0);

extern "C"
unsigned long get_ttbr0_el1();

extern "C"
unsigned long set_ttbr0_el1(unsigned long ttbr0_el1);

#endif