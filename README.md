syncrestrict
============

This repository builds an out-of-tree module named "syncrestrict.ko".

In environments with lots of filesystems and lots of IO, unintentional "sync" might introduce system stability issues.
Filesystems may block (until data is committed to disk) and many processes can got to disksleep (D) for quite some time because of this.
Since "sync" can be called by any unpriviledge user (and thus as part of other software/script execution), this module might help increasing overall system stability.

When loaded, syncrestrict will restrict regular calls of "sync" to CAP_SYS_ADMIN capable users only.
Alternativly all binaries NOT called "sync" are allowed to raise "ksys_sync" without any restrictions (identical as it was before loading syncrestrict).

So the current implementation is NOT a security hardening, but a safety one...

The module can be build via "make", setting KDIR to the kernels (source-/header-)directory.

It can be loaded directly via "insmod syncrestrict.ko",
or copied (+depmod) into the kernels module directory (extra/) and then modprobed with "modprobe syncrestrict".

by Stephan Baerwolf, Erfurt 2020
