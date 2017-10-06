.. _UIO:

###
UIO
###

Userspace input output or `UIO <https://www.kernel.org/doc/html/v4.12/driver-api/uio-howto.html>`_
Enables writing hardware drivers in user space with a small kernel module providing
memory space mapping and interrupt support.

Although dedicated UIO kernel drivers can be written,
we are using a generic driver named ``uio_pdrv_genirq``.
Since this driver is actually intended as an example on how to write UIO drivers,
it is missing the ``compatible`` identifiers used by device tree nodes.
This issues can be solved by providing a compatible string as
a kernel boot argument ``uio_pdrv_genirq.of_id="generic-uio"``.
We do this in :download:`u-boot.script <../../../patches/u-boot/u-boot.script>`.

****************
Device tree node
****************

A new UIO device is added by providing a device tree node describing it.

The next example describes an arbitrary signal generator:

.. code-block:: c

   gen0: gen0@40040000 {
     compatible = "generic-uio";
     reg = <0x40040000 0x01000>,
           <0x40050000 0x10000>;  // 2**14 * sizeof(int32_t), TODO: int16_t
     reg-names = "regset", "buffer";
     interrupt-parent = <&axi_intc_0>;
     interrupts = <0 1>;
   };

The ``compatible`` attribute must use the same driver name as provided as kernel argument.

The ``reg`` attribute must contain one or more address space windows.
Each window is defined by the base address and size. In our case
the window is inside the address space used by the AXI-GP0 port on the ZYNQ device.
In the given examples two windows are given, the first for the register set,
the second for the buffer. The ``reg-names`` attribute provides window names.

Optionally an interrupt can be provided. ``interrupt-parent`` links
to the interrupt controller the signals is connected to.
In argument ``interrupts`` the first value specifis the index of the interrupt signal
on the connected interrupt controller, the cecond value is interrupt type.

****
UDEV
****

The Linux kernel will index each UIO device in the order it processed it.
So devices like ``/dev/uio0``, ``/dev/uio1``, ... will be present on the system.
Since this names depend on the loading order, and can change if new devices are added,
an `UDEV configuration file </OS/debian/overlay/etc/udev/rules.d/10-redpitaya.rules>`_
can be used to give each device a symbolink link, containing the name specified in the device tree.

.. code-block:: c

   SUBSYSTEM=="uio", SYMLINK+="uio/%s{name}", GROUP="uio"

.. code-block:: shell-session

   # ll /dev/uio
   total 0
   drwxr-xr-x  2 root root  280 Sep 18 10:28 ./
   drwxr-xr-x 11 root root 3240 Sep 18 10:28 ../
   lrwxrwxrwx  1 root root    7 Sep 14 08:52 api -> ../uio1
   lrwxrwxrwx  1 root root    7 Sep 18 10:28 gen0 -> ../uio6
   lrwxrwxrwx  1 root root    7 Sep 18 10:28 gen1 -> ../uio7

Additional ``sysfs`` nodes provide details on the given device:

.. code-block:: shell-session

   # find /sys/devices/soc0/amba_pl/40040000.gen0
   /sys/devices/soc0/amba_pl/40040000.gen0
   /sys/devices/soc0/amba_pl/40040000.gen0/subsystem
   /sys/devices/soc0/amba_pl/40040000.gen0/driver
   /sys/devices/soc0/amba_pl/40040000.gen0/uio
   /sys/devices/soc0/amba_pl/40040000.gen0/uio/uio6
   /sys/devices/soc0/amba_pl/40040000.gen0/uio/uio6/version
   /sys/devices/soc0/amba_pl/40040000.gen0/uio/uio6/device
   /sys/devices/soc0/amba_pl/40040000.gen0/uio/uio6/event
   /sys/devices/soc0/amba_pl/40040000.gen0/uio/uio6/subsystem
   /sys/devices/soc0/amba_pl/40040000.gen0/uio/uio6/power
   /sys/devices/soc0/amba_pl/40040000.gen0/uio/uio6/power/runtime_suspended_time
   /sys/devices/soc0/amba_pl/40040000.gen0/uio/uio6/power/autosuspend_delay_ms
   /sys/devices/soc0/amba_pl/40040000.gen0/uio/uio6/power/runtime_active_time
   /sys/devices/soc0/amba_pl/40040000.gen0/uio/uio6/power/control
   /sys/devices/soc0/amba_pl/40040000.gen0/uio/uio6/power/runtime_status
   /sys/devices/soc0/amba_pl/40040000.gen0/uio/uio6/maps
   /sys/devices/soc0/amba_pl/40040000.gen0/uio/uio6/maps/map0
   /sys/devices/soc0/amba_pl/40040000.gen0/uio/uio6/maps/map0/offset
   /sys/devices/soc0/amba_pl/40040000.gen0/uio/uio6/maps/map0/size
   /sys/devices/soc0/amba_pl/40040000.gen0/uio/uio6/maps/map0/name
   /sys/devices/soc0/amba_pl/40040000.gen0/uio/uio6/maps/map0/addr
   /sys/devices/soc0/amba_pl/40040000.gen0/uio/uio6/maps/map1
   /sys/devices/soc0/amba_pl/40040000.gen0/uio/uio6/maps/map1/offset
   /sys/devices/soc0/amba_pl/40040000.gen0/uio/uio6/maps/map1/size
   /sys/devices/soc0/amba_pl/40040000.gen0/uio/uio6/maps/map1/name
   /sys/devices/soc0/amba_pl/40040000.gen0/uio/uio6/maps/map1/addr
   /sys/devices/soc0/amba_pl/40040000.gen0/uio/uio6/dev
   /sys/devices/soc0/amba_pl/40040000.gen0/uio/uio6/uevent
   /sys/devices/soc0/amba_pl/40040000.gen0/uio/uio6/name
   /sys/devices/soc0/amba_pl/40040000.gen0/power
   /sys/devices/soc0/amba_pl/40040000.gen0/power/runtime_suspended_time
   /sys/devices/soc0/amba_pl/40040000.gen0/power/autosuspend_delay_ms
   /sys/devices/soc0/amba_pl/40040000.gen0/power/runtime_active_time
   /sys/devices/soc0/amba_pl/40040000.gen0/power/control
   /sys/devices/soc0/amba_pl/40040000.gen0/power/runtime_status
   /sys/devices/soc0/amba_pl/40040000.gen0/driver_override
   /sys/devices/soc0/amba_pl/40040000.gen0/modalias
   /sys/devices/soc0/amba_pl/40040000.gen0/uevent
   /sys/devices/soc0/amba_pl/40040000.gen0/of_node

Memory window settings provided in the device tree can be read from
``maps/map0`` and ``maps/map1`` nodes.

*****************
User space driver
*****************

Access to UIO memory windows is similar to mapping ``/dev/mem``.
The device ``/dev/uio/gen0`` is opened and ``mmap`` is used to
map the physical memory window into virtual address space.

.. code-block:: C

   #include <fcntl.h>
   #include <unistd.h>
   #include <sys/mman.h>
   #include <stdio.h>

   static int fd = 0;
   uint32_t *regset;
   int16_t *buffer;

   int uio_open(int *fd, uint32_t **regset, int16_t **buffer) {
       size_t offset;
       size_t size;

       # open UIO device file
       if ((*fd = open("/dev/uio/gen0", O_RDWR | O_SYNC)) == -1) {
           return -1;
       }
       // map regset memory window
       offset = 0x0;
       size = 0x1000;
       *regset = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, *fd, 0);
       if (regset == (void *) -1) {
           return -1;
       }
       // map buffer memory window
       // each consecutive memory window reqiures an offset of (index * PAGESIZE)
       offset = sysconf(_SC_PAGESIZE);
       size = 0x10000;
       *buffer = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, *fd, offset);
       if (buffer == (void *) -1) {
           return -1;
       }
       return 0;
   }

   int uio_close(int * fd, uint32_t **regset, uint16_t **buffer) {
       size_t size;

       size = 0x1000;
       if (munmap(*regset, size) < 0) {
           return -1;
       }
       size = 0x10000;
       if (munmap(*buffer, size) < 0) {
           return -1;
       }
       if (close(*fd) < 0) {
           return -1;
       }
       return 0;
   }

If regset is cast onto a structure containing 32bit registers,
registers can be read or written to using elements of this structure.

********
Examples
********

The **mercury** FPGA image and related user space code is using UIO extensively.

Each HW module inside the FPGA is listed as an UIO device in the device tree.

/fpga/prj/mercury/dts/fpga.dtso

A Python API is provided:

https://github.com/RedPitaya/jupyter/blob/master/redpitaya/drv/uio.py
