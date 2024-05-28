CC = aarch64-none-elf-gcc
CXX = aarch64-none-elf-g++
AS = aarch64-none-elf-as
LD = aarch64-none-elf-ld
AR = aarch64-none-elf-ar
OBJCOPY = aarch64-none-elf-objcopy
prefix:=$(HOME)/.cros/root

aarch64_ldscript = src/aarch64/aarch64.ld
aarch64_objs = src/aarch64/boot.o src/aarch64/aarch64.o \
		src/aarch64/irq/exceptions.o src/aarch64/irq/irq.o \
		src/aarch64/bootstrap.o src/aarch64/sysreg.o src/aarch64/irq/interrupts.o

memory_objs_common = src/memory/addressspace.o src/memory/heap.o src/memory/memorymap.o \
	src/memory/mmap.o src/memory/new.o src/memory/pageallocator.o
memory_objs_aarch64 = src/memory/aarch64/mmu.o

fs_objs_common = src/fs/fat32/helpers.o src/fs/fat32/entry_helpers.o src/fs/fat32/entry.o \
	src/fs/fat32/fat32.o src/fs/fat32/fs_helpers.o \
	src/fs/fat32/disk_interface/disk_interface.o src/fs/fat32/filecontextfat32.o \
	src/fs/pipe.o src/fs/filecontext.o

loader_objs_common = src/loader/elf.o

sched_objs_common = src/sched/process.o src/sched/queue.o
sched_objs_aarch64 = src/sched/aarch64/context.o src/sched/aarch64/loadcontext.o

device_objs_common = src/devices/timer.o src/devices/uart.o

util_objs_common = src/util/log.o src/util/string.o src/util/hasrefcount.o
util_objs_aarch64 = src/util/aarch64/hacf.o

objs = src/kernel.o src/irq/interrupts.o src/containers/string.o \
	$(memory_objs_common) $(memory_objs_aarch64) $(loader_objs_common) $(fs_objs_common) $(device_objs_common) $(sched_objs_common) $(sched_objs_aarch64) $(util_objs_common) $(util_objs_aarch64)

CRTI_OBJ=src/aarch64/crti.o
CRTBEGIN_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtend.o)
CRTN_OBJ=src/aarch64/crtn.o

kernel_elf = kernel.elf
kernel_binary = kernel8.img

libsyscall = libsyscall.a
libsyscall_obj = src/aarch64/dosyscall.o

testprog_bin = init
testprog_obj = test/entry.o test/main.o

CFLAGS = -Iinclude/ -Isrc/  -ffreestanding -Wall -Wextra -ggdb -O0 -mgeneral-regs-only
CXXFLAGS = -Iinclude/ -Isrc/ -ffreestanding -fpermissive -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -Wextra -ggdb -O0 -mgeneral-regs-only
LDFLAGS = -T $(aarch64_ldscript) -nostdlib

.PHONY: all
all: $(libsyscall) $(testprog_bin) $(kernel_binary) 

.PHONY: clean
clean:
	rm -f $(CRTI_OBJ) $(CRTN_OBJ) $(objs) $(aarch64_objs) $(kernel_elf) $(kernel_binary) $(libsyscall) $(libsyscall_obj) $(testprog_bin) $(testprog_obj)

.PHONY: install
install:
	./../scripts/create_and_mount_img.sh $(prefix)
	mkdir -p $(prefix)/include
	mkdir -p $(prefix)/boot
	mkdir -p $(prefix)/lib
	mkdir -p $(prefix)/bin
	cp kernel8.img $(prefix)/boot
	cp -r include/* $(prefix)/include
	cp libsyscall.a $(prefix)/lib
	cp $(testprog_bin) $(prefix)/bin

$(kernel_binary): $(kernel_elf)
	$(OBJCOPY) $(kernel_elf) -O binary $@

$(kernel_elf): $(CRTI_OBJ) $(CRTBEGIN_OBJ) $(objs) $(aarch64_objs) $(CRTEND_OBJ) $(CRTN_OBJ)  $(aarch64_ldscript) $(testprog_bin)
	$(CXX) -o $@ $(LDFLAGS) $(CRTI_OBJ) $(CRTBEGIN_OBJ) $(objs) $(aarch64_objs) $(CRTEND_OBJ) $(CRTN_OBJ)  -lgcc

$(libsyscall): $(libsyscall_obj)
	$(AR) -rcs $@ $^

$(testprog_bin): $(testprog_obj)
	$(CC) -o $@ -T test/linker.ld -nostdlib $^ -L. -lgcc -lsyscall

.PHONY: clobber
clobber:
	./../scripts/unmount_img.sh $(prefix)
