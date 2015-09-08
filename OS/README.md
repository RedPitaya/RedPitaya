This is the GNU/Linux Operating system for Red Pitaya, an Open-source
instrument based on Xilinx Zynq.

| directories | contents
|-------------|-----------------------------------------------------------------
| buildroot   | Root filesystem (ramdisk).
| tools       | IP discovery connection agent, and other scripts.
| filesystem  | Overlay for SD card filesystem, holding user configurable files.

Upon boot, the ramdisk image and thus the root filesystem resides in RAM.
Although it is writable, its content is lost at next reboot or power cycle.
This way, better OS robustness to abrupt power loss is achieved, compared 
to a root filesystem on an SD card partition being mounted in read-write
mode.


BUILD

All the OS components, including the u-boot, FSBL, FPGA image, devicetree,
Linux kernel and the ramdisk are built from sources using ../Makefile.

The Linux kernel uImage is built from git repository defined by LINUX_GIT
build valiable. By defaulty local (/var/git/linux-xlnx.git) bare repository
is used. This can be changed if remote repository is to be used.
E.g. make clean all LINUX_GIT=git://<host>/linux-xlnx.git.

The u-boot is built from git repository defined by UBOOT_GIT build valiable.
By defaulty local (/var/git/u-boot-xlnx.git) bare repository is used.
This can be changed if remote repository is to be used.
E.g. make clean all UBOOT_GIT=git://<host>/u-boot-xlnx.git.

