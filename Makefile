TARGET ?= x86_64-elf
ARCH ?= x86_64
CC = $(TARGET)-gcc
AR = $(TARGET)-ar

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

CFLAGS = -Wall -Wextra -Wdeclaration-after-statement -ffreestanding -fno-builtin -fno-stack-protector -fno-pic -Os -fomit-frame-pointer -nostdinc -ffunction-sections -fdata-sections -fdiagnostics-color=always
CPPFLAGS = -isystem $(LIBC_ABS)/leblibc/include -isystem $(abspath $(SYSROOT))/usr/include -isystem $(LIBC_ABS)/leblibc/arch/$(ARCH) -isystem $(LIBC_ABS)/leblibc/arch/generic -I$(LIBC_ABS)/include -I$(LIBC_ABS)/src -Iinclude
CFLAGS += $(EXTRA_CFLAGS)
CPPFLAGS += $(EXTRA_CPPFLAGS)

SRCS = src/terminal.c src/input.c src/draw.c src/tabs.c \
       src/menus.c src/dialogs.c src/progress.c
OBJS = $(patsubst src/%.c,build/%.o,$(SRCS))
DEPS = $(OBJS:.o=.d)

LIBDIR = lib
PREFIX ?= /usr
DESTDIR ?=

.PHONY: all clean install

all: $(LIBDIR)/liblebui.a

build/%.o: src/%.c include/lebui.h
	$(Q)mkdir -p $(dir $@)
	$(MSG_CC)$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -c $< -o $@

$(LIBDIR)/liblebui.a: $(OBJS)
	$(Q)mkdir -p $(LIBDIR)
	$(MSG_AR)$(AR) rcs $@ $(OBJS)

clean:
	rm -rf build $(LIBDIR)

install: $(LIBDIR)/liblebui.a
	$(Q)mkdir -p $(DESTDIR)$(PREFIX)/include $(DESTDIR)$(PREFIX)/lib
	$(Q)cp include/lebui.h $(DESTDIR)$(PREFIX)/include/lebui.h
	$(Q)cp $(LIBDIR)/liblebui.a $(DESTDIR)$(PREFIX)/lib/liblebui.a

-include $(DEPS)
