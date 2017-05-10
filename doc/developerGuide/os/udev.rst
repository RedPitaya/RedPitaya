.. _udev:

##########
UDEV rules
##########

The documentation for writing UDEV rules can be found here:
https://www.freedesktop.org/software/systemd/man/udev.html

.. |10-redpitaya.rule| replace:: ``/etc/udev/rules.d/10-redpitaya.rule``
.. _10-redpitaya.rule: ../../../OS/debian/overlay/etc/udev/rules.d/10-redpitaya.rule

.. |uio-api.dtsi| replace:: ``uio-api.dtsi``
.. _uio-api.dtsi: ../../../fpga/dts/uio-api.dtsi


Red Pitaya provides an UDEV rule |10-redpitaya.rule|_ file
used to provide **non root users** access rights to hardware components.

*********************************
Non root access to device drivers
*********************************

One or more **match keys** are used to find a specific device or a group of devices.
Usually the ``SUBSYSTEM`` key is enough, but sometimes a more detailed search is necessary.

For hardware components which only require *read/write/ioctl* access
to a ``/dev/*`` device node the only required **assignment key** is ``GROUP="device-name"``.

While non root users can read ``/sys/*``, writing is restricted.
It is possible to write UDEV rules which modify access permissions
to ``/sys/*`` files, but it does not work very well.
For example some files are created as needed,
but it takes some time for permission changes to be applied,
so it is possible to attempt write access to a file,
before permissions are granged.
This issue affects current GPIO, LED, EEPROM and IIO (XADC) drivers.

=======
XDEVCFG
=======

Device node ``/dev/xdevcfg`` enables users to write a FPGA bitstream.

Users should be added into the ``xdevcfg`` group.

===========
UIO devices
===========

https://www.kernel.org/doc/html/latest/driver-api/uio-howto.html

The Linux UIO subsystem provides a generic driver,
which can ge used to ``mmap`` a specified memory mapped device
into user space and it also provides support for
a single interrupt per UIO device.
UIO devices are listed in the device tree,
for each device one or more base address, size pairs
and the interrupt number are listed.
Device tree |uio-api.dtsi|_ for example is used by our API.

Users should be added into the ``xdevcfg`` group.

By default Linux kernel enumerates UIO devices in the order
they apper in the device tree ``/dev/uio[012...]``.
The provided UDEV rule also creates a symbolic link ``/dev/uio/uio-device-name``
with the name of the device extracted from the device tree.

Users should be added into the ``uio`` group.

By matching against UIO device names,
it is also possible to write UDEV rules,
which give different users different access rights for each UIO device.

===
LED
===

Modyfying LED status requires write access to some ``/sys/*`` files.

The UDEV rules for LED are present in |10-redpitaya.rule|_ for review,
but are currently commented out.

====
GPIO
====

Modyfying GPIO status requires write access to some ``/sys/*`` files.
Additionaly each GPIO must be exported before it can be used.
Exporting creates new files which require access rights modifications.
This change takes time which makes it unreliable.

The UDEV rules for GPIO are present in |10-redpitaya.rule|_ for review,
but are currently commented out.

Kernel 4.8 provides a new character device driver for GPIO,
but we are not using it yet.

======
EEPROM
======

Commented out due to ``/sys/*`` access issues.

==========
IIO (XADC)
==========

Only basic support which can be achieved
by only reading ``/sys/*`` files is provided.

Users should be added into the ``xadc`` group.

*********
Debugging
*********

TODO
