#include <stdint.h>
#include "sys/syscall.h"

void trampoline()
{
    sigret();
}

void handler(void *userdata)
{
    printk("Handler called.\n");
}

void thread(void *data)
{
    printk("Thread 2\n");
    terminate();
}

int main(int argc, char **argv, char **envp)
{
    printk("Process start: ");
    printk(argv[0]);
    printk("\n");
    mmap((void *)0, 0x10000, 1);
    sigaction(17, handler, trampoline, (void *)0);
    while (1)
    {
        clone(thread, (void *)0x10000, 0, 0);
        sigwait();
    }
}
