ifneq ($(KERNELRELEASE),)
# kbuild part of makefile
obj-m  := syncrestrict.o
syncrestrict-y := syncrestrict_core.o

else
# normal makefile
KDIR ?= /lib/modules/`uname -r`/build

default:
	$(MAKE) -C $(KDIR) M=$$PWD

clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean

endif
