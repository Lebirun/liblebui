CC = x86_64-elf-gcc
AR = x86_64-elf-ar

V ?= 0
ifeq ($(V),0)
  Q = @
  MSG_CC    = @printf '  CC      %s\n' $<;
  MSG_AR    = @printf '  AR      %s\n' $@;
else
  Q =
  MSG_CC =
  MSG_AR =
endif

LIBC = ../../libc
LIBC_ABS = $(abspath $(LIBC))
SYSROOT = ../../sysroot

CFLAGS = -Wall -Wextra -ffreestanding -fno-builtin -fno-stack-protector -fno-pic -Os -fomit-frame-pointer -nostdinc -ffunction-sections -fdata-sections -fdiagnostics-color=always
CPPFLAGS = -isystem $(LIBC_ABS)/leblibc/include -isystem $(abspath $(SYSROOT))/usr/include -isystem $(LIBC_ABS)/leblibc/arch/x86_64 -isystem $(LIBC_ABS)/leblibc/arch/generic -I$(LIBC_ABS)/include -I$(LIBC_ABS)/src -Iinclude

SRCS = src/lebui.c
OBJS = $(patsubst src/%.c,build/%.o,$(SRCS))

LIBDIR = lib

.PHONY: all clean

all: $(LIBDIR)/liblebui.a

build/%.o: src/%.c include/lebui.h
	$(Q)mkdir -p $(dir $@)
	$(MSG_CC)$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(LIBDIR)/liblebui.a: $(OBJS)
	$(Q)mkdir -p $(LIBDIR)
	$(MSG_AR)$(AR) rcs $@ $(OBJS)

clean:
	rm -rf build $(LIBDIR)
