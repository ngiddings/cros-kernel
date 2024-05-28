.section ".text"
.global do_syscall
.type do_syscall, "function"
do_syscall:
    svc #0
    ret
